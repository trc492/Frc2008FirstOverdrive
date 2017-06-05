#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="gyro.h" />
///
/// <summary>
///     This module contains the definitions of the gyro sensor module.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on code by: R. Kevin Watson (kevinw@jpl.nasa.gov)
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _gyro_h
#define _gyro_h

//
// Pick the angular unit by uncommenting one of the two lines below.
//
#define TENTHS_OF_A_DEGREE
//#define MILLI_RADIANS

//
// If you modify stuff below this line, you'll break the software.
//
// These constants convert the gyro output voltage, expressed in volts,
// to an angular rate of change in either tenths of a degree per second
// or milliradians per second.
//
// BEI GyroChip AQRS-00064-xxx (sensitivity = 35.0mV/deg/sec)
#ifdef GYROCHIP_64
#define GYRO_SENSITIVITY_DEG 286L       // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 499L       // in units of milliradians/sec/volt
#endif

// BEI GyroChip AQRS-00075-xxx (sensitivity = 30.0mV/deg/sec)
#ifdef GYROCHIP_75
#define GYRO_SENSITIVITY_DEG 333L       // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 581L       // in units of milliradians/sec/volt
#endif

// Murata ENV-05D (sensitivity = 25mV/deg/sec)
#ifdef ENV_05D
#define GYRO_SENSITIVITY_DEG 400L       // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 698L       // in units of milliradians/sec/volt
#endif

// Silicon Sensing Systems' CRS03 (sensitivity = 20mV/deg/sec)
#ifdef CRS03
#define GYRO_SENSITIVITY_DEG 500L       // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 873L       // in units of milliradians/sec/volt
#endif

// Analog Devices' ADXRS401 (sensitivity = 15mV/deg/sec)
#ifdef ADXRS401
#define GYRO_SENSITIVITY_DEG 667L       // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 1163L      // in units of milliradians/sec/volt
#endif

// Analog Devices' ADXRS150 (sensitivity = 12.5mV/deg/sec)
#ifdef ADXRS150
#define GYRO_SENSITIVITY_DEG 800L       // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 1396L      // in units of milliradians/sec/volt
#endif

// Analog Devices' ADXRS300 (sensitivity = 5.0mV/deg/sec)
#ifdef ADXRS300
#define GYRO_SENSITIVITY_DEG 2000L      // in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 3490L      // in units of milliradians/sec/volt
#endif
	
// Select the appropriate angular unit
#ifdef MILLI_RADIANS
#define GYRO_SENSITIVITY        GYRO_SENSITIVITY_RAD
#endif

#ifdef TENTHS_OF_A_DEGREE
#define GYRO_SENSITIVITY        GYRO_SENSITIVITY_DEG
#endif

//
// Exported function prototypes.
//
void
GyroInitialize(
    __in BYTE bADCChannel,
    __in WORD wSensitivity,
    __in WORD wScaleFactor,
    __in WORD wNoiseMargin
    );

void
GyroCalibrate(
    void
    );
    
int
GyroGetRate(
    void
    );

long
GyroGetAngle(
    void
    );

WORD
GyroGetBias(
    void
    );

void
GyroSetBias(
    __in WORD wBias
    );

void
GyroResetAngle(
    void
    );

void
GyroPrintData(
    void
    );
    
void
GyroProcessData(
    void
    );

#endif

