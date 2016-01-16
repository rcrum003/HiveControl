/*
 * This file is part of libphidget21
 *
 * Copyright 2006-2015 Phidgets Inc <patrick@phidgets.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see 
 * <http://www.gnu.org/licenses/>
 */

#ifndef __CPHIDGETTEMPERATURESENSOR
#define __CPHIDGETTEMPERATURESENSOR
#include "cphidget.h"

/** \defgroup phidtemp Phidget Temperature Sensor 
 * \ingroup phidgets
 * These calls are specific to the Phidget Temperature Sensor object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 *
 * All temperatures are in degrees celcius.
 * @{
 */

DPHANDLE(TemperatureSensor)
CHDRSTANDARD(TemperatureSensor)

/**
 * The Phidget Temperature Sensor supports these types of thermocouples.
 */
typedef enum {
	PHIDGET_TEMPERATURE_SENSOR_K_TYPE = 1,	/**< K-Type thermocouple */
	PHIDGET_TEMPERATURE_SENSOR_J_TYPE,		/**< J-Type thermocouple */
	PHIDGET_TEMPERATURE_SENSOR_E_TYPE,		/**< E-Type thermocouple */
	PHIDGET_TEMPERATURE_SENSOR_T_TYPE		/**< T-Type thermocouple */
}  CPhidgetTemperatureSensor_ThermocoupleType;

/**
 * Gets the number of thermocouple inputs supported by this board.
 * @param phid An attached phidget themperature sensor handle.
 * @param count The thermocouple input count.
 */
CHDRGET(TemperatureSensor,TemperatureInputCount,int *count)
/**
 * Gets the temperature measured by a thermocouple input.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param temperature The temperature.
 */
CHDRGETINDEX(TemperatureSensor,Temperature,double *temperature)
/**
 * Gets the maximum temperature that can be measured by a thermocouple input. This depends on the type of thermocouple attached, as well as the ambient temperature.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param max The maximum temperature.
 */
CHDRGETINDEX(TemperatureSensor,TemperatureMax,double *max)
/**
 * Gets the minimum temperature that can be measured by a thermocouple input. This depends on the type of thermocouple attached, as well as the ambient temperature.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param min The minimum temperature.
 */
CHDRGETINDEX(TemperatureSensor,TemperatureMin,double *min)
/**
 * Set a temperature change handler. This is called when the temperature changes by more then the change trigger.
 * @param phid An attached phidget temperature sensor handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(TemperatureSensor,TemperatureChange,double temperature)
/**
 * Gets the change trigger for a thermocouple input.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param trigger The change trigger.
 */
CHDRGETINDEX(TemperatureSensor,TemperatureChangeTrigger,double *trigger)
/**
 * Sets the change trigger for a thermocouple input.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param trigger The change trigger.
 */
CHDRSETINDEX(TemperatureSensor,TemperatureChangeTrigger,double trigger)

/**
 * Gets the currently sensed potential for a thermocouple input.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param potential The potential.
 */
CHDRGETINDEX(TemperatureSensor,Potential,double *potential)
/**
 * Gets the maximum potential that a thermocouple input can measure.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param max The maximum potential.
 */
CHDRGETINDEX(TemperatureSensor,PotentialMax,double *max)
/**
 * Gets the minimum potential that a thermocouple input can measure.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param min The minimum potential.
 */
CHDRGETINDEX(TemperatureSensor,PotentialMin,double *min)

/**
 * Gets the ambient (board) temperature.
 * @param phid An attached phidget themperature sensor handle.
 * @param ambient The ambient (board) temperature.
 */
CHDRGET(TemperatureSensor,AmbientTemperature,double *ambient)
/**
 * Gets the maximum temperature that the ambient onboard temperature sensor can measure.
 * @param phid An attached phidget themperature sensor handle.
 * @param max The maximum temperature.
 */
CHDRGET(TemperatureSensor,AmbientTemperatureMax,double *max)
/**
 * Gets the minimum temperature that the ambient onboard temperature sensor can measure.
 * @param phid An attached phidget themperature sensor handle.
 * @param min The minimum temperature.
 */
CHDRGET(TemperatureSensor,AmbientTemperatureMin,double *min)

/**
 * Gets the type of thermocouple set to be at a thermocouple input. By default this is K-Type.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param type The thermocouple type.
 */
CHDRGETINDEX(TemperatureSensor, ThermocoupleType, CPhidgetTemperatureSensor_ThermocoupleType *type)
/**
 * Sets the type of thermocouple plugged into a thermocouple input. By default this is K-Type.
 * @param phid An attached phidget themperature sensor handle.
 * @param index The thermocouple index.
 * @param type The thermocouple type.
 */
CHDRSETINDEX(TemperatureSensor, ThermocoupleType, CPhidgetTemperatureSensor_ThermocoupleType type)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetTemepratureSensor_getTemperatureInputCount",TemperatureSensor,NumTemperatureInputs,int *)
#endif

#ifndef EXTERNALPROTO
#define TEMPSENSOR_MAXSENSORS 8

#define GAIN 85.0
#define OFFSET_200 -6.5
#define OFFSET_300 ((200.0/237.0)*5.0)

#define PHIDID_TEMPERATURESENSOR_4_GAIN		((80 / 2.2) + 5)
//using 53.6K + 10K offset resistors: VOffset = (4.096Vref * 10K) / (10K + 53.6K)
#define PHIDID_TEMPERATURESENSOR_4_OFFSET	(4.096 / 6.36)

struct _CPhidgetTemperatureSensor {
	CPhidget phid;

	int (CCONV *fptrTemperatureChange)(CPhidgetTemperatureSensorHandle, void *, int, double);
	void *fptrTemperatureChangeptr;

	double AmbientTemperature;
	double Temperature[TEMPSENSOR_MAXSENSORS];
	double Potential[TEMPSENSOR_MAXSENSORS];

	double lastTrigger[TEMPSENSOR_MAXSENSORS];

	double TempChangeTrigger[TEMPSENSOR_MAXSENSORS];
	CPhidgetTemperatureSensor_ThermocoupleType ThermocoupleType[TEMPSENSOR_MAXSENSORS];

	double ambientTemperatureMax, ambientTemperatureMin;
	double temperatureMax[TEMPSENSOR_MAXSENSORS], temperatureMin[TEMPSENSOR_MAXSENSORS];
	double potentialMax, potentialMin;

	unsigned char AmbientTemperatureError, TemperatureError[TEMPSENSOR_MAXSENSORS], PotentialError[TEMPSENSOR_MAXSENSORS];

} typedef CPhidgetTemperatureSensorInfo;
#endif

/** @} */

#endif
