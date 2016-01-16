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

#include "stdafx.h"
#include "cphidgettemperaturesensor.h"
#include <math.h>
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

double lookup_voltage(double Tjunction, CPhidgetTemperatureSensor_ThermocoupleType type);
double lookup_temperature(double Vthermocouple, CPhidgetTemperatureSensor_ThermocoupleType type);

//Range on either side of 0 for the thermocouple tables
// - this is difference is degrees Celcius between junction and tip temperature.
//   ie. if the board is at 100C, the lowest you could measure is -170C.
const int thermocouple_table_range[5][2] = 
{
	{0,0},//null
	{-270, 1372}, //Type K
	{-210, 1200}, //Type J
	{-270, 1000}, //Type E
	{-270, 400} //Type T
};
//Useful measurement range for each type of thermocouple
// - this is the actual absolute temperature at the thermocouple tip, in degrees Celcius.
//These ranges are "Maximum Useful Temperature Range" specified here: http://www.omega.com/techref/colorcodes.html
const int thermocouple_useful_range[5][2] = 
{
	{0,0},//null
	{-200, 1250}, //Type K
	{0, 750}, //Type J
	{-200, 900}, //Type E
	{-200, 350} //Type T
};

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(TemperatureSensor)
	int i = 0;

	phid->ambientTemperatureMax = PUNI_DBL;
	phid->ambientTemperatureMin = PUNI_DBL;
	phid->potentialMax = PUNI_DBL;
	phid->potentialMin = PUNI_DBL;
	phid->AmbientTemperature = PUNI_DBL;

	for (i = 0; i<TEMPSENSOR_MAXSENSORS; i++)
	{
		phid->ThermocoupleType[i] = -1;
		phid->temperatureMax[i] = PUNI_DBL;
		phid->temperatureMin[i] = PUNI_DBL;
		phid->Potential[i] = PUNI_DBL;
		phid->Temperature[i] = PUNI_DBL;
		phid->lastTrigger[i] = PUNK_DBL;
		phid->TempChangeTrigger[i] = PUNI_DBL;
	}

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(TemperatureSensor)
	int i;

	TESTPTR(phid);

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR:
			if(phid->phid.deviceVersion < 200) 
			{
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					phid->ThermocoupleType[i] = PHIDGET_TEMPERATURE_SENSOR_K_TYPE;
					phid->temperatureMax[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][1];
					phid->temperatureMin[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][0];
				}
				phid->ambientTemperatureMax = 125;
				phid->ambientTemperatureMin = -40;
			}
			else if(phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300) 
			{
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					phid->ThermocoupleType[i] = PHIDGET_TEMPERATURE_SENSOR_K_TYPE;
					phid->temperatureMax[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][1];
					phid->temperatureMin[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][0];
				}
				phid->ambientTemperatureMax = 125;
				phid->ambientTemperatureMin = -40;
				phid->potentialMax = round_double((((5 / GAIN) * 1000.0) + OFFSET_200), 5);
				phid->potentialMin = round_double(((((5 - (65535.0 / 13104.0)) / GAIN) * 1000.0) + OFFSET_200), 5);
			}
			else if(phid->phid.deviceVersion >= 300 && phid->phid.deviceVersion < 400)
			{
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					phid->ThermocoupleType[i] = PHIDGET_TEMPERATURE_SENSOR_K_TYPE;
					phid->temperatureMax[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][1];
					phid->temperatureMin[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][0];
				}
				phid->ambientTemperatureMax = 150;
				phid->ambientTemperatureMin = -50;
				phid->potentialMax = round_double((-((-OFFSET_300) / GAIN) * 1000.0), 5);
				phid->potentialMin = round_double((-(((65535.0 / 13104.0) - OFFSET_300) / GAIN) * 1000.0), 5);
			}
			else if(phid->phid.deviceVersion >= 400 && phid->phid.deviceVersion < 500) 
			{
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					phid->ThermocoupleType[i] = PHIDGET_TEMPERATURE_SENSOR_K_TYPE;
					phid->temperatureMax[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][1];
					phid->temperatureMin[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][0];
				}
				phid->ambientTemperatureMax = 150;
				phid->ambientTemperatureMin = -55;
				phid->potentialMin = round_double((((0.23 - PHIDID_TEMPERATURESENSOR_4_OFFSET) / PHIDID_TEMPERATURESENSOR_4_GAIN) * 1000.0), 5);
				phid->potentialMax = round_double((((3.846 - PHIDID_TEMPERATURESENSOR_4_OFFSET) / PHIDID_TEMPERATURESENSOR_4_GAIN) * 1000.0), 5);
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		case PHIDID_TEMPERATURESENSOR_4:
			if(phid->phid.deviceVersion >= 100 && phid->phid.deviceVersion < 200) 
			{
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					phid->ThermocoupleType[i] = PHIDGET_TEMPERATURE_SENSOR_K_TYPE;
					phid->temperatureMax[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][1];
					phid->temperatureMin[i] = thermocouple_useful_range[PHIDGET_TEMPERATURE_SENSOR_K_TYPE][0];
				}
				phid->ambientTemperatureMax = 150;
				phid->ambientTemperatureMin = -55;
				phid->potentialMin = round_double((((0.23 - PHIDID_TEMPERATURESENSOR_4_OFFSET) / PHIDID_TEMPERATURESENSOR_4_GAIN) * 1000.0), 5);
				phid->potentialMax = round_double((((3.846 - PHIDID_TEMPERATURESENSOR_4_OFFSET) / PHIDID_TEMPERATURESENSOR_4_GAIN) * 1000.0), 5);
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		case PHIDID_TEMPERATURESENSOR_IR:
			if(phid->phid.deviceVersion >= 100 && phid->phid.deviceVersion < 200) 
			{
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					phid->temperatureMax[i] = 380;
					phid->temperatureMin[i] = -70;
				}
				phid->ambientTemperatureMax = 125;
				phid->ambientTemperatureMin = -40;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
	{
		phid->PotentialError[i] = PFALSE;
		phid->TemperatureError[i] = PFALSE;
		phid->Potential[i] = PUNK_DBL;
		phid->Temperature[i] = PUNK_DBL;
		phid->lastTrigger[i] = PUNK_DBL;
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_TEMPERATURESENSOR:
				phid->TempChangeTrigger[i] = 0.5;
				break;
			case PHIDID_TEMPERATURESENSOR_4:
			case PHIDID_TEMPERATURESENSOR_IR:
				phid->TempChangeTrigger[i] = 0.1;
				break;
			default:
				break;
		}
	}
	phid->AmbientTemperature = PUNK_DBL;
	phid->AmbientTemperatureError = PFALSE;

	//issue one read
	CPhidget_read((CPhidgetHandle)phid);

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(TemperatureSensor)
	int i = 0;
	double Vad = 0, Vthermocouple = 0, VtempIC = 0, Temp;
	double Temperature[TEMPSENSOR_MAXSENSORS] = {0}, Potential[TEMPSENSOR_MAXSENSORS] = {0}, AmbientTemperature = 0;
	char error_buffer[200];

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	ZEROMEM(Temperature, sizeof(Temperature));
	ZEROMEM(Potential, sizeof(Potential));

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR:
			/* OLD version */
			if(phid->phid.deviceVersion < 200)
			{
				AmbientTemperature = (double)((short)buffer[2]);
				Temperature[0] = (double)((signed short)((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8)));
			}
			/* newer versions... calculate the temp here from Vad */
			else if(phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300) {
				//The ambient sensor reading from firmware isn't being sign-extended properly!
				if(buffer[3] & 0x40)
					buffer[3] |= 0x80;
				if(buffer[3] & 0x30)
					buffer[3] |= 0xf0;
				AmbientTemperature = (double)((signed short)((unsigned short)buffer[2]+((unsigned short)buffer[3]<<8))) / 64.0;
				printf("Ambient: 0x%02x%02x\n",buffer[3],buffer[2]);
				Vad = ((double)((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8))) / 13104.0;

				Potential[0] = round_double(((((5 - Vad) / GAIN) * 1000.0) + OFFSET_200), 5);

				//This is the potential at the thermocouple tip - with the junction potential eliminated
				Vthermocouple = Potential[0] + lookup_voltage(AmbientTemperature, phid->ThermocoupleType[0]);
				Temp = lookup_temperature(Vthermocouple, phid->ThermocoupleType[0]);
				if(Temp != PUNK_DBL)
					Temperature[0] = round_double(Temp, 3);
				else
					Temperature[0] = PUNK_DBL;

			}
			//upgrade to MCP3202 and AD22100
			else if(phid->phid.deviceVersion >= 300 && phid->phid.deviceVersion < 400)
			{
				//Temp IC - absolute possible range: -61.1 - 161.16
				VtempIC = ((double)((unsigned short)buffer[2]+((unsigned short)buffer[3]<<8))) / 13104.0;
				AmbientTemperature = round_double(((VtempIC - 1.375)/0.0225), 3);

				//Thermocouple
				Vad = ((double)((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8))) / 13104.0;
				Potential[0] = round_double((-((Vad - OFFSET_300) / GAIN) * 1000.0), 5);

				//This is the potential at the thermocouple tip - with the junction potential eliminated
				Vthermocouple = Potential[0] + lookup_voltage(AmbientTemperature, phid->ThermocoupleType[0]);
				Temp = lookup_temperature(Vthermocouple, phid->ThermocoupleType[0]);
				if(Temp != PUNK_DBL)
					Temperature[0] = round_double(Temp, 4);
				else
					Temperature[0] = PUNK_DBL;
			}
			else if(phid->phid.deviceVersion >= 400 && phid->phid.deviceVersion < 500) 
			{
				//Temp IC - absolute possible range: -61.1 - 161.16
				AmbientTemperature = round_double(((short)(((short)buffer[2] << 8) + (buffer[3])) / 128.0), 4);

				//Thermocouple
				Vad = ((double)((unsigned short)buffer[1]+((unsigned short)buffer[0]<<8))) / 16000.0;
				Potential[0] = round_double((((Vad - PHIDID_TEMPERATURESENSOR_4_OFFSET) / PHIDID_TEMPERATURESENSOR_4_GAIN) * 1000.0), 5);

				//This is the potential at the thermocouple tip - with the junction potential eliminated
				Vthermocouple = Potential[0] + lookup_voltage(AmbientTemperature, phid->ThermocoupleType[0]);
				Temp = lookup_temperature(Vthermocouple, phid->ThermocoupleType[0]);
				if(Temp != PUNK_DBL)
					Temperature[0] = round_double(Temp, 4);
				else
					Temperature[0] = PUNK_DBL;
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_TEMPERATURESENSOR_4:
			if(phid->phid.deviceVersion >= 100 && phid->phid.deviceVersion < 200) 
			{
				//Temp IC - absolute possible range: -61.1 - 161.16
				AmbientTemperature = round_double(((short)(((short)buffer[8] << 8) + (buffer[9])) / 128.0), 4);
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					//Thermocouple
					Vad = ((double)((unsigned short)buffer[1+2*i]+((unsigned short)buffer[0+2*i]<<8))) / 16000.0;
					Potential[i] = round_double((((Vad - PHIDID_TEMPERATURESENSOR_4_OFFSET) / PHIDID_TEMPERATURESENSOR_4_GAIN) * 1000.0), 5);

					//This is the potential at the thermocouple tip - with the junction potential eliminated
					Vthermocouple = Potential[i] + lookup_voltage(AmbientTemperature, phid->ThermocoupleType[i]);
					Temp = lookup_temperature(Vthermocouple, phid->ThermocoupleType[i]);
					if(Temp != PUNK_DBL)
						Temperature[i] = round_double(Temp, 4);
					else
						Temperature[i] = PUNK_DBL;
					//LOG(PHIDGET_LOG_DEBUG, "Index: %d Val: %08x Potential: %0.6lf", i, ((unsigned short)buffer[1+2*i]+((unsigned short)buffer[0+2*i]<<8)), Potential[i]);
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_TEMPERATURESENSOR_IR:
			if(phid->phid.deviceVersion >= 100 && phid->phid.deviceVersion < 200) 
			{
				//Temp IC - absolute possible range: -61.1 - 161.16
				AmbientTemperature = round_double(((((short)buffer[3] << 8) + buffer[2]) * 0.02) - 273.15, 2);
				Temperature[0] = round_double(((((short)buffer[1] << 8) + buffer[0]) * 0.02) - 273.15, 2);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure values are within defined range, and store to structure
	if(AmbientTemperature > phid->ambientTemperatureMax)
	{
		phid->AmbientTemperature = PUNK_DBL;
		if (phid->AmbientTemperatureError == PFALSE)
		{
			phid->AmbientTemperatureError = PTRUE;
			FIRE_ERROR(EEPHIDGET_OUTOFRANGE, "Ambient sensor reported a temperature of %.4fC which is over the maximum of %.4fC.", 
				AmbientTemperature, phid->ambientTemperatureMax);
		}
	}
	else if(AmbientTemperature < phid->ambientTemperatureMin)
	{
		phid->AmbientTemperature = PUNK_DBL;
		if (phid->AmbientTemperatureError == PFALSE)
		{
			phid->AmbientTemperatureError = PTRUE;
			FIRE_ERROR(EEPHIDGET_OUTOFRANGE, "Ambient sensor reported a temperature of %.4fC which is below the minimum of %.4fC.", 
				AmbientTemperature, phid->ambientTemperatureMin);
		}
	}
	else
	{
		phid->AmbientTemperature = AmbientTemperature;
		phid->AmbientTemperatureError = PFALSE;

		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_TEMPERATURESENSOR:
				if(phid->phid.deviceVersion < 200)
					break; //OLD version  just reports a temperature
			case PHIDID_TEMPERATURESENSOR_4:
				//Recalibrate min/max temperatures depending on ambient temperature
				for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
				{
					if(lookup_voltage(thermocouple_useful_range[phid->ThermocoupleType[i]][1] - phid->AmbientTemperature, phid->ThermocoupleType[i]) > phid->potentialMax)
						phid->temperatureMax[i] = (int)((lookup_temperature(phid->potentialMax, phid->ThermocoupleType[i]) + phid->AmbientTemperature) / 10) * 10;
					if(lookup_voltage(thermocouple_useful_range[phid->ThermocoupleType[i]][0] - phid->AmbientTemperature, phid->ThermocoupleType[i]) < phid->potentialMin)
						phid->temperatureMin[i] = (int)((lookup_temperature(phid->potentialMin, phid->ThermocoupleType[i]) + phid->AmbientTemperature) / 10) * 10;
				}
				break;
			case PHIDID_TEMPERATURESENSOR_IR:
			default:
				break;
		}
	}

	for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
	{
		//If ambient temperature was outside of allowed range, then calculated temperature will be wrong
		if(phid->AmbientTemperature == PUNK_DBL)
		{
			phid->Temperature[i] = PUNK_DBL;
			if (phid->TemperatureError[i] == PFALSE)
			{
				phid->TemperatureError[i] = PTRUE;
				FIRE_ERROR(EEPHIDGET_OUTOFRANGE, 
					"Temperature %d cannot be calculated because the ambient temperature is unknown.", 
					i);
			}
		}
		else if(Temperature[i] == PUNK_DBL)
		{
			phid->Temperature[i] = PUNK_DBL;
			if (phid->TemperatureError[i] == PFALSE)
			{
				phid->TemperatureError[i] = PTRUE;
				FIRE_ERROR(EEPHIDGET_OUTOFRANGE, 
					"Input %d reported an undefined temperature. Probably there is no thermocouple plugged in.", 
					i);
			}
		}
		else if(Temperature[i] > phid->temperatureMax[i])
		{
			phid->Temperature[i] = PUNK_DBL;
			if (phid->TemperatureError[i] == PFALSE)
			{
				phid->TemperatureError[i] = PTRUE;
				FIRE_ERROR(EEPHIDGET_OUTOFRANGE, 
					"Input %d reported a temperature of %.4fC which is above the maximum of %.4fC.", 
					i, Temperature[i], phid->temperatureMax[i]);
			}
		}
		else if(Temperature[i] < phid->temperatureMin[i])
		{
			phid->Temperature[i] = PUNK_DBL;
			if (phid->TemperatureError[i] == PFALSE)
			{
				phid->TemperatureError[i] = PTRUE;
				FIRE_ERROR(EEPHIDGET_OUTOFRANGE, 
					"Input %d reported a temperature of %.4fC which is below the minimum of %.4fC.", 
					i, Temperature[i], phid->temperatureMin[i]);
			}
		}
		else
		{
			phid->Temperature[i] = Temperature[i];
			phid->TemperatureError[i] = PFALSE;
		}

		//Potential only on the thermocouple versions
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_TEMPERATURESENSOR:
			case PHIDID_TEMPERATURESENSOR_4:
				if(Potential[i] > phid->potentialMax)
				{
					phid->Potential[i] = PUNK_DBL;
					if (phid->PotentialError[i] == PFALSE)
					{
						phid->PotentialError[i] = PTRUE;
						FIRE_ERROR(EEPHIDGET_OUTOFRANGE, 
							"Thermocouple %d reported a potential of %.4fmV which is above the maximum of %.4fmV.", 
							i, Potential[i], phid->potentialMax);
					}
				}
				else if(Potential[i] < phid->potentialMin)
				{
					phid->Potential[i] = PUNK_DBL;
					if (phid->PotentialError[i] == PFALSE)
					{
						phid->PotentialError[i] = PTRUE;
						FIRE_ERROR(EEPHIDGET_OUTOFRANGE, 
							"Thermocouple %d reported a potential of %.4fmV which is below the minimum of %.4fmV.", 
							i, Potential[i], phid->potentialMin);
					}
				}
				else
				{
					phid->Potential[i] = Potential[i];
					phid->PotentialError[i] = PFALSE;
				}
				break;
			case PHIDID_TEMPERATURESENSOR_IR:
			default:
				break;
		}
	}

	//send out any events that exceed or match the trigger
	for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
	{
		if ((fabs(phid->Temperature[i] - phid->lastTrigger[i]) >= phid->TempChangeTrigger[i] || phid->lastTrigger[i] == PUNK_DBL) 
			&& phid->Temperature[i] != PUNK_DBL)
		{
			FIRE(TemperatureChange, i, phid->Temperature[i]);
			phid->lastTrigger[i] = phid->Temperature[i];
		}
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(TemperatureSensor)

	for (i = 0; i<phid->phid.attr.temperaturesensor.numTempInputs; i++)
	{
		if(phid->Temperature[i] != PUNK_DBL)
			FIRE(TemperatureChange, i, phid->Temperature[i]);
	}
	return EPHIDGET_OK;
}

//getPacket - not used for TemperatureSensor
CGETPACKET(TemperatureSensor)
	return EPHIDGET_UNEXPECTED;
}

