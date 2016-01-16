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
#include "cphidgetspatial.h"
#include "cusb.h"
#include "math.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //
static double getCorrectedField(CPhidgetSpatialHandle phid, double fields[], int axis);

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Spatial)
	int i = 0;

	phid->dataRateMin = PUNI_INT;
	phid->dataRate = PUNI_INT;
	phid->dataRateMax = PUNI_INT;

	phid->accelerationMax = PUNI_DBL;
	phid->accelerationMin = PUNI_DBL;
	phid->angularRateMax = PUNI_DBL;
	phid->angularRateMin = PUNI_DBL;
	phid->magneticFieldMax = PUNI_DBL;
	phid->magneticFieldMin = PUNI_DBL;
	phid->interruptRate = PUNI_INT;

	phid->spatialDataNetwork = PUNI_BOOL;

	for (i = 0; i<SPATIAL_MAX_ACCELAXES; i++)
	{
		phid->accelAxis[i] = PUNI_DBL;
	}
	for (i = 0; i<SPATIAL_MAX_GYROAXES; i++)
	{
		phid->gyroAxis[i] = PUNI_DBL;
	}
	for (i = 0; i<SPATIAL_MAX_COMPASSAXES; i++)
	{
		phid->compassAxis[i] = PUNI_DBL;
	}
	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Spatial)
	int i = 0;

	TESTPTR(phid);

	//Setup max/min values
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1049:
			phid->accelerationMax = 5.1;
			phid->accelerationMin = -5.1;
			phid->interruptRate = 8;
			phid->dataRateMin = SPATIAL_MIN_DATA_RATE;
			phid->dataRate = phid->interruptRate;
			phid->dataRateMax = 1; //actual data rate
			phid->angularRateMax = 0;
			phid->angularRateMin = 0;
			phid->magneticFieldMax = 0;
			phid->magneticFieldMin = 0;
			phid->calDataValid = PFALSE;
			break;

		case PHIDUID_SPATIAL_ACCEL_3AXIS_1041:
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1043:
			phid->accelerationMax = 8.1;
			phid->accelerationMin = -8.1;
			phid->interruptRate = 8;
			phid->dataRateMin = SPATIAL_MIN_DATA_RATE;
			phid->dataRate = phid->interruptRate;
			phid->dataRateMax = 1; //actual data rate
			phid->angularRateMax = 0;
			phid->angularRateMin = 0;
			phid->magneticFieldMax = 0;
			phid->magneticFieldMin = 0;
			phid->calDataValid = PFALSE;
			break;

		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:
			phid->accelerationMax = 5.1;
			phid->accelerationMin = -5.1;
			phid->interruptRate = 8;
			phid->dataRateMin = SPATIAL_MIN_DATA_RATE;
			phid->dataRate = phid->interruptRate;
			phid->dataRateMax = 4; //actual data rate
			phid->angularRateMax = 400.1;
			phid->angularRateMin = -400.1;
			phid->magneticFieldMax = 4.1;
			phid->magneticFieldMin = -4.1;
			phid->userMagField = 1.0;
			phid->calDataValid = PFALSE;
			break;

		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
			phid->accelerationMax = 8.1;
			phid->accelerationMin = -8.1;
			phid->interruptRate = 4;
			phid->dataRateMin = SPATIAL_MIN_DATA_RATE;
			phid->dataRate = 8;
			phid->dataRateMax = 4; //actual data rate
			phid->angularRateMax = 2000.1;
			phid->angularRateMin = -2000.1;
			phid->magneticFieldMax = 5.6;
			phid->magneticFieldMin = -5.6;
			phid->userMagField = 1.0;
			phid->calDataValid = PFALSE;
			break;

		default:
			return EPHIDGET_UNEXPECTED;
	}

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
	{
		phid->accelAxis[i] = PUNK_DBL;
		phid->accelGain1[i] = PUNK_DBL;
		phid->accelGain2[i] = PUNK_DBL;
		phid->accelOffset[i] = PUNK_INT;
	}
	for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
	{
		phid->gyroAxis[i] = PUNK_DBL;
		phid->gryoCorrection[i] = 0;
		phid->gyroGain1[i] = PUNK_DBL;
		phid->gyroGain2[i] = PUNK_DBL;
		phid->gyroOffset[i] = PUNK_INT;
	}
	for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
	{
		phid->compassAxis[i] = PUNK_DBL;
		phid->userCompassGain[i] = 1.0;
	}
	phid->bufferReadPtr = 0;
	phid->bufferWritePtr = 0;
	phid->timestamp.seconds = 0;
	phid->timestamp.microseconds = 0;
	phid->lastEventTime.seconds = 0;
	phid->lastEventTime.microseconds = 0;
	phid->latestDataTime.seconds = 0;
	phid->latestDataTime.microseconds = 0;

	phid->lastTimeCounterValid = PFALSE;
	phid->doZeroGyro = PFALSE;

	//get calibration values
	switch(phid->phid.deviceUID) {
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1049:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:
			{
				unsigned char buffer[8] = { 0 };
				int result;
				int readCount = 125; // up to 1 second of data - should be PLENTY
				//ask for calibration values
				buffer[0] = SPATIAL_READCALIB;
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
				while(phid->calDataValid == PFALSE && readCount--)
				{
					//note that Windows queues up to 32 packets, so we need to read at least this many to get the calibration packet
					CPhidget_read((CPhidgetHandle)phid);
				}
				if(!phid->calDataValid)
					return EPHIDGET_UNEXPECTED;
			}
			break;
		// No streamed Calibration
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1041:
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1043:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
		default:
			break;
	}

	//issue one read
	//this should fill in the data because the dataRate is the interrupt rate
	CPhidget_read((CPhidgetHandle)phid);

	return EPHIDGET_OK;
}

static void updateTimestamp(CPhidgetSpatialHandle phid, int time)
{
	if(phid->lastTimeCounterValid)
	{
		//1-255 ms
		int timechange = (unsigned short)((unsigned short)time - (unsigned short)phid->lastTimeCounter);
		timechange *= 1000; //us

		//if(timechange > 500000)
		//	LOG(PHIDGET_LOG_DEBUG, "Timechange: %d",timechange);

		phid->timestamp.seconds = phid->timestamp.seconds + (phid->timestamp.microseconds + timechange) / 1000000;
		phid->timestamp.microseconds = (phid->timestamp.microseconds + timechange) % 1000000;
	}
	else
	{
		phid->lastTimeCounterValid = PTRUE;
	}
	phid->lastTimeCounter = time;
}

static void updateLatestDataTime(CPhidgetSpatialHandle phid, int i)
{
	phid->latestDataTime.seconds = phid->timestamp.seconds + (phid->timestamp.microseconds + (i + 1) * phid->dataRateMax * 1000) / 1000000;
	phid->latestDataTime.microseconds = (phid->timestamp.microseconds + (i + 1) * phid->dataRateMax * 1000) % 1000000;
}

