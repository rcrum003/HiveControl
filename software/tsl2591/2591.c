/*
 * TSL2591 light sensor driver for Linux / Raspberry Pi
 *
 * Standalone C port of the Adafruit TSL2591 Arduino library register protocol.
 * Reads lux via raw Linux I2C and prints for use by HiveControl shell scripts.
 *
 * Output format:  "Lux: <value>"
 * (tsl2591.sh parses this with: grep Lux | grep -o "[0-9]*$")
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/i2c-dev.h>

#define TSL2591_ADDR             0x29

/* Command bit: bits 7 and 5 set (0xA0) for normal command */
#define TSL2591_COMMAND_BIT      0xA0

/* Registers */
#define TSL2591_REG_ENABLE       0x00
#define TSL2591_REG_CONTROL      0x01
#define TSL2591_REG_DEVICE_ID    0x12
#define TSL2591_REG_CHAN0_LOW    0x14
#define TSL2591_REG_CHAN1_LOW    0x16

/* Enable register flags */
#define TSL2591_ENABLE_POWERON   0x01
#define TSL2591_ENABLE_AEN       0x02
#define TSL2591_ENABLE_POWEROFF  0x00

/* Gain values (written to CONTROL register bits 5:4) */
#define TSL2591_GAIN_LOW         0x00  /* 1x   */
#define TSL2591_GAIN_MED         0x10  /* 25x  */
#define TSL2591_GAIN_HIGH        0x20  /* 428x */
#define TSL2591_GAIN_MAX         0x30  /* 9876x */

/* Integration time values (written to CONTROL register bits 2:0) */
#define TSL2591_INTEG_100MS      0x00
#define TSL2591_INTEG_200MS      0x01
#define TSL2591_INTEG_300MS      0x02
#define TSL2591_INTEG_400MS      0x03
#define TSL2591_INTEG_500MS      0x04
#define TSL2591_INTEG_600MS      0x05

/* Lux coefficients (AMS / Adafruit) */
#define TSL2591_LUX_DF    408.0f

static int i2c_fd = -1;

static int tsl2591_write8(uint8_t reg, uint8_t value) {
	uint8_t buf[2];
	buf[0] = TSL2591_COMMAND_BIT | reg;
	buf[1] = value;
	if(write(i2c_fd, buf, 2) != 2) return -1;
	return 0;
}

static int tsl2591_read8(uint8_t reg, uint8_t *value) {
	uint8_t buf[1];
	buf[0] = TSL2591_COMMAND_BIT | reg;
	if(write(i2c_fd, buf, 1) != 1) return -1;
	if(read(i2c_fd, buf, 1) != 1) return -1;
	*value = buf[0];
	return 0;
}

static int tsl2591_read16(uint8_t reg, uint16_t *value) {
	uint8_t buf[2];
	buf[0] = TSL2591_COMMAND_BIT | reg;
	if(write(i2c_fd, buf, 1) != 1) return -1;
	if(read(i2c_fd, buf, 2) != 2) return -1;
	*value = (uint16_t)buf[1] << 8 | buf[0];
	return 0;
}

static float gain_factor(uint8_t gain) {
	switch(gain) {
		case TSL2591_GAIN_LOW:  return 1.0f;
		case TSL2591_GAIN_MED:  return 25.0f;
		case TSL2591_GAIN_HIGH: return 428.0f;
		case TSL2591_GAIN_MAX:  return 9876.0f;
		default: return 1.0f;
	}
}

static float integ_ms(uint8_t integ) {
	switch(integ) {
		case TSL2591_INTEG_100MS: return 100.0f;
		case TSL2591_INTEG_200MS: return 200.0f;
		case TSL2591_INTEG_300MS: return 300.0f;
		case TSL2591_INTEG_400MS: return 400.0f;
		case TSL2591_INTEG_500MS: return 500.0f;
		case TSL2591_INTEG_600MS: return 600.0f;
		default: return 100.0f;
	}
}