#define THERMOCOUPLE_TABLE_E_SIZE 1271
//-270 to 1000 in degrees celcius increments
const double thermocouple_table_e_type[THERMOCOUPLE_TABLE_E_SIZE] = {
-9.835, -9.833, -9.831, -9.828, -9.825, -9.821, -9.817, -9.813, -9.808, -9.802,
-9.797, -9.790, -9.784, -9.777, -9.770, -9.762, -9.754, -9.746, -9.737, -9.728,
-9.718, -9.709, -9.698, -9.688, -9.677, -9.666, -9.654, -9.642, -9.630, -9.617,
-9.604, -9.591, -9.577, -9.563, -9.548, -9.534, -9.519, -9.503, -9.487, -9.471,
-9.455, -9.438, -9.421, -9.404, -9.386, -9.368, -9.350, -9.331, -9.313, -9.293,
-9.274, -9.254, -9.234, -9.214, -9.193, -9.172, -9.151, -9.129, -9.107, -9.085,
-9.063, -9.040, -9.017, -8.994, -8.971, -8.947, -8.923, -8.899, -8.874, -8.850,
-8.825, -8.799, -8.774, -8.748, -8.722, -8.696, -8.669, -8.643, -8.616, -8.588,
-8.561, -8.533, -8.505, -8.477, -8.449, -8.420, -8.391, -8.362, -8.333, -8.303,
-8.273, -8.243, -8.213, -8.183, -8.152, -8.121, -8.090, -8.059, -8.027, -7.995,
-7.963, -7.931, -7.899, -7.866, -7.833, -7.800, -7.767, -7.733, -7.700, -7.666,
-7.632, -7.597, -7.563, -7.528, -7.493, -7.458, -7.423, -7.387, -7.351, -7.315,
-7.279, -7.243, -7.206, -7.170, -7.133, -7.096, -7.058, -7.021, -6.983, -6.945,
-6.907, -6.869, -6.831, -6.792, -6.753, -6.714, -6.675, -6.636, -6.596, -6.556,
-6.516, -6.476, -6.436, -6.396, -6.355, -6.314, -6.273, -6.232, -6.191, -6.149,
-6.107, -6.065, -6.023, -5.981, -5.939, -5.896, -5.853, -5.810, -5.767, -5.724,
-5.681, -5.637, -5.593, -5.549, -5.505, -5.461, -5.417, -5.372, -5.327, -5.282,
-5.237, -5.192, -5.147, -5.101, -5.055, -5.009, -4.963, -4.917, -4.871, -4.824,
-4.777, -4.731, -4.684, -4.636, -4.589, -4.542, -4.494, -4.446, -4.398, -4.350,
-4.302, -4.254, -4.205, -4.156, -4.107, -4.058, -4.009, -3.960, -3.911, -3.861,
-3.811, -3.761, -3.711, -3.661, -3.611, -3.561, -3.510, -3.459, -3.408, -3.357,
-3.306, -3.255, -3.204, -3.152, -3.100, -3.048, -2.996, -2.944, -2.892, -2.840,
-2.787, -2.735, -2.682, -2.629, -2.576, -2.523, -2.469, -2.416, -2.362, -2.309,
-2.255, -2.201, -2.147, -2.093, -2.038, -1.984, -1.929, -1.874, -1.820, -1.765,
-1.709, -1.654, -1.599, -1.543, -1.488, -1.432, -1.376, -1.320, -1.264, -1.208,
-1.152, -1.095, -1.039, -0.982, -0.925, -0.868, -0.811, -0.754, -0.697, -0.639,
-0.582, -0.524, -0.466, -0.408, -0.350, -0.292, -0.234, -0.176, -0.117, -0.059,
0.000, 	0.059, 	0.118, 	0.176, 	0.235, 	0.294, 	0.354, 	0.413, 	0.472, 	0.532,
0.591, 	0.651, 	0.711, 	0.770, 	0.830, 	0.890, 	0.950, 	1.010, 	1.071, 	1.131,
1.192, 	1.252, 	1.313, 	1.373, 	1.434, 	1.495, 	1.556, 	1.617, 	1.678, 	1.740,
1.801, 	1.862, 	1.924, 	1.986, 	2.047, 	2.109, 	2.171, 	2.233, 	2.295, 	2.357,
2.420, 	2.482, 	2.545, 	2.607, 	2.670, 	2.733, 	2.795, 	2.858, 	2.921, 	2.984,
3.048, 	3.111, 	3.174, 	3.238, 	3.301, 	3.365, 	3.429, 	3.492, 	3.556, 	3.620,
3.685, 	3.749, 	3.813, 	3.877, 	3.942, 	4.006, 	4.071, 	4.136, 	4.200, 	4.265,
4.330, 	4.395, 	4.460, 	4.526, 	4.591, 	4.656, 	4.722, 	4.788, 	4.853, 	4.919,
4.985, 	5.051, 	5.117, 	5.183, 	5.249, 	5.315, 	5.382, 	5.448, 	5.514, 	5.581,
5.648, 	5.714, 	5.781, 	5.848, 	5.915, 	5.982, 	6.049, 	6.117, 	6.184, 	6.251,
6.319, 	6.386, 	6.454, 	6.522, 	6.590, 	6.658, 	6.725, 	6.794, 	6.862, 	6.930,
6.998, 	7.066, 	7.135, 	7.203, 	7.272, 	7.341, 	7.409, 	7.478, 	7.547, 	7.616,
7.685, 	7.754, 	7.823, 	7.892, 	7.962, 	8.031, 	8.101, 	8.170, 	8.240, 	8.309,
8.379, 	8.449, 	8.519, 	8.589, 	8.659, 	8.729, 	8.799, 	8.869, 	8.940, 	9.010,
9.081, 	9.151, 	9.222, 	9.292, 	9.363, 	9.434, 	9.505, 	9.576, 	9.647, 	9.718,
9.789, 	9.860, 	9.931, 	10.003, 10.074, 10.145, 10.217, 10.288, 10.360, 10.432,
10.503, 10.575, 10.647, 10.719, 10.791, 10.863, 10.935, 11.007, 11.080, 11.152,
11.224, 11.297, 11.369, 11.442, 11.514, 11.587, 11.660, 11.733, 11.805, 11.878,
11.951, 12.024, 12.097, 12.170, 12.243, 12.317, 12.390, 12.463, 12.537, 12.610,
12.684, 12.757, 12.831, 12.904, 12.978, 13.052, 13.126, 13.199, 13.273, 13.347,
13.421, 13.495, 13.569, 13.644, 13.718, 13.792, 13.866, 13.941, 14.015, 14.090,
14.164, 14.239, 14.313, 14.388, 14.463, 14.537, 14.612, 14.687, 14.762, 14.837,
14.912, 14.987, 15.062, 15.137, 15.212, 15.287, 15.362, 15.438, 15.513, 15.588,
15.664, 15.739, 15.815, 15.890, 15.966, 16.041, 16.117, 16.193, 16.269, 16.344,
16.420, 16.496, 16.572, 16.648, 16.724, 16.800, 16.876, 16.952, 17.028, 17.104,
17.181, 17.257, 17.333, 17.409, 17.486, 17.562, 17.639, 17.715, 17.792, 17.868,
17.945, 18.021, 18.098, 18.175, 18.252, 18.328, 18.405, 18.482, 18.559, 18.636,
18.713, 18.790, 18.867, 18.944, 19.021, 19.098, 19.175, 19.252, 19.330, 19.407,
19.484, 19.561, 19.639, 19.716, 19.794, 19.871, 19.948, 20.026, 20.103, 20.181,
20.259, 20.336, 20.414, 20.492, 20.569, 20.647, 20.725, 20.803, 20.880, 20.958,
21.036, 21.114, 21.192, 21.270, 21.348, 21.426, 21.504, 21.582, 21.660, 21.739,
21.817, 21.895, 21.973, 22.051, 22.130, 22.208, 22.286, 22.365, 22.443, 22.522,
22.600, 22.678, 22.757, 22.835, 22.914, 22.993, 23.071, 23.150, 23.228, 23.307,
23.386, 23.464, 23.543, 23.622, 23.701, 23.780, 23.858, 23.937, 24.016, 24.095,
24.174, 24.253, 24.332, 24.411, 24.490, 24.569, 24.648, 24.727, 24.806, 24.885,
24.964, 25.044, 25.123, 25.202, 25.281, 25.360, 25.440, 25.519, 25.598, 25.678,
25.757, 25.836, 25.916, 25.995, 26.075, 26.154, 26.233, 26.313, 26.392, 26.472,
26.552, 26.631, 26.711, 26.790, 26.870, 26.950, 27.029, 27.109, 27.189, 27.268,
27.348, 27.428, 27.507, 27.587, 27.667, 27.747, 27.827, 27.907, 27.986, 28.066,
28.146, 28.226, 28.306, 28.386, 28.466, 28.546, 28.626, 28.706, 28.786, 28.866,
28.946, 29.026, 29.106, 29.186, 29.266, 29.346, 29.427, 29.507, 29.587, 29.667,
29.747, 29.827, 29.908, 29.988, 30.068, 30.148, 30.229, 30.309, 30.389, 30.470,
30.550, 30.630, 30.711, 30.791, 30.871, 30.952, 31.032, 31.112, 31.193, 31.273,
31.354, 31.434, 31.515, 31.595, 31.676, 31.756, 31.837, 31.917, 31.998, 32.078,
32.159, 32.239, 32.320, 32.400, 32.481, 32.562, 32.642, 32.723, 32.803, 32.884,
32.965, 33.045, 33.126, 33.207, 33.287, 33.368, 33.449, 33.529, 33.610, 33.691,
33.772, 33.852, 33.933, 34.014, 34.095, 34.175, 34.256, 34.337, 34.418, 34.498,
34.579, 34.660, 34.741, 34.822, 34.902, 34.983, 35.064, 35.145, 35.226, 35.307,
35.387, 35.468, 35.549, 35.630, 35.711, 35.792, 35.873, 35.954, 36.034, 36.115,
36.196, 36.277, 36.358, 36.439, 36.520, 36.601, 36.682, 36.763, 36.843, 36.924,
37.005, 37.086, 37.167, 37.248, 37.329, 37.410, 37.491, 37.572, 37.653, 37.734,
37.815, 37.896, 37.977, 38.058, 38.139, 38.220, 38.300, 38.381, 38.462, 38.543,
38.624, 38.705, 38.786, 38.867, 38.948, 39.029, 39.110, 39.191, 39.272, 39.353,
39.434, 39.515, 39.596, 39.677, 39.758, 39.839, 39.920, 40.001, 40.082, 40.163,
40.243, 40.324, 40.405, 40.486, 40.567, 40.648, 40.729, 40.810, 40.891, 40.972,
41.053, 41.134, 41.215, 41.296, 41.377, 41.457, 41.538, 41.619, 41.700, 41.781,
41.862, 41.943, 42.024, 42.105, 42.185, 42.266, 42.347, 42.428, 42.509, 42.590,
42.671, 42.751, 42.832, 42.913, 42.994, 43.075, 43.156, 43.236, 43.317, 43.398,
43.479, 43.560, 43.640, 43.721, 43.802, 43.883, 43.963, 44.044, 44.125, 44.206,
44.286, 44.367, 44.448, 44.529, 44.609, 44.690, 44.771, 44.851, 44.932, 45.013,
45.093, 45.174, 45.255, 45.335, 45.416, 45.497, 45.577, 45.658, 45.738, 45.819,
45.900, 45.980, 46.061, 46.141, 46.222, 46.302, 46.383, 46.463, 46.544, 46.624,
46.705, 46.785, 46.866, 46.946, 47.027, 47.107, 47.188, 47.268, 47.349, 47.429,
47.509, 47.590, 47.670, 47.751, 47.831, 47.911, 47.992, 48.072, 48.152, 48.233,
48.313, 48.393, 48.474, 48.554, 48.634, 48.715, 48.795, 48.875, 48.955, 49.035,
49.116, 49.196, 49.276, 49.356, 49.436, 49.517, 49.597, 49.677, 49.757, 49.837,
49.917, 49.997, 50.077, 50.157, 50.238, 50.318, 50.398, 50.478, 50.558, 50.638,
50.718, 50.798, 50.878, 50.958, 51.038, 51.118, 51.197, 51.277, 51.357, 51.437,
51.517, 51.597, 51.677, 51.757, 51.837, 51.916, 51.996, 52.076, 52.156, 52.236,
52.315, 52.395, 52.475, 52.555, 52.634, 52.714, 52.794, 52.873, 52.953, 53.033,
53.112, 53.192, 53.272, 53.351, 53.431, 53.510, 53.590, 53.670, 53.749, 53.829,
53.908, 53.988, 54.067, 54.147, 54.226, 54.306, 54.385, 54.465, 54.544, 54.624,
54.703, 54.782, 54.862, 54.941, 55.021, 55.100, 55.179, 55.259, 55.338, 55.417,
55.497, 55.576, 55.655, 55.734, 55.814, 55.893, 55.972, 56.051, 56.131, 56.210,
56.289, 56.368, 56.447, 56.526, 56.606, 56.685, 56.764, 56.843, 56.922, 57.001,
57.080, 57.159, 57.238, 57.317, 57.396, 57.475, 57.554, 57.633, 57.712, 57.791,
57.870, 57.949, 58.028, 58.107, 58.186, 58.265, 58.343, 58.422, 58.501, 58.580,
58.659, 58.738, 58.816, 58.895, 58.974, 59.053, 59.131, 59.210, 59.289, 59.367,
59.446, 59.525, 59.604, 59.682, 59.761, 59.839, 59.918, 59.997, 60.075, 60.154,
60.232, 60.311, 60.390, 60.468, 60.547, 60.625, 60.704, 60.782, 60.860, 60.939,
61.017, 61.096, 61.174, 61.253, 61.331, 61.409, 61.488, 61.566, 61.644, 61.723,
61.801, 61.879, 61.958, 62.036, 62.114, 62.192, 62.271, 62.349, 62.427, 62.505,
62.583, 62.662, 62.740, 62.818, 62.896, 62.974, 63.052, 63.130, 63.208, 63.286,
63.364, 63.442, 63.520, 63.598, 63.676, 63.754, 63.832, 63.910, 63.988, 64.066,
64.144, 64.222, 64.300, 64.377, 64.455, 64.533, 64.611, 64.689, 64.766, 64.844,
64.922, 65.000, 65.077, 65.155, 65.233, 65.310, 65.388, 65.465, 65.543, 65.621,
65.698, 65.776, 65.853, 65.931, 66.008, 66.086, 66.163, 66.241, 66.318, 66.396,
66.473, 66.550, 66.628, 66.705, 66.782, 66.860, 66.937, 67.014, 67.092, 67.169,
67.246, 67.323, 67.400, 67.478, 67.555, 67.632, 67.709, 67.786, 67.863, 67.940,
68.017, 68.094, 68.171, 68.248, 68.325, 68.402, 68.479, 68.556, 68.633, 68.710,
68.787, 68.863, 68.940, 69.017, 69.094, 69.171, 69.247, 69.324, 69.401, 69.477,
69.554, 69.631, 69.707, 69.784, 69.860, 69.937, 70.013, 70.090, 70.166, 70.243,
70.319, 70.396, 70.472, 70.548, 70.625, 70.701, 70.777, 70.854, 70.930, 71.006,
71.082, 71.159, 71.235, 71.311, 71.387, 71.463, 71.539, 71.615, 71.692, 71.768,
71.844, 71.920, 71.996, 72.072, 72.147, 72.223, 72.299, 72.375, 72.451, 72.527,
72.603, 72.678, 72.754, 72.830, 72.906, 72.981, 73.057, 73.133, 73.208, 73.284,
73.360, 73.435, 73.511, 73.586, 73.662, 73.738, 73.813, 73.889, 73.964, 74.040,
74.115, 74.190, 74.266, 74.341, 74.417, 74.492, 74.567, 74.643, 74.718, 74.793,
74.869, 74.944, 75.019, 75.095, 75.170, 75.245, 75.320, 75.395, 75.471, 75.546,
75.621, 75.696, 75.771, 75.847, 75.922, 75.997, 76.072, 76.147, 76.223, 76.298,
76.373};