//dataInput - parses device packets
CPHIDGETDATA(Spatial)
	int i = 0, j = 0, count = 0, dataRate = phid->dataRate, cal;
	unsigned char doneGyroZero = PFALSE;
	double accelAvg[SPATIAL_MAX_ACCELAXES], angularRateAvg[SPATIAL_MAX_ACCELAXES], magneticFieldAvg[SPATIAL_MAX_ACCELAXES], magneticFieldCorr[SPATIAL_MAX_ACCELAXES];
	CPhidgetSpatial_SpatialEventDataHandle *eventData;
	
	ZEROMEM(accelAvg, sizeof(accelAvg));
	ZEROMEM(angularRateAvg, sizeof(angularRateAvg));
	ZEROMEM(magneticFieldAvg, sizeof(magneticFieldAvg));

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	//Parse device packets - store data locally
	switch(phidG->deviceUID)
	{
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1049:
		{
			int data;
			double accelUncalib[3] = {0,0,0};
			int time;
			
			//top 2 bits in buffer[0] are packet type
			switch(buffer[0] & 0xc0)
			{
				case SPATIAL_PACKET_DATA:
					if(phid->calDataValid)
					{
						count = buffer[0] / 3;
						if(count == 0)
							goto done;

						//this timestamp is for the latest data
						time = ((unsigned short)buffer[1]<<8) + (unsigned short)buffer[2];
						if(phid->lastTimeCounterValid)
						{
							//0-255 ms
							int timechange = (unsigned short)((unsigned short)time - (unsigned short)phid->lastTimeCounter);
							timechange *= 1000; //us

							phid->timestamp.seconds = phid->timestamp.seconds + (phid->timestamp.microseconds + timechange) / 1000000;
							phid->timestamp.microseconds = (phid->timestamp.microseconds + timechange) % 1000000;
						}
						else
						{
							phid->lastTimeCounterValid = PTRUE;
						}
						phid->lastTimeCounter = time;

						//add data to data buffer
						for(i=0;i<count;i++)
						{
							//LIS344ALH - Vdd/15 V/g - 0x1fff/15 = 0x222 (546.06666666666666666666666666667)
							for(j=0;j<3;j++)
							{
								data = ((unsigned short)buffer[3 + j * 2 + i * 6]<<8) + (unsigned short)buffer[4 + j * 2 + i * 6];
								accelUncalib[j] = ((double)data - 0x0fff) / 546.066667;
							}
							accelUncalib[1] = -accelUncalib[1]; //reverse Y-axis
							//Apply offsets
							for(j=0;j<3;j++)
							{
								accelUncalib[j] -= phid->accelOffset[j];
							}
							//X
							if(accelUncalib[0] > 0)
								phid->dataBuffer[phid->bufferWritePtr].acceleration[0] = accelUncalib[0] * phid->accelGain1[0] + accelUncalib[1] * phid->accelFactor1[0] + accelUncalib[2] * phid->accelFactor2[0];
							else
								phid->dataBuffer[phid->bufferWritePtr].acceleration[0] = accelUncalib[0] * phid->accelGain2[0] + accelUncalib[1] * phid->accelFactor1[0] + accelUncalib[2] * phid->accelFactor2[0];
							//Y
							if(accelUncalib[1] > 0)
								phid->dataBuffer[phid->bufferWritePtr].acceleration[1] = accelUncalib[1] * phid->accelGain1[1] + accelUncalib[0] * phid->accelFactor1[1] + accelUncalib[2] * phid->accelFactor2[1];
							else
								phid->dataBuffer[phid->bufferWritePtr].acceleration[1] = accelUncalib[1] * phid->accelGain2[1] + accelUncalib[0] * phid->accelFactor1[1] + accelUncalib[2] * phid->accelFactor2[1];
							//Z
							if(accelUncalib[2] > 0)
								phid->dataBuffer[phid->bufferWritePtr].acceleration[2] = accelUncalib[2] * phid->accelGain1[2] + accelUncalib[0] * phid->accelFactor1[2] + accelUncalib[1] * phid->accelFactor2[2];
							else
								phid->dataBuffer[phid->bufferWritePtr].acceleration[2] = accelUncalib[2] * phid->accelGain2[2] + accelUncalib[0] * phid->accelFactor1[2] + accelUncalib[1] * phid->accelFactor2[2];

							updateLatestDataTime(phid, i);

							phid->dataBuffer[phid->bufferWritePtr].timestamp = phid->latestDataTime;

							phid->bufferWritePtr++;
							if(phid->bufferWritePtr >= SPATIAL_DATA_BUFFER_SIZE)
								phid->bufferWritePtr = 0;
						}
					}
					break;
				case SPATIAL_PACKET_CALIB:
					for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
					{
						cal = ((unsigned short)buffer[i*7 + 1]<<4) + ((unsigned short)buffer[i*7 + 2]>>4);
						phid->accelGain1[i] = cal / (4096/0.4) + 0.8;
						cal = (((unsigned short)buffer[i*7 + 2]<<8) & 0x0F00) | ((unsigned short)buffer[i*7 + 3]);
						phid->accelGain2[i] = cal / (4096/0.4) + 0.8;
						cal = (unsigned short)((unsigned short)buffer[i*7 + 4]<<8) + (unsigned short)buffer[i*7 + 5];
						phid->accelOffset[i] = cal / (65535 / 1.0) - 0.5;
						cal = (unsigned char)buffer[i*7 + 6];
						phid->accelFactor1[i] = cal / (256 / 0.2) - 0.1;
						cal = (unsigned char)buffer[i*7 + 7];
						phid->accelFactor2[i] = cal / (256 / 0.2) - 0.1;
						//LOG(PHIDGET_LOG_INFO, "Accel(%d) Calib: %1.4lf, %1.4lf, %1.4lf, %1.4lf, %1.4lf", i, 
						//	phid->accelGain1[i], phid->accelGain2[i], phid->accelOffset[i], phid->accelFactor1[i], phid->accelFactor2[i]);
					}
					phid->calDataValid = PTRUE;
					break;
			}
			break;
		}
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:
			//top 2 bits in buffer[0] are packet type
			switch(buffer[0])
			{
				case SPATIAL_PACKET_DATA:
					if(phid->calDataValid)
					{
						int data;
						double accelUncalib[3] = {0,0,0};
						double gyroUncalib[3] = {0,0,0};
						int time;
						
						count = (buffer[1] & 0x1f) / 9;
						if(count == 0)
							goto done;

						//this timestamp is for the latest data
						time = ((unsigned short)buffer[2]<<8) + (unsigned short)buffer[3];
						if(phid->lastTimeCounterValid)
						{
							//0-255 ms
							int timechange = (unsigned short)((unsigned short)time - (unsigned short)phid->lastTimeCounter);
							timechange *= 1000; //us

							phid->timestamp.seconds = phid->timestamp.seconds + (phid->timestamp.microseconds + timechange) / 1000000;
							phid->timestamp.microseconds = (phid->timestamp.microseconds + timechange) % 1000000;
						}
						else
						{
							phid->lastTimeCounterValid = PTRUE;
						}
						phid->lastTimeCounter = time;

						//add data to data buffer
						for(i=0;i<count;i++)
						{
							//LIS344ALH - Vdd/15 V/g - 0xffff/15 = 0x1111 (4369.0)
							for(j=0;j<3;j++)
							{
								data = ((unsigned short)buffer[4 + j * 2 + i * 18]<<8) + (unsigned short)buffer[5 + j * 2 + i * 18];
								accelUncalib[j] = ((double)data - 0x7fff) / 4369.0;
							}
							accelUncalib[1] = -accelUncalib[1]; //reverse Y-axis
							//Apply offsets
							for(j=0;j<3;j++)
							{
								accelUncalib[j] -= phid->accelOffset[j];
							}
							//X
							if(accelUncalib[0] > 0)
								phid->dataBuffer[phid->bufferWritePtr].acceleration[0] = accelUncalib[0] * phid->accelGain1[0] + accelUncalib[1] * phid->accelFactor1[0] + accelUncalib[2] * phid->accelFactor2[0];
							else
								phid->dataBuffer[phid->bufferWritePtr].acceleration[0] = accelUncalib[0] * phid->accelGain2[0] + accelUncalib[1] * phid->accelFactor1[0] + accelUncalib[2] * phid->accelFactor2[0];
							//Y
							if(accelUncalib[1] > 0)
								phid->dataBuffer[phid->bufferWritePtr].acceleration[1] = accelUncalib[1] * phid->accelGain1[1] + accelUncalib[0] * phid->accelFactor1[1] + accelUncalib[2] * phid->accelFactor2[1];
							else
								phid->dataBuffer[phid->bufferWritePtr].acceleration[1] = accelUncalib[1] * phid->accelGain2[1] + accelUncalib[0] * phid->accelFactor1[1] + accelUncalib[2] * phid->accelFactor2[1];
							//Z
							if(accelUncalib[2] > 0)
								phid->dataBuffer[phid->bufferWritePtr].acceleration[2] = accelUncalib[2] * phid->accelGain1[2] + accelUncalib[0] * phid->accelFactor1[2] + accelUncalib[1] * phid->accelFactor2[2];
							else
								phid->dataBuffer[phid->bufferWritePtr].acceleration[2] = accelUncalib[2] * phid->accelGain2[2] + accelUncalib[0] * phid->accelFactor1[2] + accelUncalib[1] * phid->accelFactor2[2];

							//ADC ref is 0-3.3V - 50.355uV/bit, gyro zero rate is 1.23V, 2.5mV/deg/s - these voltages are fixed, non-ratiometric to Vref
							// 1 / 0.000050355 = 19859 (1V)
							// 1.23 * 19859 = 24427
							// 0.0025 * 19859 = 49.6477bits/deg/s
							for(j=0;j<3;j++)
							{
								data = ((unsigned short)buffer[10 + j * 2 + i * 18]<<8) + (unsigned short)buffer[11 + j * 2 + i * 18];
								if(j==1)
									gyroUncalib[j] = ((double)(data-24427) + phid->gyroOffset[j]) / 49.6477;
								else
									gyroUncalib[j] = ((double)-(data-24427) + phid->gyroOffset[j]) / 49.6477; //reverse X/Z-axis
							}
							//0 - we multiply these by their gains so revered axes will still appear positive and get the correct gain
							if((gyroUncalib[0] * phid->gyroGain1[0]) > 0)
								phid->dataBuffer[phid->bufferWritePtr].angularRate[0] = gyroUncalib[0] * phid->gyroGain1[0] - gyroUncalib[1] * phid->gyroFactor1[0] - gyroUncalib[2] * phid->gyroFactor2[0];
							else
								phid->dataBuffer[phid->bufferWritePtr].angularRate[0] = gyroUncalib[0] * phid->gyroGain2[0] - gyroUncalib[1] * phid->gyroFactor1[0] - gyroUncalib[2] * phid->gyroFactor2[0];
							//1
							if((gyroUncalib[1] * phid->gyroGain1[1]) > 0)
								phid->dataBuffer[phid->bufferWritePtr].angularRate[1] = gyroUncalib[1] * phid->gyroGain1[1] - gyroUncalib[0] * phid->gyroFactor1[1] - gyroUncalib[2] * phid->gyroFactor2[1];
							else
								phid->dataBuffer[phid->bufferWritePtr].angularRate[1] = gyroUncalib[1] * phid->gyroGain2[1] - gyroUncalib[0] * phid->gyroFactor1[1] - gyroUncalib[2] * phid->gyroFactor2[1];
							//2
							if((gyroUncalib[2] * phid->gyroGain1[2]) > 0)
								phid->dataBuffer[phid->bufferWritePtr].angularRate[2] = gyroUncalib[2] * phid->gyroGain1[2] - gyroUncalib[0] * phid->gyroFactor1[2] - gyroUncalib[1] * phid->gyroFactor2[2];
							else
								phid->dataBuffer[phid->bufferWritePtr].angularRate[2] = gyroUncalib[2] * phid->gyroGain2[2] - gyroUncalib[0] * phid->gyroFactor1[2] - gyroUncalib[1] * phid->gyroFactor2[2];

							//checks if compass data is valid
							//Note: we miss ~7 samples (28ms) every second while the compass is callibrating
							if(buffer[1] & (0x80 >> i))
							{
								//ADC 50.355uV/bit (0-3.3V)
								//ideal compass midpoint is 0x7FFF (32767) (1.65V) 
								//valid range for zero field offset is: 0.825V - 2.475V (16384-49151) (+-16384)
								// Note that this may be less (~3x) because the Gain is less, but I'm not sure. (+-5460)
								//valid output voltage range is defined as 0.165V - 3.135V (3277-62258), 
								// so we can't really trust values outside of this, though we do seem to get valid data...
								//ideal sensitivity is 250mV/gauss (ext. resistor), valid range is 195 - 305
								// 1 / 0.000050355 = 19859 (1Volt)
								// 0.250 * 19859 = 4964.75 bits/gauss (1.0 gain) (ideal) - valid range is (3861-6068) (+-1103)
								//We have defined the compass gain multiplier to be based on 6500bits/gauss to keep the math resonable,
								// so we must use that value here. Implications?
								//The largest range we can guarantee is:
								// 16384-3277/6068 = +-2.16 gauss or, more likely: +-3.96 gauss
								// Ideal is: 32767-3277/4964.75 = +-5.94 gauss
								// we can tell from the incoming data whether it's valid or not, 
								// we'll probably have more range in one dirrection then the other because of offset.
								for(j=0;j<phid->phid.attr.spatial.numCompassAxes; j++)
								{
									data = ((unsigned short)buffer[16 + i * 18 + j * 2]<<8) + (unsigned short)buffer[17 + i * 18 + j * 2];
									//if we are not within (3277-62258), data is not valid
									if(data < 3277)
									{
										phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = phid->magneticFieldMin;
										break;
									}
									if(data > 62258)
									{
										phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = phid->magneticFieldMax;
										break;
									}

									//if gain or offset don't make sense, throw out data
									//if(phid->compassGain[j] > 6068 || phid->compassGain[j] < 3861 || 
									if(phid->compassGain[j] > 6068 || phid->compassGain[j] < 2500 || //lower gains seem to be common
										phid->compassOffset[j] > 5460 || phid->compassOffset[j] < -5460)
									{
										if(data > 32767)
											phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = phid->magneticFieldMax;
										else
											phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = phid->magneticFieldMin;
										break;
									}

									//Convert ADC to Gauss
									phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = 
										-((double)data - 0x7fff - phid->compassOffset[j]) / phid->compassGain[j];

									//constrain to max/min
									//ie if field is 4.02 and max is 4.1, make it 4.1, since real max is 4.0
									if(phid->dataBuffer[phid->bufferWritePtr].magneticField[j] > (phid->magneticFieldMax - 0.1))
										phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = phid->magneticFieldMax;
									if(phid->dataBuffer[phid->bufferWritePtr].magneticField[j] < (phid->magneticFieldMin + 0.1))
										phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = phid->magneticFieldMin;
								}

							}
							else
							{
								phid->dataBuffer[phid->bufferWritePtr].magneticField[0] = PUNK_DBL;
								phid->dataBuffer[phid->bufferWritePtr].magneticField[1] = PUNK_DBL;
								phid->dataBuffer[phid->bufferWritePtr].magneticField[2] = PUNK_DBL;
							}

							updateLatestDataTime(phid, i);

							phid->dataBuffer[phid->bufferWritePtr].timestamp = phid->latestDataTime;

							phid->bufferWritePtr++;
							if(phid->bufferWritePtr >= SPATIAL_DATA_BUFFER_SIZE)
								phid->bufferWritePtr = 0;
						}
					}
					break;
				case SPATIAL_PACKET_CALIB:
					for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
					{
						cal = ((unsigned short)buffer[i*7 + 1]<<4) + ((unsigned short)buffer[i*7 + 2]>>4);
						phid->accelGain1[i] = cal / (4096/0.4) + 0.8;
						cal = (((unsigned short)buffer[i*7 + 2]<<8) & 0x0F00) | ((unsigned short)buffer[i*7 + 3]);
						phid->accelGain2[i] = cal / (4096/0.4) + 0.8;
						cal = (unsigned short)((unsigned short)buffer[i*7 + 4]<<8) + (unsigned short)buffer[i*7 + 5];
						phid->accelOffset[i] = cal / (65535 / 1.0) - 0.5;
						cal = (unsigned char)buffer[i*7 + 6];
						phid->accelFactor1[i] = cal / (256 / 0.2) - 0.1;
						cal = (unsigned char)buffer[i*7 + 7];
						phid->accelFactor2[i] = cal / (256 / 0.2) - 0.1;
						//LOG(PHIDGET_LOG_INFO, "Accel(%d) Calib: %1.4lf, %1.4lf, %1.4lf, %1.4lf, %1.4lf", i, 
						//	phid->accelGain1[i], phid->accelGain2[i], phid->accelOffset[i], phid->accelFactor1[i], phid->accelFactor2[i]);
					}
					for (j=0; j<phid->phid.attr.spatial.numGyroAxes; i++,j++)
					{
						if (phidG->deviceUID == PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056)
						{
							cal = ((unsigned short)buffer[i*7 + 1]<<4) + ((unsigned short)buffer[i*7 + 2]>>4);
							phid->gyroGain1[j] = cal / (4096/0.4) + 0.8;
							cal = (((unsigned short)buffer[i*7 + 2]<<8) & 0x0F00) | ((unsigned short)buffer[i*7 + 3]);
							phid->gyroGain2[j] = cal / (4096/0.4) + 0.8;
						}
						//Allow for negative gains
						else if (phidG->deviceUID == PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN)
						{
							cal = ((signed short)((signed char)buffer[i*7 + 1])<<4) + ((unsigned short)buffer[i*7 + 2]>>4);
							phid->gyroGain1[j] = cal / (4096/0.4) + (cal > 0 ? 0.9 : -0.9);
							cal = ((((unsigned short)buffer[i*7 + 2]<<8) & 0x0F00) | ((unsigned short)buffer[i*7 + 3]) ) << 4;
							cal = (signed short)cal >> 4;
							phid->gyroGain2[j] = cal / (4096/0.4) + (cal > 0 ? 0.9 : -0.9);
						}
						cal = (signed short)((unsigned short)buffer[i*7 + 4]<<8) + (unsigned short)buffer[i*7 + 5];
						phid->gyroOffset[j] = cal;
						cal = (unsigned char)buffer[i*7 + 6];
						phid->gyroFactor1[j] = cal / (256 / 0.1) - 0.05;
						cal = (unsigned char)buffer[i*7 + 7];
						phid->gyroFactor2[j] = cal / (256 / 0.1) - 0.05;

//Zero out calibrations
#if 0
						phid->gyroGain1[j] = 1;
						phid->gyroGain2[j] = 1;
						phid->gyroOffset[j] = 0;
						phid->gyroFactor1[j] = 0;
						phid->gyroFactor2[j] = 0;
#endif

						LOG(PHIDGET_LOG_VERBOSE, "Gyro(%d) Calib: %1.4lf, %1.4lf, %1.4lf, %1.4lf, %1.4lf", j, 
							phid->gyroGain1[j], phid->gyroGain2[j], phid->gyroOffset[j], phid->gyroFactor1[j], phid->gyroFactor2[j]);
					}
					for(j=0;j<phid->phid.attr.spatial.numCompassAxes; j++)
					{
						phid->compassOffset[j] = (signed short)((unsigned short)buffer[j*4 + 49]<<8) + (unsigned short)buffer[j*4 + 50];
						phid->compassGain[j] = ((unsigned short)buffer[j*4 + 51]<<8) + (unsigned short)buffer[j*4 + 52];
						//phid->compassGain[j] = 4964;
					}
					//LOG(PHIDGET_LOG_INFO, "Compass Gain: %d, %d, %d", phid->compassGain[0], phid->compassGain[1], phid->compassGain[2]);
					//LOG(PHIDGET_LOG_INFO, "Compass Offset: %d, %d, %d", phid->compassOffset[0], phid->compassOffset[1], phid->compassOffset[2]);
					phid->calDataValid = PTRUE;
					break;
			}
			break;
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1041:
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1043:
		{
			int time;
			int analogOrDigital = ((unsigned short)buffer[1]<<8) + (unsigned short)buffer[2];
			count = buffer[0];

			if(count == 0)
				goto done;

			//this timestamp is for the latest data
			time = ((unsigned short)buffer[3]<<8) + (unsigned short)buffer[4];
			updateTimestamp(phid, time);

			//add data to data buffer
			for(i=0;i<count;i++)
			{
				int countOffset = i * 6; //Each set of samples is 6 bytes
				for(j=0;j<3;j++)
				{
					int indexOffset = j * 2; //Each value is 2 bytes
					short accelData	= (signed short)((unsigned short)buffer[5 + indexOffset + countOffset]<<8) + (unsigned short)buffer[6 + indexOffset + countOffset];

					//digital accel
					if(analogOrDigital & (0x01 << i))
						phid->dataBuffer[phid->bufferWritePtr].acceleration[j] = (double)accelData / SPATIAL_MMA8451Q_BITS_PER_G;
					//analog accel
					else
						phid->dataBuffer[phid->bufferWritePtr].acceleration[j] = (double)accelData / SPATIAL_KXR94_2050_w_AD7689_BITS_PER_G;
				}

				updateLatestDataTime(phid, i);

				phid->dataBuffer[phid->bufferWritePtr].timestamp = phid->latestDataTime;

				phid->bufferWritePtr++;
				if(phid->bufferWritePtr >= SPATIAL_DATA_BUFFER_SIZE)
					phid->bufferWritePtr = 0;
			}
			break;
		}
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
		{
			int time;
			int flags = buffer[1];
			count = buffer[0];

			if(count == 0)
				goto done;

			//this timestamp is for the latest data
			time = ((unsigned short)buffer[2]<<8) + (unsigned short)buffer[3];
			updateTimestamp(phid, time);

			//add data to data buffer
			for(i=0;i<count;i++)
			{
				int countOffset = i * 18; //Each set of samples is 18 bytes
				for(j=0;j<3;j++)
				{
					int indexOffset = j * 2; //Each value is 2 bytes
					short accelData	= (signed short)((unsigned short)buffer[ 4 + indexOffset + countOffset] << 8) + (unsigned short)buffer[ 5 + indexOffset + countOffset];
					short gyroData  = (signed short)((unsigned short)buffer[10 + indexOffset + countOffset] << 8) + (unsigned short)buffer[11 + indexOffset + countOffset];
					short magData   = (signed short)((unsigned short)buffer[16 + indexOffset + countOffset] << 8) + (unsigned short)buffer[17 + indexOffset + countOffset];

					//digital accel
					if(flags & (0x02 >> i))
						phid->dataBuffer[phid->bufferWritePtr].acceleration[j] = (double)accelData / SPATIAL_MMA8451Q_BITS_PER_G;
					//analog accel
					else
						phid->dataBuffer[phid->bufferWritePtr].acceleration[j] = (double)accelData / SPATIAL_KXR94_2050_w_AD7689_BITS_PER_G;
					
					//digital gyro
					if(flags & (0x08 >> i))
							phid->dataBuffer[phid->bufferWritePtr].angularRate[j] = (double)gyroData / SPATIAL_L3GD20_BITS_PER_DPS;
					//analog gyro
					else
						if(j==2)
							phid->dataBuffer[phid->bufferWritePtr].angularRate[j] = (double)(gyroData) / SPATIAL_LY330ALH_w_AD7689_BITS_PER_DPS;
						else
							phid->dataBuffer[phid->bufferWritePtr].angularRate[j] = (double)(gyroData) / SPATIAL_LRP410AL_w_AD7689_BITS_PER_DPS;
									
					//compass valid
					if(flags & (0x20 >> i))
						phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = magData / SPATIAL_HMC5883L_BITS_PER_GAUSS;
					//no compass data
					else
						phid->dataBuffer[phid->bufferWritePtr].magneticField[j] = PUNK_DBL;
				}

				updateLatestDataTime(phid, i);

				phid->dataBuffer[phid->bufferWritePtr].timestamp = phid->latestDataTime;

				phid->bufferWritePtr++;
				if(phid->bufferWritePtr >= SPATIAL_DATA_BUFFER_SIZE)
					phid->bufferWritePtr = 0;
			}
			break;
		}
		default:
			return EPHIDGET_UNEXPECTED;
	}

	if(phid->doZeroGyro)
	{
		//done
		if(timestampdiff(phid->latestDataTime, phid->dataBuffer[phid->gyroZeroReadPtr].timestamp) >= SPATIAL_ZERO_GYRO_TIME)
		{
			double gryoCorrectionTemp[SPATIAL_MAX_GYROAXES] = {0,0,0};
			int gryoCorrectionCount = 0;

			while(phid->gyroZeroReadPtr != phid->bufferWritePtr)
			{
				for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
				{
					gryoCorrectionTemp[i] += phid->dataBuffer[phid->gyroZeroReadPtr].angularRate[i];
				}

				phid->gyroZeroReadPtr++;
				if(phid->gyroZeroReadPtr >= SPATIAL_DATA_BUFFER_SIZE)
					phid->gyroZeroReadPtr = 0;

				gryoCorrectionCount++;
			}
			
			for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
			{
				phid->gryoCorrection[i] = gryoCorrectionTemp[i] / (double)gryoCorrectionCount;
			}

			doneGyroZero = PTRUE;
		}
	}

	//see if it's time for an event
	if(timestampdiff(phid->latestDataTime, phid->lastEventTime) >= dataRate)
	{
		CPhidget_Timestamp tempTime;
		//int lastPtr;
		int accelCounter[SPATIAL_MAX_ACCELAXES], angularRateCounter[SPATIAL_MAX_ACCELAXES], magneticFieldCounter[SPATIAL_MAX_ACCELAXES];

		int dataPerEvent = 0;

		int multipleDataPerEvent = PFALSE;

		if(dataRate < phid->interruptRate)
			multipleDataPerEvent = PTRUE;

		//max of 16 data per event
		eventData = malloc(16 * sizeof(CPhidgetSpatial_SpatialEventDataHandle));
		
		for(j=0;;j++)
		{
			//makes sure we read all data
			if(phid->bufferReadPtr == phid->bufferWritePtr || j>=16)
			{
				dataPerEvent = j;
				break;
			}

			eventData[j] = malloc(sizeof(CPhidgetSpatial_SpatialEventData));
			ZEROMEM(accelCounter, sizeof(accelCounter));
			ZEROMEM(angularRateCounter, sizeof(angularRateCounter));
			ZEROMEM(magneticFieldCounter, sizeof(magneticFieldCounter));

			tempTime = phid->dataBuffer[phid->bufferReadPtr].timestamp;

			//average data for each stage
			while(phid->bufferReadPtr != phid->bufferWritePtr && 
				(!multipleDataPerEvent || timestampdiff(phid->dataBuffer[phid->bufferReadPtr].timestamp, tempTime) < dataRate))
			{
				for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
				{
					if(phid->dataBuffer[phid->bufferReadPtr].acceleration[i] != PUNK_DBL)
					{
						if(phid->dataBuffer[phid->bufferReadPtr].acceleration[i] > phid->accelerationMax)
							phid->dataBuffer[phid->bufferReadPtr].acceleration[i] = phid->accelerationMax;
						if(phid->dataBuffer[phid->bufferReadPtr].acceleration[i] < phid->accelerationMin) 
							phid->dataBuffer[phid->bufferReadPtr].acceleration[i] = phid->accelerationMin;
						accelAvg[i] += phid->dataBuffer[phid->bufferReadPtr].acceleration[i];
						accelCounter[i]++;
					}
				}
				for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
				{
					if(phid->dataBuffer[phid->bufferReadPtr].angularRate[i] != PUNK_DBL)
					{
						double rate = phid->dataBuffer[phid->bufferReadPtr].angularRate[i] - phid->gryoCorrection[i];

						if(rate > phid->angularRateMax) 
							angularRateAvg[i] += phid->angularRateMax;
						else if(rate < phid->angularRateMin) 
							angularRateAvg[i] += phid->angularRateMin;
						else
							angularRateAvg[i] += rate;
						angularRateCounter[i]++;
					}
				}
				for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
				{
					if(phid->dataBuffer[phid->bufferReadPtr].magneticField[i] != PUNK_DBL)
					{
						if(phid->dataBuffer[phid->bufferReadPtr].magneticField[i] > phid->magneticFieldMax) 
							phid->dataBuffer[phid->bufferReadPtr].magneticField[i] = phid->magneticFieldMax;
						if(phid->dataBuffer[phid->bufferReadPtr].magneticField[i] < phid->magneticFieldMin) 
							phid->dataBuffer[phid->bufferReadPtr].magneticField[i] = phid->magneticFieldMin;
						magneticFieldAvg[i] += phid->dataBuffer[phid->bufferReadPtr].magneticField[i];
						magneticFieldCounter[i]++;
					}
				}

				//lastPtr = phid->bufferReadPtr;

				phid->bufferReadPtr++;
				if(phid->bufferReadPtr >= SPATIAL_DATA_BUFFER_SIZE)
					phid->bufferReadPtr = 0;
			}

			for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
			{
				if(accelCounter[i] > 0)
					eventData[j]->acceleration[i] = round_double(accelAvg[i] / (double)accelCounter[i], 5);
				else
					eventData[j]->acceleration[i] = PUNK_DBL;
				accelAvg[i] = 0;
			}
			for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
			{
				if(angularRateCounter[i] > 0)
				{
					if(phid->doZeroGyro && !doneGyroZero)
						eventData[j]->angularRate[i] = 0;
					else
						eventData[j]->angularRate[i] = round_double(angularRateAvg[i] / (double)angularRateCounter[i], 5);
				}
				else
					eventData[j]->angularRate[i] = PUNK_DBL;
				angularRateAvg[i] = 0;
			}
			for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
			{
				if(magneticFieldCounter[i] > 0)
					eventData[j]->magneticField[i] = round_double(magneticFieldAvg[i] / (double)magneticFieldCounter[i], 5);
				else
					eventData[j]->magneticField[i] = PUNK_DBL;
				magneticFieldAvg[i] = 0;
			}
			eventData[j]->timestamp = tempTime;
		}

		//correct magnetic field data in the event structure
		// But only on devices that don't do this in Firmware!
		switch(phid->phid.deviceUID)
		{
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:
				for( j = 0; j < dataPerEvent; j++)
				{
					for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
					{
						magneticFieldCorr[i] = eventData[j]->magneticField[i];
					}
					for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
					{
						if(eventData[j]->magneticField[i] != PUNK_DBL)
						{
							eventData[j]->magneticField[i] = getCorrectedField(phid, magneticFieldCorr, i);
						}
					}
				}
				break;
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
			default:
				break;
		}

		//store to local structure
		ZEROMEM(accelCounter, sizeof(accelCounter));
		ZEROMEM(angularRateCounter, sizeof(angularRateCounter));
		ZEROMEM(magneticFieldCounter, sizeof(magneticFieldCounter));
		for( j = 0; j < dataPerEvent; j++)
		{
			for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
			{
				if(eventData[j]->acceleration[i] != PUNK_DBL)
				{
					accelAvg[i] += eventData[j]->acceleration[i];
					accelCounter[i]++;
				}
			}
			for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
			{
				if(eventData[j]->angularRate[i] != PUNK_DBL)
				{
					angularRateAvg[i] += eventData[j]->angularRate[i];
					angularRateCounter[i]++;
				}
			}
			for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
			{
				if(eventData[j]->magneticField[i] != PUNK_DBL)
				{
					magneticFieldAvg[i] += eventData[j]->magneticField[i];
					magneticFieldCounter[i]++;
				}
			}
		}

		//Set local get data to averages
		for (i = 0; i<phid->phid.attr.spatial.numAccelAxes; i++)
		{
			if(accelCounter[i] > 0)
				phid->accelAxis[i] = round_double(accelAvg[i] / (double)accelCounter[i], 5);
			else
				phid->accelAxis[i] = PUNK_DBL;
		}
		for (i = 0; i<phid->phid.attr.spatial.numGyroAxes; i++)
		{
			if(angularRateCounter[i] > 0)
			{
				if(phid->doZeroGyro && !doneGyroZero)
					phid->gyroAxis[i] = 0;
				else
					phid->gyroAxis[i] = round_double(angularRateAvg[i] / (double)angularRateCounter[i], 5);
			}
			else
				phid->gyroAxis[i] = PUNK_DBL;
		}
		for (i = 0; i<phid->phid.attr.spatial.numCompassAxes; i++)
		{
			if(magneticFieldCounter[i] > 0)
				phid->compassAxis[i] = round_double(magneticFieldAvg[i] / (double)magneticFieldCounter[i], 5);
			else
				phid->compassAxis[i] = PUNK_DBL;
		}
		
		//send out any events
		FIRE(SpatialData, eventData, dataPerEvent);

		phid->lastEventTime = phid->latestDataTime;

		for(i=0;i<dataPerEvent;i++)
			free(eventData[i]);
		free(eventData);
	}