static int try_bus(int bus_nr) {
	char filename[20];
	uint8_t id;
	uint16_t ch0, ch1;

	uint8_t gain = TSL2591_GAIN_MED;
	uint8_t integ = TSL2591_INTEG_300MS;

	snprintf(filename, sizeof(filename), "/dev/i2c-%d", bus_nr);
	i2c_fd = open(filename, O_RDWR);
	if(i2c_fd < 0) return -1;

	if(ioctl(i2c_fd, I2C_SLAVE, TSL2591_ADDR) < 0) {
		close(i2c_fd); i2c_fd = -1;
		return -1;
	}

	/* Verify this is actually a TSL2591 (ID register = 0x50) */
	if(tsl2591_read8(TSL2591_REG_DEVICE_ID, &id) != 0 || id != 0x50) {
		close(i2c_fd); i2c_fd = -1;
		return -1;
	}

	/* Enable: power on + ALS enable */
	if(tsl2591_write8(TSL2591_REG_ENABLE, TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN) != 0) {
		fprintf(stderr, "Error enabling TSL2591 on i2c-%d (%s)\n", bus_nr, strerror(errno));
		close(i2c_fd); i2c_fd = -1;
		return -1;
	}

	/* Set gain and integration time */
	if(tsl2591_write8(TSL2591_REG_CONTROL, integ | gain) != 0) {
		fprintf(stderr, "Error configuring TSL2591 on i2c-%d (%s)\n", bus_nr, strerror(errno));
		close(i2c_fd); i2c_fd = -1;
		return -1;
	}

	/* Wait for ADC: (integ_value + 1) * 120ms, matching Adafruit library */
	usleep((integ + 1) * 120000);

	/* Read channel 0 (visible + IR) — must be read before channel 1 */
	if(tsl2591_read16(TSL2591_REG_CHAN0_LOW, &ch0) != 0) {
		fprintf(stderr, "Error reading TSL2591 CH0 on i2c-%d (%s)\n", bus_nr, strerror(errno));
		tsl2591_write8(TSL2591_REG_ENABLE, TSL2591_ENABLE_POWEROFF);
		close(i2c_fd); i2c_fd = -1;
		return -1;
	}

	/* Read channel 1 (IR only) */
	if(tsl2591_read16(TSL2591_REG_CHAN1_LOW, &ch1) != 0) {
		fprintf(stderr, "Error reading TSL2591 CH1 on i2c-%d (%s)\n", bus_nr, strerror(errno));
		tsl2591_write8(TSL2591_REG_ENABLE, TSL2591_ENABLE_POWEROFF);
		close(i2c_fd); i2c_fd = -1;
		return -1;
	}

	/* Power off */
	tsl2591_write8(TSL2591_REG_ENABLE, TSL2591_ENABLE_POWEROFF);
	close(i2c_fd); i2c_fd = -1;

	/* Overflow check */
	if(ch0 == 0xFFFF || ch1 == 0xFFFF) {
		printf("Lux: 0\n");
		return 0;
	}

	/* Calculate lux using AMS coefficients (same formula as Adafruit library) */
	float cpl = (integ_ms(integ) * gain_factor(gain)) / TSL2591_LUX_DF;
	float lux;

	if(ch0 == 0) {
		lux = 0.0f;
	} else {
		lux = ((float)ch0 - (float)ch1) * (1.0f - ((float)ch1 / (float)ch0)) / cpl;
	}

	if(lux < 0.0f) lux = 0.0f;

	printf("Lux: %d\n", (int)lux);

	return 0;
}

int main() {
	int buses[] = {1, 3, 0};
	int nbuses = sizeof(buses) / sizeof(buses[0]);

	for(int i = 0; i < nbuses; i++) {
		if(try_bus(buses[i]) == 0) {
			return 0;
		}
	}

	fprintf(stderr, "Error: TSL2591 sensor not found on any i2c bus.\n");
	fprintf(stderr, "Run 'i2cdetect -y 1' to check wiring. Expected at address 0x29.\n");
	return 1;
}