#define THERMOCOUPLE_TABLE_T_SIZE 671
//-270 to 400 in degrees celcius increments
const double thermocouple_table_t_type[THERMOCOUPLE_TABLE_T_SIZE] = {
-6.258, -6.256, -6.255, -6.253, -6.251, -6.248, -6.245, -6.242, -6.239, -6.236,
-6.232, -6.228, -6.223, -6.219, -6.214, -6.209, -6.204, -6.198, -6.193, -6.187,
-6.180, -6.174, -6.167, -6.160, -6.153, -6.146, -6.138, -6.130, -6.122, -6.114,
-6.105, -6.096, -6.087, -6.078, -6.068, -6.059, -6.049, -6.038, -6.028, -6.017,
-6.007, -5.996, -5.985, -5.973, -5.962, -5.950, -5.938, -5.926, -5.914, -5.901,
-5.888, -5.876, -5.863, -5.850, -5.836, -5.823, -5.809, -5.795, -5.782, -5.767,
-5.753, -5.739, -5.724, -5.710, -5.695, -5.680, -5.665, -5.650, -5.634, -5.619,
-5.603, -5.587, -5.571, -5.555, -5.539, -5.523, -5.506, -5.489, -5.473, -5.456,
-5.439, -5.421, -5.404, -5.387, -5.369, -5.351, -5.334, -5.316, -5.297, -5.279,
-5.261, -5.242, -5.224, -5.205, -5.186, -5.167, -5.148, -5.128, -5.109, -5.089,
-5.070, -5.050, -5.030, -5.010, -4.989, -4.969, -4.949, -4.928, -4.907, -4.886,
-4.865, -4.844, -4.823, -4.802, -4.780, -4.759, -4.737, -4.715, -4.693, -4.671,
-4.648, -4.626, -4.604, -4.581, -4.558, -4.535, -4.512, -4.489, -4.466, -4.443,
-4.419, -4.395, -4.372, -4.348, -4.324, -4.300, -4.275, -4.251, -4.226, -4.202,
-4.177, -4.152, -4.127, -4.102, -4.077, -4.052, -4.026, -4.000, -3.975, -3.949,
-3.923, -3.897, -3.871, -3.844, -3.818, -3.791, -3.765, -3.738, -3.711, -3.684,
-3.657, -3.629, -3.602, -3.574, -3.547, -3.519, -3.491, -3.463, -3.435, -3.407,
-3.379, -3.350, -3.322, -3.293, -3.264, -3.235, -3.206, -3.177, -3.148, -3.118,
-3.089, -3.059, -3.030, -3.000, -2.970, -2.940, -2.910, -2.879, -2.849, -2.818,
-2.788, -2.757, -2.726, -2.695, -2.664, -2.633, -2.602, -2.571, -2.539, -2.507,
-2.476, -2.444, -2.412, -2.380, -2.348, -2.316, -2.283, -2.251, -2.218, -2.186,
-2.153, -2.120, -2.087, -2.054, -2.021, -1.987, -1.954, -1.920, -1.887, -1.853,
-1.819, -1.785, -1.751, -1.717, -1.683, -1.648, -1.614, -1.579, -1.545, -1.510,
-1.475, -1.440, -1.405, -1.370, -1.335, -1.299, -1.264, -1.228, -1.192, -1.157,
-1.121, -1.085, -1.049, -1.013, -0.976, -0.940, -0.904, -0.867, -0.830, -0.794,
-0.757, -0.720, -0.683, -0.646, -0.608, -0.571, -0.534, -0.496, -0.459, -0.421,
-0.383, -0.345, -0.307, -0.269, -0.231, -0.193, -0.154, -0.116, -0.077, -0.039,
0.000, 0.039, 0.078, 0.117, 0.156, 0.195, 0.234, 0.273, 0.312, 0.352,
0.391, 0.431, 0.470, 0.510, 0.549, 0.589, 0.629, 0.669, 0.709, 0.749,
0.790, 0.830, 0.870, 0.911, 0.951, 0.992, 1.033, 1.074, 1.114, 1.155,
1.196, 1.238, 1.279, 1.320, 1.362, 1.403, 1.445, 1.486, 1.528, 1.570,
1.612, 1.654, 1.696, 1.738, 1.780, 1.823, 1.865, 1.908, 1.950, 1.993,
2.036, 2.079, 2.122, 2.165, 2.208, 2.251, 2.294, 2.338, 2.381, 2.425,
2.468, 2.512, 2.556, 2.600, 2.643, 2.687, 2.732, 2.776, 2.820, 2.864,
2.909, 2.953, 2.998, 3.043, 3.087, 3.132, 3.177, 3.222, 3.267, 3.312,
3.358, 3.403, 3.448, 3.494, 3.539, 3.585, 3.631, 3.677, 3.722, 3.768,
3.814, 3.860, 3.907, 3.953, 3.999, 4.046, 4.092, 4.138, 4.185, 4.232,
4.279, 4.325, 4.372, 4.419, 4.466, 4.513, 4.561, 4.608, 4.655, 4.702,
4.750, 4.798, 4.845, 4.893, 4.941, 4.988, 5.036, 5.084, 5.132, 5.180,
5.228, 5.277, 5.325, 5.373, 5.422, 5.470, 5.519, 5.567, 5.616, 5.665,
5.714, 5.763, 5.812, 5.861, 5.910, 5.959, 6.008, 6.057, 6.107, 6.156,
6.206, 6.255, 6.305, 6.355, 6.404, 6.454, 6.504, 6.554, 6.604, 6.654,
6.704, 6.754, 6.805, 6.855, 6.905, 6.956, 7.006, 7.057, 7.107, 7.158,
7.209, 7.260, 7.310, 7.361, 7.412, 7.463, 7.515, 7.566, 7.617, 7.668,
7.720, 7.771, 7.823, 7.874, 7.926, 7.977, 8.029, 8.081, 8.133, 8.185,
8.237, 8.289, 8.341, 8.393, 8.445, 8.497, 8.550, 8.602, 8.654, 8.707,
8.759, 8.812, 8.865, 8.917, 8.970, 9.023, 9.076, 9.129, 9.182, 9.235,
9.288, 9.341, 9.395, 9.448, 9.501, 9.555, 9.608, 9.662, 9.715, 9.769,
9.822, 9.876, 9.930, 9.984, 10.038, 10.092, 10.146, 10.200, 10.254, 10.308,
10.362, 10.417, 10.471, 10.525, 10.580, 10.634, 10.689, 10.743, 10.798, 10.853,
10.907, 10.962, 11.017, 11.072, 11.127, 11.182, 11.237, 11.292, 11.347, 11.403,
11.458, 11.513, 11.569, 11.624, 11.680, 11.735, 11.791, 11.846, 11.902, 11.958,
12.013, 12.069, 12.125, 12.181, 12.237, 12.293, 12.349, 12.405, 12.461, 12.518,
12.574, 12.630, 12.687, 12.743, 12.799, 12.856, 12.912, 12.969, 13.026, 13.082,
13.139, 13.196, 13.253, 13.310, 13.366, 13.423, 13.480, 13.537, 13.595, 13.652,
13.709, 13.766, 13.823, 13.881, 13.938, 13.995, 14.053, 14.110, 14.168, 14.226,
14.283, 14.341, 14.399, 14.456, 14.514, 14.572, 14.630, 14.688, 14.746, 14.804,
14.862, 14.920, 14.978, 15.036, 15.095, 15.153, 15.211, 15.270, 15.328, 15.386,
15.445, 15.503, 15.562, 15.621, 15.679, 15.738, 15.797, 15.856, 15.914, 15.973,
16.032, 16.091, 16.150, 16.209, 16.268, 16.327, 16.387, 16.446, 16.505, 16.564,
16.624, 16.683, 16.742, 16.802, 16.861, 16.921, 16.980, 17.040, 17.100, 17.159,
17.219, 17.279, 17.339, 17.399, 17.458, 17.518, 17.578, 17.638, 17.698, 17.759,
17.819, 17.879, 17.939, 17.999, 18.060, 18.120, 18.180, 18.241, 18.301, 18.362,
18.422, 18.483, 18.543, 18.604, 18.665, 18.725, 18.786, 18.847, 18.908, 18.969,
19.030, 19.091, 19.152, 19.213, 19.274, 19.335, 19.396, 19.457, 19.518, 19.579,
19.641, 19.702, 19.763, 19.825, 19.886, 19.947, 20.009, 20.070, 20.132, 20.193,
20.255, 20.317, 20.378, 20.440, 20.502, 20.563, 20.625, 20.687, 20.748, 20.810,
20.872};