done:

	//this will signal the zero function to return;
	if(doneGyroZero)
		phid->doZeroGyro = PFALSE;

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(Spatial)
	TESTPTR(phid);
	//don't need to worry, because the interrupts come at a set rate
	return EPHIDGET_OK;
}

//getPacket - not used for spatial
CGETPACKET(Spatial)
	return EPHIDGET_UNEXPECTED;
}

static double getCorrectedField(CPhidgetSpatialHandle phid, double fields[], int axis)
{
	switch(axis)
	{
	case 0:
		return phid->userMagField * 
			(phid->userCompassGain[0] * (fields[0] - phid->userCompassOffset[0])
			+ phid->userCompassTransform[0] * (fields[1] - phid->userCompassOffset[1])
			+ phid->userCompassTransform[1] * (fields[2] - phid->userCompassOffset[2]));
	case 1:
		return phid->userMagField * 
			(phid->userCompassGain[1] * (fields[1] - phid->userCompassOffset[1])
			+ phid->userCompassTransform[2] * (fields[0] - phid->userCompassOffset[0])
			+ phid->userCompassTransform[3] * (fields[2] - phid->userCompassOffset[2]));
	case 2:
		return phid->userMagField * 
			(phid->userCompassGain[2] * (fields[2] - phid->userCompassOffset[2])
			+ phid->userCompassTransform[4] * (fields[0] - phid->userCompassOffset[0])
			+ phid->userCompassTransform[5] * (fields[1] - phid->userCompassOffset[1]));
	default:
		return 0;
	}
}

