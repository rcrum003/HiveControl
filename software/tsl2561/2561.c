
#include <stdio.h>
#include <string.h>
#include "TSL2561.h"

static int try_bus(int bus_nr, uint8_t addr) {
	int rc;
	uint16_t broadband, ir;
	uint32_t lux=0;

	TSL2561 light1 = TSL2561_INIT(bus_nr, addr);

	rc = TSL2561_OPEN(&light1);
	if(rc != 0) {
		TSL2561_CLOSE(&light1);
		return -1;
	}

	rc = TSL2561_SETGAIN(&light1, TSL2561_GAIN_1X);
	if(rc != 0) {
		fprintf(stderr, "Error setting gain on i2c-%d addr 0x%02x (%s)\n", bus_nr, addr, strerror(light1.lasterr));
		TSL2561_CLOSE(&light1);
		return -1;
	}

	rc = TSL2561_SETINTEGRATIONTIME(&light1, TSL2561_INTEGRATIONTIME_101MS);
	if(rc != 0) {
		fprintf(stderr, "Error setting integration time on i2c-%d addr 0x%02x (%s)\n", bus_nr, addr, strerror(light1.lasterr));
		TSL2561_CLOSE(&light1);
		return -1;
	}

	rc = TSL2561_SENSELIGHT(&light1, &broadband, &ir, &lux, 1);
	if(rc != 0) {
		fprintf(stderr, "Error reading sensor on i2c-%d addr 0x%02x (%s)\n", bus_nr, addr, strerror(light1.lasterr));
		TSL2561_CLOSE(&light1);
		return -1;
	}

	printf("%i, %i, %i, %i\n", rc, broadband, ir, lux);

	TSL2561_CLOSE(&light1);
	return 0;
}

int main() {
	int buses[] = {1, 3, 0};
	int nbuses = sizeof(buses) / sizeof(buses[0]);
	uint8_t addrs[] = {TSL2561_ADDR_LOW, TSL2561_ADDR_FLOAT, TSL2561_ADDR_HIGH};
	int naddrs = sizeof(addrs) / sizeof(addrs[0]);

	for(int i = 0; i < nbuses; i++) {
		for(int j = 0; j < naddrs; j++) {
			if(try_bus(buses[i], addrs[j]) == 0) {
				return 0;
			}
		}
	}

	fprintf(stderr, "Error: TSL2561 sensor not found on any i2c bus/address.\n");
	fprintf(stderr, "Run 'i2cdetect -y 1' to check wiring. Expected at 0x29, 0x39, or 0x49.\n");
	return 1;
}