#define THERMOCOUPLE_TABLE_J_SIZE 1411
//-210 to 1200 in degrees celcius increments
const double thermocouple_table_j_type[THERMOCOUPLE_TABLE_J_SIZE] = {
-8.095, -8.076, -8.057, -8.037, -8.017, -7.996, -7.976, -7.955, -7.934, -7.912,
-7.890, -7.868, -7.846, -7.824, -7.801, -7.778, -7.755, -7.731, -7.707, -7.683,
-7.659, -7.634, -7.610, -7.585, -7.559, -7.534, -7.508, -7.482, -7.456, -7.429,
-7.403, -7.376, -7.348, -7.321, -7.293, -7.265, -7.237, -7.209, -7.181, -7.152,
-7.123, -7.094, -7.064, -7.035, -7.005, -6.975, -6.944, -6.914, -6.883, -6.853,
-6.821, -6.790, -6.759, -6.727, -6.695, -6.663, -6.631, -6.598, -6.566, -6.533,
-6.500, -6.467, -6.433, -6.400, -6.366, -6.332, -6.298, -6.263, -6.229, -6.194,
-6.159, -6.124, -6.089, -6.054, -6.018, -5.982, -5.946, -5.910, -5.874, -5.838,
-5.801, -5.764, -5.727, -5.690, -5.653, -5.616, -5.578, -5.541, -5.503, -5.465,
-5.426, -5.388, -5.350, -5.311, -5.272, -5.233, -5.194, -5.155, -5.116, -5.076,
-5.037, -4.997, -4.957, -4.917, -4.877, -4.836, -4.796, -4.755, -4.714, -4.674,
-4.633, -4.591, -4.550, -4.509, -4.467, -4.425, -4.384, -4.342, -4.300, -4.257,
-4.215, -4.173, -4.130, -4.088, -4.045, -4.002, -3.959, -3.916, -3.872, -3.829,
-3.786, -3.742, -3.698, -3.654, -3.610, -3.566, -3.522, -3.478, -3.434, -3.389,
-3.344, -3.300, -3.255, -3.210, -3.165, -3.120, -3.075, -3.029, -2.984, -2.938,
-2.893, -2.847, -2.801, -2.755, -2.709, -2.663, -2.617, -2.571, -2.524, -2.478,
-2.431, -2.385, -2.338, -2.291, -2.244, -2.197, -2.150, -2.103, -2.055, -2.008,
-1.961, -1.913, -1.865, -1.818, -1.770, -1.722, -1.674, -1.626, -1.578, -1.530,
-1.482, -1.433, -1.385, -1.336, -1.288, -1.239, -1.190, -1.142, -1.093, -1.044,
-0.995, -0.946, -0.896, -0.847, -0.798, -0.749, -0.699, -0.650, -0.600, -0.550,
-0.501, -0.451, -0.401, -0.351, -0.301, -0.251, -0.201, -0.151, -0.101, -0.050,
0.000, 0.050, 0.101, 0.151, 0.202, 0.253, 0.303, 0.354, 0.405, 0.456,
0.507, 0.558, 0.609, 0.660, 0.711, 0.762, 0.814, 0.865, 0.916, 0.968,
1.019, 1.071, 1.122, 1.174, 1.226, 1.277, 1.329, 1.381, 1.433, 1.485,
1.537, 1.589, 1.641, 1.693, 1.745, 1.797, 1.849, 1.902, 1.954, 2.006,
2.059, 2.111, 2.164, 2.216, 2.269, 2.322, 2.374, 2.427, 2.480, 2.532,
2.585, 2.638, 2.691, 2.744, 2.797, 2.850, 2.903, 2.956, 3.009, 3.062,
3.116, 3.169, 3.222, 3.275, 3.329, 3.382, 3.436, 3.489, 3.543, 3.596,
3.650, 3.703, 3.757, 3.810, 3.864, 3.918, 3.971, 4.025, 4.079, 4.133,
4.187, 4.240, 4.294, 4.348, 4.402, 4.456, 4.510, 4.564, 4.618, 4.672,
4.726, 4.781, 4.835, 4.889, 4.943, 4.997, 5.052, 5.106, 5.160, 5.215,
5.269, 5.323, 5.378, 5.432, 5.487, 5.541, 5.595, 5.650, 5.705, 5.759,
5.814, 5.868, 5.923, 5.977, 6.032, 6.087, 6.141, 6.196, 6.251, 6.306,
6.360, 6.415, 6.470, 6.525, 6.579, 6.634, 6.689, 6.744, 6.799, 6.854,
6.909, 6.964, 7.019, 7.074, 7.129, 7.184, 7.239, 7.294, 7.349, 7.404,
7.459, 7.514, 7.569, 7.624, 7.679, 7.734, 7.789, 7.844, 7.900, 7.955,
8.010, 8.065, 8.120, 8.175, 8.231, 8.286, 8.341, 8.396, 8.452, 8.507,
8.562, 8.618, 8.673, 8.728, 8.783, 8.839, 8.894, 8.949, 9.005, 9.060,
9.115, 9.171, 9.226, 9.282, 9.337, 9.392, 9.448, 9.503, 9.559, 9.614,
9.669, 9.725, 9.780, 9.836, 9.891, 9.947, 10.002, 10.057, 10.113, 10.168,
10.224, 10.279, 10.335, 10.390, 10.446, 10.501, 10.557, 10.612, 10.668, 10.723,
10.779, 10.834, 10.890, 10.945, 11.001, 11.056, 11.112, 11.167, 11.223, 11.278,
11.334, 11.389, 11.445, 11.501, 11.556, 11.612, 11.667, 11.723, 11.778, 11.834,
11.889, 11.945, 12.000, 12.056, 12.111, 12.167, 12.222, 12.278, 12.334, 12.389,
12.445, 12.500, 12.556, 12.611, 12.667, 12.722, 12.778, 12.833, 12.889, 12.944,
13.000, 13.056, 13.111, 13.167, 13.222, 13.278, 13.333, 13.389, 13.444, 13.500,
13.555, 13.611, 13.666, 13.722, 13.777, 13.833, 13.888, 13.944, 13.999, 14.055,
14.110, 14.166, 14.221, 14.277, 14.332, 14.388, 14.443, 14.499, 14.554, 14.609,
14.665, 14.720, 14.776, 14.831, 14.887, 14.942, 14.998, 15.053, 15.109, 15.164,
15.219, 15.275, 15.330, 15.386, 15.441, 15.496, 15.552, 15.607, 15.663, 15.718,
15.773, 15.829, 15.884, 15.940, 15.995, 16.050, 16.106, 16.161, 16.216, 16.272,
16.327, 16.383, 16.438, 16.493, 16.549, 16.604, 16.659, 16.715, 16.770, 16.825,
16.881, 16.936, 16.991, 17.046, 17.102, 17.157, 17.212, 17.268, 17.323, 17.378,
17.434, 17.489, 17.544, 17.599, 17.655, 17.710, 17.765, 17.820, 17.876, 17.931,
17.986, 18.041, 18.097, 18.152, 18.207, 18.262, 18.318, 18.373, 18.428, 18.483,
18.538, 18.594, 18.649, 18.704, 18.759, 18.814, 18.870, 18.925, 18.980, 19.035,
19.090, 19.146, 19.201, 19.256, 19.311, 19.366, 19.422, 19.477, 19.532, 19.587,
19.642, 19.697, 19.753, 19.808, 19.863, 19.918, 19.973, 20.028, 20.083, 20.139,
20.194, 20.249, 20.304, 20.359, 20.414, 20.469, 20.525, 20.580, 20.635, 20.690,
20.745, 20.800, 20.855, 20.911, 20.966, 21.021, 21.076, 21.131, 21.186, 21.241,
21.297, 21.352, 21.407, 21.462, 21.517, 21.572, 21.627, 21.683, 21.738, 21.793,
21.848, 21.903, 21.958, 22.014, 22.069, 22.124, 22.179, 22.234, 22.289, 22.345,
22.400, 22.455, 22.510, 22.565, 22.620, 22.676, 22.731, 22.786, 22.841, 22.896,
22.952, 23.007, 23.062, 23.117, 23.172, 23.228, 23.283, 23.338, 23.393, 23.449,
23.504, 23.559, 23.614, 23.670, 23.725, 23.780, 23.835, 23.891, 23.946, 24.001,
24.057, 24.112, 24.167, 24.223, 24.278, 24.333, 24.389, 24.444, 24.499, 24.555,
24.610, 24.665, 24.721, 24.776, 24.832, 24.887, 24.943, 24.998, 25.053, 25.109,
25.164, 25.220, 25.275, 25.331, 25.386, 25.442, 25.497, 25.553, 25.608, 25.664,
25.720, 25.775, 25.831, 25.886, 25.942, 25.998, 26.053, 26.109, 26.165, 26.220,
26.276, 26.332, 26.387, 26.443, 26.499, 26.555, 26.610, 26.666, 26.722, 26.778,
26.834, 26.889, 26.945, 27.001, 27.057, 27.113, 27.169, 27.225, 27.281, 27.337,
27.393, 27.449, 27.505, 27.561, 27.617, 27.673, 27.729, 27.785, 27.841, 27.897,
27.953, 28.010, 28.066, 28.122, 28.178, 28.234, 28.291, 28.347, 28.403, 28.460,
28.516, 28.572, 28.629, 28.685, 28.741, 28.798, 28.854, 28.911, 28.967, 29.024,
29.080, 29.137, 29.194, 29.250, 29.307, 29.363, 29.420, 29.477, 29.534, 29.590,
29.647, 29.704, 29.761, 29.818, 29.874, 29.931, 29.988, 30.045, 30.102, 30.159,
30.216, 30.273, 30.330, 30.387, 30.444, 30.502, 30.559, 30.616, 30.673, 30.730,
30.788, 30.845, 30.902, 30.960, 31.017, 31.074, 31.132, 31.189, 31.247, 31.304,
31.362, 31.419, 31.477, 31.535, 31.592, 31.650, 31.708, 31.766, 31.823, 31.881,
31.939, 31.997, 32.055, 32.113, 32.171, 32.229, 32.287, 32.345, 32.403, 32.461,
32.519, 32.577, 32.636, 32.694, 32.752, 32.810, 32.869, 32.927, 32.985, 33.044,
33.102, 33.161, 33.219, 33.278, 33.337, 33.395, 33.454, 33.513, 33.571, 33.630,
33.689, 33.748, 33.807, 33.866, 33.925, 33.984, 34.043, 34.102, 34.161, 34.220,
34.279, 34.338, 34.397, 34.457, 34.516, 34.575, 34.635, 34.694, 34.754, 34.813,
34.873, 34.932, 34.992, 35.051, 35.111, 35.171, 35.230, 35.290, 35.350, 35.410,
35.470, 35.530, 35.590, 35.650, 35.710, 35.770, 35.830, 35.890, 35.950, 36.010,
36.071, 36.131, 36.191, 36.252, 36.312, 36.373, 36.433, 36.494, 36.554, 36.615,
36.675, 36.736, 36.797, 36.858, 36.918, 36.979, 37.040, 37.101, 37.162, 37.223,
37.284, 37.345, 37.406, 37.467, 37.528, 37.590, 37.651, 37.712, 37.773, 37.835,
37.896, 37.958, 38.019, 38.081, 38.142, 38.204, 38.265, 38.327, 38.389, 38.450,
38.512, 38.574, 38.636, 38.698, 38.760, 38.822, 38.884, 38.946, 39.008, 39.070,
39.132, 39.194, 39.256, 39.318, 39.381, 39.443, 39.505, 39.568, 39.630, 39.693,
39.755, 39.818, 39.880, 39.943, 40.005, 40.068, 40.131, 40.193, 40.256, 40.319,
40.382, 40.445, 40.508, 40.570, 40.633, 40.696, 40.759, 40.822, 40.886, 40.949,
41.012, 41.075, 41.138, 41.201, 41.265, 41.328, 41.391, 41.455, 41.518, 41.581,
41.645, 41.708, 41.772, 41.835, 41.899, 41.962, 42.026, 42.090, 42.153, 42.217,
42.281, 42.344, 42.408, 42.472, 42.536, 42.599, 42.663, 42.727, 42.791, 42.855,
42.919, 42.983, 43.047, 43.111, 43.175, 43.239, 43.303, 43.367, 43.431, 43.495,
43.559, 43.624, 43.688, 43.752, 43.817, 43.881, 43.945, 44.010, 44.074, 44.139,
44.203, 44.267, 44.332, 44.396, 44.461, 44.525, 44.590, 44.655, 44.719, 44.784,
44.848, 44.913, 44.977, 45.042, 45.107, 45.171, 45.236, 45.301, 45.365, 45.430,
45.494, 45.559, 45.624, 45.688, 45.753, 45.818, 45.882, 45.947, 46.011, 46.076,
46.141, 46.205, 46.270, 46.334, 46.399, 46.464, 46.528, 46.593, 46.657, 46.722,
46.786, 46.851, 46.915, 46.980, 47.044, 47.109, 47.173, 47.238, 47.302, 47.367,
47.431, 47.495, 47.560, 47.624, 47.688, 47.753, 47.817, 47.881, 47.946, 48.010,
48.074, 48.138, 48.202, 48.267, 48.331, 48.395, 48.459, 48.523, 48.587, 48.651,
48.715, 48.779, 48.843, 48.907, 48.971, 49.034, 49.098, 49.162, 49.226, 49.290,
49.353, 49.417, 49.481, 49.544, 49.608, 49.672, 49.735, 49.799, 49.862, 49.926,
49.989, 50.052, 50.116, 50.179, 50.243, 50.306, 50.369, 50.432, 50.495, 50.559,
50.622, 50.685, 50.748, 50.811, 50.874, 50.937, 51.000, 51.063, 51.126, 51.188,
51.251, 51.314, 51.377, 51.439, 51.502, 51.565, 51.627, 51.690, 51.752, 51.815,
51.877, 51.940, 52.002, 52.064, 52.127, 52.189, 52.251, 52.314, 52.376, 52.438,
52.500, 52.562, 52.624, 52.686, 52.748, 52.810, 52.872, 52.934, 52.996, 53.057,
53.119, 53.181, 53.243, 53.304, 53.366, 53.427, 53.489, 53.550, 53.612, 53.673,
53.735, 53.796, 53.857, 53.919, 53.980, 54.041, 54.102, 54.164, 54.225, 54.286,
54.347, 54.408, 54.469, 54.530, 54.591, 54.652, 54.713, 54.773, 54.834, 54.895,
54.956, 55.016, 55.077, 55.138, 55.198, 55.259, 55.319, 55.380, 55.440, 55.501,
55.561, 55.622, 55.682, 55.742, 55.803, 55.863, 55.923, 55.983, 56.043, 56.104,
56.164, 56.224, 56.284, 56.344, 56.404, 56.464, 56.524, 56.584, 56.643, 56.703,
56.763, 56.823, 56.883, 56.942, 57.002, 57.062, 57.121, 57.181, 57.240, 57.300,
57.360, 57.419, 57.479, 57.538, 57.597, 57.657, 57.716, 57.776, 57.835, 57.894,
57.953, 58.013, 58.072, 58.131, 58.190, 58.249, 58.309, 58.368, 58.427, 58.486,
58.545, 58.604, 58.663, 58.722, 58.781, 58.840, 58.899, 58.957, 59.016, 59.075,
59.134, 59.193, 59.252, 59.310, 59.369, 59.428, 59.487, 59.545, 59.604, 59.663,
59.721, 59.780, 59.838, 59.897, 59.956, 60.014, 60.073, 60.131, 60.190, 60.248,
60.307, 60.365, 60.423, 60.482, 60.540, 60.599, 60.657, 60.715, 60.774, 60.832,
60.890, 60.949, 61.007, 61.065, 61.123, 61.182, 61.240, 61.298, 61.356, 61.415,
61.473, 61.531, 61.589, 61.647, 61.705, 61.763, 61.822, 61.880, 61.938, 61.996,
62.054, 62.112, 62.170, 62.228, 62.286, 62.344, 62.402, 62.460, 62.518, 62.576,
62.634, 62.692, 62.750, 62.808, 62.866, 62.924, 62.982, 63.040, 63.098, 63.156,
63.214, 63.271, 63.329, 63.387, 63.445, 63.503, 63.561, 63.619, 63.677, 63.734,
63.792, 63.850, 63.908, 63.966, 64.024, 64.081, 64.139, 64.197, 64.255, 64.313,
64.370, 64.428, 64.486, 64.544, 64.602, 64.659, 64.717, 64.775, 64.833, 64.890,
64.948, 65.006, 65.064, 65.121, 65.179, 65.237, 65.295, 65.352, 65.410, 65.468,
65.525, 65.583, 65.641, 65.699, 65.756, 65.814, 65.872, 65.929, 65.987, 66.045,
66.102, 66.160, 66.218, 66.275, 66.333, 66.391, 66.448, 66.506, 66.564, 66.621,
66.679, 66.737, 66.794, 66.852, 66.910, 66.967, 67.025, 67.082, 67.140, 67.198,
67.255, 67.313, 67.370, 67.428, 67.486, 67.543, 67.601, 67.658, 67.716, 67.773,
67.831, 67.888, 67.946, 68.003, 68.061, 68.119, 68.176, 68.234, 68.291, 68.348,
68.406, 68.463, 68.521, 68.578, 68.636, 68.693, 68.751, 68.808, 68.865, 68.923,
68.980, 69.037, 69.095, 69.152, 69.209, 69.267, 69.324, 69.381, 69.439, 69.496,
69.553};