// Accel and Gyro tables are the same structure - just different table IDs.
static int setCalibrationValues_inFirmware(CPhidgetSpatialHandle phid, int tableID, int index,
	double gainPositive[3], double gainNegative[3], double offset[3], double factor1[3], double factor2[3])
{
	unsigned char buffer[SPATIAL_ACCEL_GYRO_CALIB_TABLE_LENGTH] = {0};
	int i;
	double offsetMultipliers[3];
	unsigned int header;

	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	if(!deviceSupportsGeneralUSBProtocol((CPhidgetHandle)phid))
		return EPHIDGET_UNSUPPORTED;

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1043:
			if(index == SPATIAL_ANALOG_ACCEL_CALIB_TABLE_INDEX)
			{
				offsetMultipliers[0] = offsetMultipliers[1] = offsetMultipliers[2] = SPATIAL_KXR94_2050_w_AD7689_BITS_PER_G;
				break;
			}
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1041:
			if(index == SPATIAL_DIGITAL_ACCEL_CALIB_TABLE_INDEX)
			{
				offsetMultipliers[0] = offsetMultipliers[1] = offsetMultipliers[2] = SPATIAL_MMA8451Q_BITS_PER_G;
				break;
			}
			else
				return EPHIDGET_UNSUPPORTED;

		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
			if(index == SPATIAL_ANALOG_GYRO_CALIB_TABLE_INDEX)
			{
				offsetMultipliers[0] = offsetMultipliers[1] = SPATIAL_LRP410AL_w_AD7689_BITS_PER_DPS;
				offsetMultipliers[2] = SPATIAL_LY330ALH_w_AD7689_BITS_PER_DPS;
				break;
			}
			if(index == SPATIAL_ANALOG_ACCEL_CALIB_TABLE_INDEX)
			{
				offsetMultipliers[0] = offsetMultipliers[1] = offsetMultipliers[2] = SPATIAL_KXR94_2050_w_AD7689_BITS_PER_G;
				break;
			}
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
			if(index == SPATIAL_DIGITAL_GYRO_CALIB_TABLE_INDEX)
			{
				offsetMultipliers[0] = offsetMultipliers[1] = offsetMultipliers[2] = SPATIAL_L3GD20_BITS_PER_DPS;
				break;
			}
			if(index == SPATIAL_DIGITAL_ACCEL_CALIB_TABLE_INDEX)
			{
				offsetMultipliers[0] = offsetMultipliers[1] = offsetMultipliers[2] = SPATIAL_MMA8451Q_BITS_PER_G;
				break;
			}
			else
				return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNSUPPORTED;
	}

	header = (((unsigned int)tableID) << 20) | SPATIAL_ACCEL_GYRO_CALIB_TABLE_LENGTH;

    buffer[3] = header >> 24; //header high byte
    buffer[2] = header >> 16;
    buffer[1] = header >> 8;
    buffer[0] = header >> 0; //header low byte

	for(i=0;i<3;i++)
	{
		int int32Offset = i * 4;
		int temp;

		temp = round(gainPositive[i] * (double)0x10000);
		buffer[int32Offset +  7] = temp >> 24;
		buffer[int32Offset +  6] = temp >> 16;
		buffer[int32Offset +  5] = temp >> 8;
		buffer[int32Offset +  4] = temp >> 0;
		
		temp = round(gainNegative[i] * (double)0x10000);
		buffer[int32Offset + 19] = temp >> 24;
		buffer[int32Offset + 18] = temp >> 16;
		buffer[int32Offset + 17] = temp >> 8;
		buffer[int32Offset + 16] = temp >> 0;

		temp = round(offset[i] * offsetMultipliers[i]);
		buffer[int32Offset + 31] = temp >> 24;
		buffer[int32Offset + 30] = temp >> 16;
		buffer[int32Offset + 29] = temp >> 8;
		buffer[int32Offset + 28] = temp >> 0;

		temp = round(factor1[i] * (double)0x10000);
		buffer[int32Offset + 43] = temp >> 24;
		buffer[int32Offset + 42] = temp >> 16;
		buffer[int32Offset + 41] = temp >> 8;
		buffer[int32Offset + 40] = temp >> 0;

		temp = round(factor2[i] * (double)0x10000);
		buffer[int32Offset + 55] = temp >> 24;
		buffer[int32Offset + 54] = temp >> 16;
		buffer[int32Offset + 53] = temp >> 8;
		buffer[int32Offset + 52] = temp >> 0;
	}

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		//TODO - maybe, maybe not.
		return EPHIDGET_UNSUPPORTED;
	}
	else
		return CPhidgetGPP_setDeviceSpecificConfigTable((CPhidgetHandle)phid, buffer, SPATIAL_ACCEL_GYRO_CALIB_TABLE_LENGTH, index);
}

