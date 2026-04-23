
#include <stdio.h>
#include <string.h>
#include "TSL2561.h"

static int try_bus(int bus_nr) {
	int rc;
	uint16_t broadband, ir;
	uint32_t lux=0;

	TSL2561 light1 = TSL2561_INIT(bus_nr, TSL2561_ADDR_FLOAT);

	rc = TSL2561_OPEN(&light1);
	if(rc != 0) {
		TSL2561_CLOSE(&light1);
		return -1;
	}

	rc = TSL2561_SETGAIN(&light1, TSL2561_GAIN_1X);
	if(rc != 0) {
		fprintf(stderr, "Error setting gain on i2c-%d (%s)\n", bus_nr, strerror(light1.lasterr));
		TSL2561_CLOSE(&light1);
		return -1;
	}

	rc = TSL2561_SETINTEGRATIONTIME(&light1, TSL2561_INTEGRATIONTIME_101MS);
	if(rc != 0) {
		fprintf(stderr, "Error setting integration time on i2c-%d (%s)\n", bus_nr, strerror(light1.lasterr));
		TSL2561_CLOSE(&light1);
		return -1;
	}

	rc = TSL2561_SENSELIGHT(&light1, &broadband, &ir, &lux, 1);
	if(rc != 0) {
		fprintf(stderr, "Error reading sensor on i2c-%d (%s)\n", bus_nr, strerror(light1.lasterr));
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

	for(int i = 0; i < nbuses; i++) {
		if(try_bus(buses[i]) == 0) {
			return 0;
		}
	}

	fprintf(stderr, "Error: TSL2561 sensor not found on any i2c bus. Run 'i2cdetect -y 1' to check wiring.\n");
	return 1;
}