#define THERMOCOUPLE_TABLE_K_SIZE 1643
//-270 to 1372 in degrees celcius increments
const double thermocouple_table_k_type[THERMOCOUPLE_TABLE_K_SIZE] = {
-6.458, -6.457, -6.456, -6.455, -6.453, -6.452, -6.450, -6.448, -6.446, -6.444,
-6.441, -6.438, -6.435, -6.432, -6.429, -6.425, -6.421, -6.417, -6.413, -6.408,
-6.404, -6.399, -6.393, -6.388, -6.382, -6.377, -6.370, -6.364, -6.358, -6.351,
-6.344, -6.337, -6.329, -6.322, -6.314, -6.306, -6.297, -6.289, -6.280, -6.271,
-6.262, -6.252, -6.243, -6.233, -6.223, -6.213, -6.202, -6.192, -6.181, -6.170,
-6.158, -6.147, -6.135, -6.123, -6.111, -6.099, -6.087, -6.074, -6.061, -6.048,
-6.035, -6.021, -6.007, -5.994, -5.980, -5.965, -5.951, -5.936, -5.922, -5.907,
-5.891, -5.876, -5.861, -5.845, -5.829, -5.813, -5.797, -5.780, -5.763, -5.747,
-5.730, -5.713, -5.695, -5.678, -5.660, -5.642, -5.624, -5.606, -5.588, -5.569,
-5.550, -5.531, -5.512, -5.493, -5.474, -5.454, -5.435, -5.415, -5.395, -5.374,
-5.354, -5.333, -5.313, -5.292, -5.271, -5.250, -5.228, -5.207, -5.185, -5.163,
-5.141, -5.119, -5.097, -5.074, -5.052, -5.029, -5.006, -4.983, -4.960, -4.936,
-4.913, -4.889, -4.865, -4.841, -4.817, -4.793, -4.768, -4.744, -4.719, -4.694,
-4.669, -4.644, -4.618, -4.593, -4.567, -4.542, -4.516, -4.490, -4.463, -4.437,
-4.411, -4.384, -4.357, -4.330, -4.303, -4.276, -4.249, -4.221, -4.194, -4.166,
-4.138, -4.110, -4.082, -4.054, -4.025, -3.997, -3.968, -3.939, -3.911, -3.882,
-3.852, -3.823, -3.794, -3.764, -3.734, -3.705, -3.675, -3.645, -3.614, -3.584,
-3.554, -3.523, -3.492, -3.462, -3.431, -3.400, -3.368, -3.337, -3.306, -3.274,
-3.243, -3.211, -3.179, -3.147, -3.115, -3.083, -3.050, -3.018, -2.986, -2.953,
-2.920, -2.887, -2.854, -2.821, -2.788, -2.755, -2.721, -2.688, -2.654, -2.620,
-2.587, -2.553, -2.519, -2.485, -2.450, -2.416, -2.382, -2.347, -2.312, -2.278,
-2.243, -2.208, -2.173, -2.138, -2.103, -2.067, -2.032, -1.996, -1.961, -1.925,
-1.889, -1.854, -1.818, -1.782, -1.745, -1.709, -1.673, -1.637, -1.600, -1.564,
-1.527, -1.490, -1.453, -1.417, -1.380, -1.343, -1.305, -1.268, -1.231, -1.194,
-1.156, -1.119, -1.081, -1.043, -1.006, -0.968, -0.930, -0.892, -0.854, -0.816,
-0.778, -0.739, -0.701, -0.663, -0.624, -0.586, -0.547, -0.508, -0.470, -0.431,
-0.392, -0.353, -0.314, -0.275, -0.236, -0.197, -0.157, -0.118, -0.079, -0.039,
0.000, 0.039, 0.079, 0.119, 0.158, 0.198, 0.238, 0.277, 0.317, 0.357,
0.397, 0.437, 0.477, 0.517, 0.557, 0.597, 0.637, 0.677, 0.718, 0.758,
0.798, 0.838, 0.879, 0.919, 0.960, 1.000, 1.041, 1.081, 1.122, 1.163,
1.203, 1.244, 1.285, 1.326, 1.366, 1.407, 1.448, 1.489, 1.530, 1.571,
1.612, 1.653, 1.694, 1.735, 1.776, 1.817, 1.858, 1.899, 1.941, 1.982,
2.023, 2.064, 2.106, 2.147, 2.188, 2.230, 2.271, 2.312, 2.354, 2.395,
2.436, 2.478, 2.519, 2.561, 2.602, 2.644, 2.685, 2.727, 2.768, 2.810,
2.851, 2.893, 2.934, 2.976, 3.017, 3.059, 3.100, 3.142, 3.184, 3.225,
3.267, 3.308, 3.350, 3.391, 3.433, 3.474, 3.516, 3.557, 3.599, 3.640,
3.682, 3.723, 3.765, 3.806, 3.848, 3.889, 3.931, 3.972, 4.013, 4.055,
4.096, 4.138, 4.179, 4.220, 4.262, 4.303, 4.344, 4.385, 4.427, 4.468,
4.509, 4.550, 4.591, 4.633, 4.674, 4.715, 4.756, 4.797, 4.838, 4.879,
4.920, 4.961, 5.002, 5.043, 5.084, 5.124, 5.165, 5.206, 5.247, 5.288,
5.328, 5.369, 5.410, 5.450, 5.491, 5.532, 5.572, 5.613, 5.653, 5.694,
5.735, 5.775, 5.815, 5.856, 5.896, 5.937, 5.977, 6.017, 6.058, 6.098,
6.138, 6.179, 6.219, 6.259, 6.299, 6.339, 6.380, 6.420, 6.460, 6.500,
6.540, 6.580, 6.620, 6.660, 6.701, 6.741, 6.781, 6.821, 6.861, 6.901,
6.941, 6.981, 7.021, 7.060, 7.100, 7.140, 7.180, 7.220, 7.260, 7.300,
7.340, 7.380, 7.420, 7.460, 7.500, 7.540, 7.579, 7.619, 7.659, 7.699,
7.739, 7.779, 7.819, 7.859, 7.899, 7.939, 7.979, 8.019, 8.059, 8.099,
8.138, 8.178, 8.218, 8.258, 8.298, 8.338, 8.378, 8.418, 8.458, 8.499,
8.539, 8.579, 8.619, 8.659, 8.699, 8.739, 8.779, 8.819, 8.860, 8.900,
8.940, 8.980, 9.020, 9.061, 9.101, 9.141, 9.181, 9.222, 9.262, 9.302,
9.343, 9.383, 9.423, 9.464, 9.504, 9.545, 9.585, 9.626, 9.666, 9.707,
9.747, 9.788, 9.828, 9.869, 9.909, 9.950, 9.991, 10.031, 10.072, 10.113,
10.153, 10.194, 10.235, 10.276, 10.316, 10.357, 10.398, 10.439, 10.480, 10.520,
10.561, 10.602, 10.643, 10.684, 10.725, 10.766, 10.807, 10.848, 10.889, 10.930,
10.971, 11.012, 11.053, 11.094, 11.135, 11.176, 11.217, 11.259, 11.300, 11.341,
11.382, 11.423, 11.465, 11.506, 11.547, 11.588, 11.630, 11.671, 11.712, 11.753,
11.795, 11.836, 11.877, 11.919, 11.960, 12.001, 12.043, 12.084, 12.126, 12.167,
12.209, 12.250, 12.291, 12.333, 12.374, 12.416, 12.457, 12.499, 12.540, 12.582,
12.624, 12.665, 12.707, 12.748, 12.790, 12.831, 12.873, 12.915, 12.956, 12.998,
13.040, 13.081, 13.123, 13.165, 13.206, 13.248, 13.290, 13.331, 13.373, 13.415,
13.457, 13.498, 13.540, 13.582, 13.624, 13.665, 13.707, 13.749, 13.791, 13.833,
13.874, 13.916, 13.958, 14.000, 14.042, 14.084, 14.126, 14.167, 14.209, 14.251,
14.293, 14.335, 14.377, 14.419, 14.461, 14.503, 14.545, 14.587, 14.629, 14.671,
14.713, 14.755, 14.797, 14.839, 14.881, 14.923, 14.965, 15.007, 15.049, 15.091,
15.133, 15.175, 15.217, 15.259, 15.301, 15.343, 15.385, 15.427, 15.469, 15.511,
15.554, 15.596, 15.638, 15.680, 15.722, 15.764, 15.806, 15.849, 15.891, 15.933,
15.975, 16.017, 16.059, 16.102, 16.144, 16.186, 16.228, 16.270, 16.313, 16.355,
16.397, 16.439, 16.482, 16.524, 16.566, 16.608, 16.651, 16.693, 16.735, 16.778,
16.820, 16.862, 16.904, 16.947, 16.989, 17.031, 17.074, 17.116, 17.158, 17.201,
17.243, 17.285, 17.328, 17.370, 17.413, 17.455, 17.497, 17.540, 17.582, 17.624,
17.667, 17.709, 17.752, 17.794, 17.837, 17.879, 17.921, 17.964, 18.006, 18.049,
18.091, 18.134, 18.176, 18.218, 18.261, 18.303, 18.346, 18.388, 18.431, 18.473,
18.516, 18.558, 18.601, 18.643, 18.686, 18.728, 18.771, 18.813, 18.856, 18.898,
18.941, 18.983, 19.026, 19.068, 19.111, 19.154, 19.196, 19.239, 19.281, 19.324,
19.366, 19.409, 19.451, 19.494, 19.537, 19.579, 19.622, 19.664, 19.707, 19.750,
19.792, 19.835, 19.877, 19.920, 19.962, 20.005, 20.048, 20.090, 20.133, 20.175,
20.218, 20.261, 20.303, 20.346, 20.389, 20.431, 20.474, 20.516, 20.559, 20.602,
20.644, 20.687, 20.730, 20.772, 20.815, 20.857, 20.900, 20.943, 20.985, 21.028,
21.071, 21.113, 21.156, 21.199, 21.241, 21.284, 21.326, 21.369, 21.412, 21.454,
21.497, 21.540, 21.582, 21.625, 21.668, 21.710, 21.753, 21.796, 21.838, 21.881,
21.924, 21.966, 22.009, 22.052, 22.094, 22.137, 22.179, 22.222, 22.265, 22.307,
22.350, 22.393, 22.435, 22.478, 22.521, 22.563, 22.606, 22.649, 22.691, 22.734,
22.776, 22.819, 22.862, 22.904, 22.947, 22.990, 23.032, 23.075, 23.117, 23.160,
23.203, 23.245, 23.288, 23.331, 23.373, 23.416, 23.458, 23.501, 23.544, 23.586,
23.629, 23.671, 23.714, 23.757, 23.799, 23.842, 23.884, 23.927, 23.970, 24.012,
24.055, 24.097, 24.140, 24.182, 24.225, 24.267, 24.310, 24.353, 24.395, 24.438,
24.480, 24.523, 24.565, 24.608, 24.650, 24.693, 24.735, 24.778, 24.820, 24.863,
24.905, 24.948, 24.990, 25.033, 25.075, 25.118, 25.160, 25.203, 25.245, 25.288,
25.330, 25.373, 25.415, 25.458, 25.500, 25.543, 25.585, 25.627, 25.670, 25.712,
25.755, 25.797, 25.840, 25.882, 25.924, 25.967, 26.009, 26.052, 26.094, 26.136,
26.179, 26.221, 26.263, 26.306, 26.348, 26.390, 26.433, 26.475, 26.517, 26.560,
26.602, 26.644, 26.687, 26.729, 26.771, 26.814, 26.856, 26.898, 26.940, 26.983,
27.025, 27.067, 27.109, 27.152, 27.194, 27.236, 27.278, 27.320, 27.363, 27.405,
27.447, 27.489, 27.531, 27.574, 27.616, 27.658, 27.700, 27.742, 27.784, 27.826,
27.869, 27.911, 27.953, 27.995, 28.037, 28.079, 28.121, 28.163, 28.205, 28.247,
28.289, 28.332, 28.374, 28.416, 28.458, 28.500, 28.542, 28.584, 28.626, 28.668,
28.710, 28.752, 28.794, 28.835, 28.877, 28.919, 28.961, 29.003, 29.045, 29.087,
29.129, 29.171, 29.213, 29.255, 29.297, 29.338, 29.380, 29.422, 29.464, 29.506,
29.548, 29.589, 29.631, 29.673, 29.715, 29.757, 29.798, 29.840, 29.882, 29.924,
29.965, 30.007, 30.049, 30.090, 30.132, 30.174, 30.216, 30.257, 30.299, 30.341,
30.382, 30.424, 30.466, 30.507, 30.549, 30.590, 30.632, 30.674, 30.715, 30.757,
30.798, 30.840, 30.881, 30.923, 30.964, 31.006, 31.047, 31.089, 31.130, 31.172,
31.213, 31.255, 31.296, 31.338, 31.379, 31.421, 31.462, 31.504, 31.545, 31.586,
31.628, 31.669, 31.710, 31.752, 31.793, 31.834, 31.876, 31.917, 31.958, 32.000,
32.041, 32.082, 32.124, 32.165, 32.206, 32.247, 32.289, 32.330, 32.371, 32.412,
32.453, 32.495, 32.536, 32.577, 32.618, 32.659, 32.700, 32.742, 32.783, 32.824,
32.865, 32.906, 32.947, 32.988, 33.029, 33.070, 33.111, 33.152, 33.193, 33.234,
33.275, 33.316, 33.357, 33.398, 33.439, 33.480, 33.521, 33.562, 33.603, 33.644,
33.685, 33.726, 33.767, 33.808, 33.848, 33.889, 33.930, 33.971, 34.012, 34.053,
34.093, 34.134, 34.175, 34.216, 34.257, 34.297, 34.338, 34.379, 34.420, 34.460,
34.501, 34.542, 34.582, 34.623, 34.664, 34.704, 34.745, 34.786, 34.826, 34.867,
34.908, 34.948, 34.989, 35.029, 35.070, 35.110, 35.151, 35.192, 35.232, 35.273,
35.313, 35.354, 35.394, 35.435, 35.475, 35.516, 35.556, 35.596, 35.637, 35.677,
35.718, 35.758, 35.798, 35.839, 35.879, 35.920, 35.960, 36.000, 36.041, 36.081,
36.121, 36.162, 36.202, 36.242, 36.282, 36.323, 36.363, 36.403, 36.443, 36.484,
36.524, 36.564, 36.604, 36.644, 36.685, 36.725, 36.765, 36.805, 36.845, 36.885,
36.925, 36.965, 37.006, 37.046, 37.086, 37.126, 37.166, 37.206, 37.246, 37.286,
37.326, 37.366, 37.406, 37.446, 37.486, 37.526, 37.566, 37.606, 37.646, 37.686,
37.725, 37.765, 37.805, 37.845, 37.885, 37.925, 37.965, 38.005, 38.044, 38.084,
38.124, 38.164, 38.204, 38.243, 38.283, 38.323, 38.363, 38.402, 38.442, 38.482,
38.522, 38.561, 38.601, 38.641, 38.680, 38.720, 38.760, 38.799, 38.839, 38.878,
38.918, 38.958, 38.997, 39.037, 39.076, 39.116, 39.155, 39.195, 39.235, 39.274,
39.314, 39.353, 39.393, 39.432, 39.471, 39.511, 39.550, 39.590, 39.629, 39.669,
39.708, 39.747, 39.787, 39.826, 39.866, 39.905, 39.944, 39.984, 40.023, 40.062,
40.101, 40.141, 40.180, 40.219, 40.259, 40.298, 40.337, 40.376, 40.415, 40.455,
40.494, 40.533, 40.572, 40.611, 40.651, 40.690, 40.729, 40.768, 40.807, 40.846,
40.885, 40.924, 40.963, 41.002, 41.042, 41.081, 41.120, 41.159, 41.198, 41.237,
41.276, 41.315, 41.354, 41.393, 41.431, 41.470, 41.509, 41.548, 41.587, 41.626,
41.665, 41.704, 41.743, 41.781, 41.820, 41.859, 41.898, 41.937, 41.976, 42.014,
42.053, 42.092, 42.131, 42.169, 42.208, 42.247, 42.286, 42.324, 42.363, 42.402,
42.440, 42.479, 42.518, 42.556, 42.595, 42.633, 42.672, 42.711, 42.749, 42.788,
42.826, 42.865, 42.903, 42.942, 42.980, 43.019, 43.057, 43.096, 43.134, 43.173,
43.211, 43.250, 43.288, 43.327, 43.365, 43.403, 43.442, 43.480, 43.518, 43.557,
43.595, 43.633, 43.672, 43.710, 43.748, 43.787, 43.825, 43.863, 43.901, 43.940,
43.978, 44.016, 44.054, 44.092, 44.130, 44.169, 44.207, 44.245, 44.283, 44.321,
44.359, 44.397, 44.435, 44.473, 44.512, 44.550, 44.588, 44.626, 44.664, 44.702,
44.740, 44.778, 44.816, 44.853, 44.891, 44.929, 44.967, 45.005, 45.043, 45.081,
45.119, 45.157, 45.194, 45.232, 45.270, 45.308, 45.346, 45.383, 45.421, 45.459,
45.497, 45.534, 45.572, 45.610, 45.647, 45.685, 45.723, 45.760, 45.798, 45.836,
45.873, 45.911, 45.948, 45.986, 46.024, 46.061, 46.099, 46.136, 46.174, 46.211,
46.249, 46.286, 46.324, 46.361, 46.398, 46.436, 46.473, 46.511, 46.548, 46.585,
46.623, 46.660, 46.697, 46.735, 46.772, 46.809, 46.847, 46.884, 46.921, 46.958,
46.995, 47.033, 47.070, 47.107, 47.144, 47.181, 47.218, 47.256, 47.293, 47.330,
47.367, 47.404, 47.441, 47.478, 47.515, 47.552, 47.589, 47.626, 47.663, 47.700,
47.737, 47.774, 47.811, 47.848, 47.884, 47.921, 47.958, 47.995, 48.032, 48.069,
48.105, 48.142, 48.179, 48.216, 48.252, 48.289, 48.326, 48.363, 48.399, 48.436,
48.473, 48.509, 48.546, 48.582, 48.619, 48.656, 48.692, 48.729, 48.765, 48.802,
48.838, 48.875, 48.911, 48.948, 48.984, 49.021, 49.057, 49.093, 49.130, 49.166,
49.202, 49.239, 49.275, 49.311, 49.348, 49.384, 49.420, 49.456, 49.493, 49.529,
49.565, 49.601, 49.637, 49.674, 49.710, 49.746, 49.782, 49.818, 49.854, 49.890,
49.926, 49.962, 49.998, 50.034, 50.070, 50.106, 50.142, 50.178, 50.214, 50.250,
50.286, 50.322, 50.358, 50.393, 50.429, 50.465, 50.501, 50.537, 50.572, 50.608,
50.644, 50.680, 50.715, 50.751, 50.787, 50.822, 50.858, 50.894, 50.929, 50.965,
51.000, 51.036, 51.071, 51.107, 51.142, 51.178, 51.213, 51.249, 51.284, 51.320,
51.355, 51.391, 51.426, 51.461, 51.497, 51.532, 51.567, 51.603, 51.638, 51.673,
51.708, 51.744, 51.779, 51.814, 51.849, 51.885, 51.920, 51.955, 51.990, 52.025,
52.060, 52.095, 52.130, 52.165, 52.200, 52.235, 52.270, 52.305, 52.340, 52.375,
52.410, 52.445, 52.480, 52.515, 52.550, 52.585, 52.620, 52.654, 52.689, 52.724,
52.759, 52.794, 52.828, 52.863, 52.898, 52.932, 52.967, 53.002, 53.037, 53.071,
53.106, 53.140, 53.175, 53.210, 53.244, 53.279, 53.313, 53.348, 53.382, 53.417,
53.451, 53.486, 53.520, 53.555, 53.589, 53.623, 53.658, 53.692, 53.727, 53.761,
53.795, 53.830, 53.864, 53.898, 53.932, 53.967, 54.001, 54.035, 54.069, 54.104,
54.138, 54.172, 54.206, 54.240, 54.274, 54.308, 54.343, 54.377, 54.411, 54.445,
54.479, 54.513, 54.547, 54.581, 54.615, 54.649, 54.683, 54.717, 54.751, 54.785,
54.819, 54.852, 54.886};