static int setCompassCorrectionTable_inFimrware(
	CPhidgetSpatialHandle phid, 
	double magField, 
	double offset0, double offset1, double offset2, 
	double gain0, double gain1, double gain2, 
	double T0, double T1, double T2, double T3, double T4, double T5)
{
	unsigned char buffer[SPATIAL_COMPASS_CALIB_TABLE_LENGTH] = {0};
	int i, result;
	int gains[3], offsets[3], transforms[6], mag;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol((CPhidgetHandle)phid))
		return EPHIDGET_UNSUPPORTED;

	//compass calibration table Header is: 0x3EA00038
    buffer[3] = 0x3E; //header high byte
    buffer[2] = 0xA0;
    buffer[1] = 0x00;
    buffer[0] = SPATIAL_COMPASS_CALIB_TABLE_LENGTH; //header low byte

	//Mag Field (x0x10000)
	mag = round(magField * (double)0x10000);
    buffer[7] = mag >> 24;
    buffer[6] = mag >> 16;
    buffer[5] = mag >> 8;
    buffer[4] = mag >> 0;

	//Gain (x0x10000)
	gains[0] = round(gain0 * (double)0x10000);
	gains[1] = round(gain1 * (double)0x10000);
	gains[2] = round(gain2 * (double)0x10000);
	for(i=0;i<3;i++)
	{
		buffer[i*4+11] = gains[i] >> 24;
		buffer[i*4+10] = gains[i] >> 16;
		buffer[i*4+9] = gains[i] >> 8;
		buffer[i*4+8] = gains[i] >> 0;
	}

	//Offset
	offsets[0] = round(offset0 * (double)SPATIAL_HMC5883L_BITS_PER_GAUSS);
	offsets[1] = round(offset1 * (double)SPATIAL_HMC5883L_BITS_PER_GAUSS);
	offsets[2] = round(offset2 * (double)SPATIAL_HMC5883L_BITS_PER_GAUSS);
	for(i=0;i<3;i++)
	{
		buffer[i*4+23] = offsets[i] >> 24;
		buffer[i*4+22] = offsets[i] >> 16;
		buffer[i*4+21] = offsets[i] >> 8;
		buffer[i*4+20] = offsets[i] >> 0;
	}

	//Transforms (x0x10000)
	transforms[0] = round(T0 * (double)0x10000);
	transforms[1] = round(T1 * (double)0x10000);
	transforms[2] = round(T2 * (double)0x10000);
	transforms[3] = round(T3 * (double)0x10000);
	transforms[4] = round(T4 * (double)0x10000);
	transforms[5] = round(T5 * (double)0x10000);
	for(i=0;i<6;i++)
	{
		buffer[i*4+35] = transforms[i] >> 24;
		buffer[i*4+34] = transforms[i] >> 16;
		buffer[i*4+33] = transforms[i] >> 8;
		buffer[i*4+32] = transforms[i] >> 0;
	}

	if((result=CPhidgetGPP_setDeviceSpecificConfigTable((CPhidgetHandle)phid, buffer, SPATIAL_COMPASS_CALIB_TABLE_LENGTH, SPATIAL_COMPASS_CALIB_TABLE_INDEX))==EPHIDGET_OK)
		return CPhidgetGPP_writeFlash((CPhidgetHandle)phid);
	return result;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Spatial, PHIDCLASS_SPATIAL)

