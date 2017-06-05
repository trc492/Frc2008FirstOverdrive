#if 0
/// Copyright (c) 2008 Titan Robotics Club. All rights are reserved.
///
/// <module name="project.h" />
///
/// <summary>
///     This module is the main header for the project code. It contains the configuration parameters
///     and includes other header files in dependency order.
/// </summary>
///
/// <remarks>
///     Author: Paul Malmsten (pmalmsten@gmail.com)
///             Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _project_h
#define _project_h

#define _USE_CONSOLE_

//
// Project constants.
//
#define ORGANIZATION            "Titan Robotics Club"
#define PROJECT                 "Oceanus"
#define YEAR                    2008
#define VERSION_MAJOR           0
#define VERSION_MINOR           5
#define VERSION_RELEASE         1

#define PROJF_SERVO             0x00000001

// +====================================================+
// |            Device Configuration Begins             |
// +====================================================+

//
// Configuration of RC digital I/O and Relays.
// ===========================================
//
#define PneumaticSwitch         rc_dig_in18
#define Dislodger_Fwd           relay1_fwd
#define Dislodger_Rev           relay1_rev
#define _Fwd                    relay4_fwd
#define _Rev                    relay4_rev
#include "rcio.h"

//
// Configuration of PWMs.
// ======================
//
#define R_MOTOR_1               pwm13
#define R_MOTOR_2               pwm14
#define L_MOTOR_1               pwm15
#define L_MOTOR_2               pwm16
#include "pwm.h"

//
// Configuration of A/D converters.
// ================================
//
#define NUM_ADC_CHANNELS        2
#define ADC_ELEV_POS            1
#include "adc.h"

//
// Configuration of Encoders.
// ==========================
//
#define NUM_ENCODERS            2
#define ENCODER_LEFT            0
#define ENCODER_RIGHT           1
#include "encoder.h"

//
// Configuration of Gyro.
// ======================
//
#define GYRO_ADC_CHANNEL        0

// Pick your gyro by uncommenting one of the six lines below.
//#define GYROCHIP_64             // BEI GyroChip AQRS-00064-xxx
//#define GYROCHIP_75	          // BEI GyroChip AQRS-00075-xxx
//#define ENV_05D                 // Murata ENV-05D
//#define CRS03	                  // Silicon Sensing Systems' CRS03-02
//#define ADXRS401                // Analog Devices' ADXRS401
#define ADXRS150                // Analog Devices' ADXRS150
// #define ADXRS300               // Analog Devices' ADXRS300

// For optimum performance, you'll need to calibrate the scaling factor
// to match that of your gyro's. One way to calibrate your gyro is to
// mount it very securely to a hefty, square or rectangular object.
// Mounting the gyro to a hefty object will help dampen higher frequency
// vibrations that can adversly effect your measurements. Place the
// mounted gyro against another square object when using the built-in
// calibration function. To get good results, the mount must be
// absolutely still when calibrating. After a few seconds, the gyro
// angular rate and angle will be sent to the terminal screen. If the
// angle drifts rapidly while the mounted gyro is motionless, you need
// to restart the calibration to acquire a new gyro bias measurement. Again,
// gyros are very sensitive and must be still while the bias is calculated.
// Once the gyro is running with little drift, rotate the mount 180 degrees
// and note the reported angle. If the angular units are set to tenths
// of a degree, the ideal reported angle is 1800. If set to milliradians,
// the ideal angle 1s 3142 (Pi times a thousand). For every tenth of a
// percent that the angle is high, add one to GYRO_SCALE_FACTOR.
// Conversly, for every tenth of a percent low, subtrace one from
// GYRO_SCALE_FACTOR. Repeat until you're satisfied with the accuracy.
#define GYRO_SCALE_FACTOR       1000

// Measured angular rates will only be used if their absolute value is
// greater than the calibrated noise threshold plus this value. This will
// help filter out measurement noise at the expense of measuring small
// angular rates. To use this feature, start with a value of two, re-compile
// and test. If your angular drift is now zero, iterate the value down to the
// lowest value that still gives you zero drift over a one or two minute
// period. If your angular drift didn't go to zero with a value of two,
// iterate this value up until the angular drift rate is zero for a minute or
// two.
#define GYRO_NOISE_MARGIN       1
#include "gyro.h"

//
// Configuration of OI inputs.
// ===========================
//
#define NUM_JOYSTICKS                   4
#define JOYSTICK_X                      g_InputData.Joysticks[1].ix
#define JOYSTICK_Y                      g_InputData.Joysticks[0].iy
#define JOYSTICK_Z                      g_InputData.Joysticks[2].iy
#include "input.h"

//
// Configuration of Servos.
// ========================
//
#define R_MOTOR_DIR             1       //1 is forward, -1 is backward
#define L_MOTOR_DIR             -1      //1 is forward, -1 is backward
#define DRIVE_ACCEL_RATE        100
#define DRIVE_DECEL_RATE        -100
#define DRIVE_VEL_LIMIT         70
#define TURN_VEL_LIMIT          40
#define DRIVE_WHEELBASE_NUMER   49
#define DRIVE_WHEELBASE_DENOM   2
#define DRIVE_WHEELBASE         DRIVE_WHEELBASE_NUMER/DRIVE_WHEELBASE_DENOM
#include "servo.h"

// +====================================================+
// |            Device Configuration Ends               |
// +====================================================+

//
// Global data.
//
extern DWORD    g_dwProjFlags;
extern PIDPARAM g_DrivePid;
extern SERVO    g_LeftMotor;
extern SERVO    g_RightMotor;

//
// Exported function prototypes.
//
void
ProjInitialize(
    void
    );
    
void
ProjProcessData(
    void
    );
    
void
ProjAutonomousCode(
    void
    );
    
void
ProjProcessLocalIO(
    void
    );

// This function is in command.c.
void
ProjCmdConsoleInit(
    __in BYTE bSerialPort
    );
    
void
ProjCmdConsoleTrace(
    void
    );
    
#endif