double lookup_temperature(double Vthermocouple, CPhidgetTemperatureSensor_ThermocoupleType type)
{
	int i, size;
	double temperature = PUNK_DBL;
	const double *thermocouple_table;
	int startingTemp = 0;
	
	switch(type)
	{	
		case PHIDGET_TEMPERATURE_SENSOR_K_TYPE:
			size = THERMOCOUPLE_TABLE_K_SIZE;
			thermocouple_table = thermocouple_table_k_type;
			break;
		case PHIDGET_TEMPERATURE_SENSOR_J_TYPE:
			size = THERMOCOUPLE_TABLE_J_SIZE;
			thermocouple_table = thermocouple_table_j_type;
			break;
		case PHIDGET_TEMPERATURE_SENSOR_E_TYPE:
			size = THERMOCOUPLE_TABLE_E_SIZE;
			thermocouple_table = thermocouple_table_e_type;
			break;
		case PHIDGET_TEMPERATURE_SENSOR_T_TYPE:
			size = THERMOCOUPLE_TABLE_T_SIZE;
			thermocouple_table = thermocouple_table_t_type;
			break;
		default:
			return PUNK_DBL;
	}
	startingTemp = thermocouple_table_range[type][0];

	//the voltage is too low
	if(Vthermocouple < thermocouple_table[0])
		return PUNK_DBL;
	
	for(i=1;i<size;i++) {
		if(thermocouple_table[i] > Vthermocouple) {
			temperature = (double)(i+(startingTemp-1)) + ((Vthermocouple - thermocouple_table[i-1]) / (thermocouple_table[i] - thermocouple_table[i-1]));
			break;
		}
	}

	//if(Vthermocouple > thermocouple_table[size-1]) then temperature will still equal PUNK_DBL;
	return temperature;
}