//event setup functions
CFHANDLE(Spatial, SpatialData, CPhidgetSpatial_SpatialEventDataHandle *, int)

CGET(Spatial,AccelerationAxisCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	MASGN(phid.attr.spatial.numAccelAxes)
}
CGET(Spatial,GyroAxisCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	MASGN(phid.attr.spatial.numGyroAxes)
}
CGET(Spatial,CompassAxisCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	MASGN(phid.attr.spatial.numCompassAxes)
}

CGETINDEX(Spatial,Acceleration,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTINDEX(phid.attr.spatial.numAccelAxes)
	TESTMASGN(accelAxis[Index], PUNK_DBL)

	MASGN(accelAxis[Index])
}

CGETINDEX(Spatial,AccelerationMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTINDEX(phid.attr.spatial.numAccelAxes)
	TESTMASGN(accelerationMax, PUNK_DBL)

	MASGN(accelerationMax)
}

CGETINDEX(Spatial,AccelerationMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTINDEX(phid.attr.spatial.numAccelAxes)
	TESTMASGN(accelerationMin, PUNK_DBL)

	MASGN(accelerationMin)
}

CGETINDEX(Spatial,AngularRate,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			TESTINDEX(phid.attr.spatial.numGyroAxes)
			TESTMASGN(gyroAxis[Index], PUNK_DBL)
			MASGN(gyroAxis[Index])
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Spatial,AngularRateMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			TESTINDEX(phid.attr.spatial.numGyroAxes)
			TESTMASGN(angularRateMax, PUNK_DBL)
			MASGN(angularRateMax)
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Spatial,AngularRateMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			TESTINDEX(phid.attr.spatial.numGyroAxes)
			TESTMASGN(angularRateMin, PUNK_DBL)
			MASGN(angularRateMin)
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Spatial,MagneticField,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			TESTINDEX(phid.attr.spatial.numCompassAxes)
			TESTMASGN(compassAxis[Index], PUNK_DBL)
			MASGN(compassAxis[Index])
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Spatial,MagneticFieldMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			TESTINDEX(phid.attr.spatial.numCompassAxes)
			TESTMASGN(magneticFieldMax, PUNK_DBL)
			MASGN(magneticFieldMax)
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Spatial,MagneticFieldMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			TESTINDEX(phid.attr.spatial.numCompassAxes)
			TESTMASGN(magneticFieldMin, PUNK_DBL)
			MASGN(magneticFieldMin)
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CSET(Spatial,DataRate,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTRANGE(phid->dataRateMax, phid->dataRateMin)

	//make sure it's a power of 2, or 1
	if(newVal < phid->interruptRate)
	{
		int temp = phid->dataRateMax;
		unsigned char good = FALSE;
		while(temp <= newVal)
		{
			if(temp == newVal)
			{
				good = TRUE;
				break;
			}
			temp *= 2;
		}
		if(!good)
			return EPHIDGET_INVALIDARG;
	}
	//make sure it's divisible by interruptRate
	else
	{
		if(newVal%phid->interruptRate)
			return EPHIDGET_INVALIDARG;
	}

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEY(DataRate, "%d", dataRate);
	else
		phid->dataRate = newVal;

	return EPHIDGET_OK;
}
CGET(Spatial,DataRate,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTMASGN(dataRate, PUNK_INT)

	MASGN(dataRate)
}

CGET(Spatial,DataRateMax,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTMASGN(dataRateMax, PUNK_INT)

	MASGN(dataRateMax)
}

CGET(Spatial,DataRateMin,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTMASGN(dataRateMin, PUNK_INT)

	MASGN(dataRateMin)
}

PHIDGET21_API int CCONV CPhidgetSpatial_zeroGyro(CPhidgetSpatialHandle phid)
{
	int result = EPHIDGET_OK;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	if(phid->phid.attr.spatial.numGyroAxes==0)
		return EPHIDGET_UNSUPPORTED;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		int newVal = phid->flip^1;
		ADDNETWORKKEY(ZeroGyro, "%d", flip);
	}
	else
	{
		switch(phid->phid.deviceUID)
		{
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
			{
				unsigned char buffer[8] = { 0 };
				buffer[0] = SPATIAL_ZERO_GYRO;
				result = CUSBSendPacket((CPhidgetHandle)phid, buffer);
				break;
			}
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
			case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:
				if(!phid->doZeroGyro)
				{
					phid->gyroZeroReadPtr = phid->bufferReadPtr;
					phid->doZeroGyro = PTRUE;
				}
				break;
			default:
				return EPHIDGET_UNEXPECTED;
		}
	}

	return EPHIDGET_OK;
}

PHIDGET21_API int CCONV CPhidgetSpatial_resetCompassCorrectionParameters(
	CPhidgetSpatialHandle phid)
{
	TESTPTR(phid) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
			{
				char newVal[1024];
				sprintf(newVal, "1,0,0,0,1,1,1,0,0,0,0,0,0");
				ADDNETWORKKEY(CompassCorrectionParams, "%s", compassCorrectionParamsString);
			}
			else
			{
				switch(phid->phid.deviceUID)
				{
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:

						phid->userMagField = 1;

						phid->userCompassOffset[0] = 0;
						phid->userCompassOffset[1] = 0;
						phid->userCompassOffset[2] = 0;

						phid->userCompassGain[0] = 1;
						phid->userCompassGain[1] = 1;
						phid->userCompassGain[2] = 1;

						phid->userCompassTransform[0] = 0;
						phid->userCompassTransform[1] = 0;
						phid->userCompassTransform[2] = 0;
						phid->userCompassTransform[3] = 0;
						phid->userCompassTransform[4] = 0;
						phid->userCompassTransform[5] = 0;
						break;
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
						return setCompassCorrectionTable_inFimrware(phid,
							1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0);
					default:
						return EPHIDGET_UNEXPECTED;
				}
			}
			return EPHIDGET_OK;
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}
PHIDGET21_API int CCONV CPhidgetSpatial_setCompassCorrectionParameters(
	CPhidgetSpatialHandle phid, 
	double magField, 
	double offset0, double offset1, double offset2, 
	double gain0, double gain1, double gain2, 
	double T0, double T1, double T2, double T3, double T4, double T5)
{
	TESTPTR(phid) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS:
			//Magnetic Field 0.1-1000
			if(magField < 0.1 || magField > 1000)
				return EPHIDGET_INVALIDARG;
			//Offsets need to be 0+-5.0
			if(offset0 < -5 || offset0 > 5)
				return EPHIDGET_INVALIDARG;
			if(offset1 < -5 || offset1 > 5)
				return EPHIDGET_INVALIDARG;
			if(offset2 < -5 || offset2 > 5)
				return EPHIDGET_INVALIDARG;
			//Gains need to be 0-15.0
			if(gain0 < 0 || gain0 > 15)
				return EPHIDGET_INVALIDARG;
			if(gain1 < 0 || gain1 > 15)
				return EPHIDGET_INVALIDARG;
			if(gain2 < 0 || gain2 > 15)
				return EPHIDGET_INVALIDARG;
			//T params 0+-5.0
			if(T0 < -5 || T0 > 5)
				return EPHIDGET_INVALIDARG;
			if(T1 < -5 || T1 > 5)
				return EPHIDGET_INVALIDARG;
			if(T2 < -5 || T2 > 5)
				return EPHIDGET_INVALIDARG;
			if(T3 < -5 || T3 > 5)
				return EPHIDGET_INVALIDARG;
			if(T4 < -5 || T4 > 5)
				return EPHIDGET_INVALIDARG;
			if(T5 < -5 || T5 > 5)
				return EPHIDGET_INVALIDARG;

			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
			{
				char newVal[1024];
				sprintf(newVal, "%lE,%lE,%lE,%lE,%lE,%lE,%lE,%lE,%lE,%lE,%lE,%lE,%lE",
					magField, offset0, offset1, offset2, gain0, gain1, gain2, T0, T1, T2, T3, T4, T5);
				ADDNETWORKKEY(CompassCorrectionParams, "%s", compassCorrectionParamsString);
			}
			else
			{
				switch(phid->phid.deviceUID)
				{
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056:
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN:

						phid->userMagField = magField;

						phid->userCompassOffset[0] = offset0;
						phid->userCompassOffset[1] = offset1;
						phid->userCompassOffset[2] = offset2;

						phid->userCompassGain[0] = gain0;
						phid->userCompassGain[1] = gain1;
						phid->userCompassGain[2] = gain2;

						phid->userCompassTransform[0] = T0;
						phid->userCompassTransform[1] = T1;
						phid->userCompassTransform[2] = T2;
						phid->userCompassTransform[3] = T3;
						phid->userCompassTransform[4] = T4;
						phid->userCompassTransform[5] = T5;
						break;

					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
					case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
						return setCompassCorrectionTable_inFimrware(phid,
							magField, offset0, offset1, offset2,
							gain0, gain1, gain2, T0, T1, T2, T3, T4, T5);
					default:
						return EPHIDGET_UNEXPECTED;
				}
			}
			return EPHIDGET_OK;
		case PHIDID_SPATIAL_ACCEL_3AXIS:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}