double lookup_voltage(double Tjunction, CPhidgetTemperatureSensor_ThermocoupleType type)
{
	double voltage;
	const double *thermocouple_table;
	int startingTemp = 0, size = 0;
	
	switch(type)
	{	
		case PHIDGET_TEMPERATURE_SENSOR_K_TYPE:
			size = THERMOCOUPLE_TABLE_K_SIZE;
			thermocouple_table = thermocouple_table_k_type;
			break;
		case PHIDGET_TEMPERATURE_SENSOR_J_TYPE:
			size = THERMOCOUPLE_TABLE_J_SIZE;
			thermocouple_table = thermocouple_table_j_type;
			break;
		case PHIDGET_TEMPERATURE_SENSOR_E_TYPE:
			size = THERMOCOUPLE_TABLE_E_SIZE;
			thermocouple_table = thermocouple_table_e_type;
			break;
		case PHIDGET_TEMPERATURE_SENSOR_T_TYPE:
			size = THERMOCOUPLE_TABLE_T_SIZE;
			thermocouple_table = thermocouple_table_t_type;
			break;
		default:
			return 0;
	}
	startingTemp = thermocouple_table_range[type][0];

	//Make sure we're not going to try looking outside of the arrays
	if(((int)Tjunction-startingTemp) < 0)
		return PUNK_DBL;
	if(((int)Tjunction-(startingTemp-1)) >= size)
		return PUNK_DBL;

	voltage = thermocouple_table[(int)Tjunction-startingTemp] + 
		((thermocouple_table[(int)Tjunction-(startingTemp-1)] - thermocouple_table[(int)Tjunction-startingTemp]) 
		* (double)(Tjunction - (int)Tjunction));

	return voltage;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(TemperatureSensor, PHIDCLASS_TEMPERATURESENSOR)

//event setup functions
CFHANDLE(TemperatureSensor, TemperatureChange, int, double)

CGET(TemperatureSensor,TemperatureInputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED

	MASGN(phid.attr.temperaturesensor.numTempInputs)
}

CGETINDEX(TemperatureSensor,Temperature,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)
	TESTMASGN(Temperature[Index], PUNK_DBL)

	MASGN(Temperature[Index])
}