CSET(Spatial,AnalogDigitalMode,CPhidgetSpatial_AnalogDigitalMode)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTRANGE(SPATIAL_ANALOG_AND_DIGITAL, SPATIAL_DIGITAL)

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1043:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
		{
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
			{
				//TODO
			}
			else
			{
				unsigned char buffer[8] = { 0 };
				buffer[0] = SPATIAL_SET_POLLING_TYPE;
				buffer[1] = newVal;
				return CUSBSendPacket((CPhidgetHandle)phid, buffer);
			}
		}
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

PHIDGET21_API int CCONV CPhidgetSpatial_unZeroGyro(CPhidgetSpatialHandle phid)
{
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
		{
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
			{
				//TODO
			}
			else
			{
				unsigned char buffer[8] = { 0 };
				buffer[0] = SPATIAL_UNZERO_GYRO;
				return CUSBSendPacket((CPhidgetHandle)phid, buffer);
			}
		}
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

PHIDGET21_API int CCONV CPhidgetSpatial_setDigitalGyroCalibrationValues(CPhidgetSpatialHandle phid,
	double gainPositive[3], double gainNegative[3], double offset[3], double factor1[3], double factor2[3])
{
	return setCalibrationValues_inFirmware(phid, SPATIAL_GyroCalibTable_ID, SPATIAL_DIGITAL_GYRO_CALIB_TABLE_INDEX,
		gainPositive, gainNegative, offset, factor1, factor2);
}

PHIDGET21_API int CCONV CPhidgetSpatial_setAnalogGyroCalibrationValues(CPhidgetSpatialHandle phid,
	double gainPositive[3], double gainNegative[3], double offset[3], double factor1[3], double factor2[3])
{
	return setCalibrationValues_inFirmware(phid, SPATIAL_GyroCalibTable_ID, SPATIAL_ANALOG_GYRO_CALIB_TABLE_INDEX,
		gainPositive, gainNegative, offset, factor1, factor2);
}

PHIDGET21_API int CCONV CPhidgetSpatial_setDigitalAccelCalibrationValues(CPhidgetSpatialHandle phid,
	double gainPositive[3], double gainNegative[3], double offset[3], double factor1[3], double factor2[3])
{
	return setCalibrationValues_inFirmware(phid, SPATIAL_AccelCalibTable_ID, SPATIAL_DIGITAL_ACCEL_CALIB_TABLE_INDEX,
		gainPositive, gainNegative, offset, factor1, factor2);
}

PHIDGET21_API int CCONV CPhidgetSpatial_setAnalogAccelCalibrationValues(CPhidgetSpatialHandle phid,
	double gainPositive[3], double gainNegative[3], double offset[3], double factor1[3], double factor2[3])
{
	return setCalibrationValues_inFirmware(phid, SPATIAL_AccelCalibTable_ID, SPATIAL_ANALOG_ACCEL_CALIB_TABLE_INDEX,
		gainPositive, gainNegative, offset, factor1, factor2);
}



//Maybe add these later
/*
CGET(Spatial,GyroHeading,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTMASGN(gyroHeading, PUNK_DBL)

	MASGN(gyroHeading)
}

CGET(Spatial,CompassHeading,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SPATIAL)
	TESTATTACHED
	TESTMASGN(compassHeading, PUNK_DBL)

	MASGN(compassHeading)
}*/