CGETINDEX(TemperatureSensor,TemperatureMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)
	TESTMASGN(temperatureMax[Index], PUNK_DBL)

	MASGN(temperatureMax[Index])
}

CGETINDEX(TemperatureSensor,TemperatureMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)
	TESTMASGN(temperatureMin[Index], PUNK_DBL)

	MASGN(temperatureMin[Index])
}

CGETINDEX(TemperatureSensor,TemperatureChangeTrigger,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)
	TESTMASGN(TempChangeTrigger[Index], PUNK_DBL)

	MASGN(TempChangeTrigger[Index])
}
CSETINDEX(TemperatureSensor,TemperatureChangeTrigger,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)
	TESTRANGE(0, phid->temperatureMax[Index] - phid->temperatureMin[Index])

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Trigger, "%lE", TempChangeTrigger);
	else
		phid->TempChangeTrigger[Index] = newVal;

	return EPHIDGET_OK;
}

CGETINDEX(TemperatureSensor,Potential,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR:
			if(phid->phid.deviceVersion < 200) 
				return EPHIDGET_UNSUPPORTED;
			break;
		case PHIDID_TEMPERATURESENSOR_IR:
			return EPHIDGET_UNSUPPORTED;
		default:
			break;
	}

	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)

	TESTMASGN(Potential[Index], PUNK_DBL)

	MASGN(Potential[Index])
}

CGETINDEX(TemperatureSensor,PotentialMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR:
			if(phid->phid.deviceVersion < 200) 
				return EPHIDGET_UNSUPPORTED;
			break;
		case PHIDID_TEMPERATURESENSOR_IR:
			return EPHIDGET_UNSUPPORTED;
		default:
			break;
	}

	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)

	TESTMASGN(potentialMax, PUNK_DBL)

	MASGN(potentialMax)
}

CGETINDEX(TemperatureSensor,PotentialMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR:
			if(phid->phid.deviceVersion < 200) 
				return EPHIDGET_UNSUPPORTED;
			break;
		case PHIDID_TEMPERATURESENSOR_IR:
			return EPHIDGET_UNSUPPORTED;
		default:
			break;
	}

	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)

	TESTMASGN(potentialMin, PUNK_DBL)

	MASGN(potentialMin)
}

CGET(TemperatureSensor,AmbientTemperature,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTMASGN(AmbientTemperature, PUNK_DBL)

	MASGN(AmbientTemperature)
}

CGET(TemperatureSensor,AmbientTemperatureMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTMASGN(ambientTemperatureMax, PUNK_DBL)

	MASGN(ambientTemperatureMax)
}

CGET(TemperatureSensor,AmbientTemperatureMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED
	TESTMASGN(ambientTemperatureMin, PUNK_DBL)

	MASGN(ambientTemperatureMin)
}

CGETINDEX(TemperatureSensor,ThermocoupleType, CPhidgetTemperatureSensor_ThermocoupleType)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR_IR:
			return EPHIDGET_UNSUPPORTED;
		default:
			break;
	}

	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)

	MASGN(ThermocoupleType[Index])
}
CSETINDEX(TemperatureSensor,ThermocoupleType, CPhidgetTemperatureSensor_ThermocoupleType)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEMPERATURESENSOR)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEMPERATURESENSOR:
			if(phid->phid.deviceVersion < 200) 
				return EPHIDGET_UNSUPPORTED;
			break;
		case PHIDID_TEMPERATURESENSOR_IR:
			return EPHIDGET_UNSUPPORTED;
		default:
			break;
	}

	TESTINDEX(phid.attr.temperaturesensor.numTempInputs)
	TESTRANGE(PHIDGET_TEMPERATURE_SENSOR_K_TYPE, PHIDGET_TEMPERATURE_SENSOR_T_TYPE)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(ThermocoupleType, "%d", ThermocoupleType);
	else
		phid->ThermocoupleType[Index] = newVal;
	
	phid->temperatureMax[Index] = thermocouple_useful_range[newVal][1];
	phid->temperatureMin[Index] = thermocouple_useful_range[newVal][0];

	//Make sure our max and min range is actually measurable at the current ambient temperature. This is updated as the ambient temperature
	// change in the data polling function
	if(phid->AmbientTemperature != PUNK_DBL)
	{
		if(lookup_voltage(phid->temperatureMax[Index] - phid->AmbientTemperature, newVal) > phid->potentialMax)
			phid->temperatureMax[Index] = (int)((lookup_temperature(phid->potentialMax, newVal) + phid->AmbientTemperature) / 10) * 10;
		if(lookup_voltage(phid->temperatureMin[Index] - phid->AmbientTemperature, newVal) < phid->potentialMin)
			phid->temperatureMin[Index] = (int)((lookup_temperature(phid->potentialMin, newVal) + phid->AmbientTemperature) / 10) * 10;
	}

	return EPHIDGET_OK;
}

// === Deprecated Functions === //

CGET(TemperatureSensor,NumTemperatureInputs,int)
	return CPhidgetTemperatureSensor_getTemperatureInputCount(phid, pVal);
}
