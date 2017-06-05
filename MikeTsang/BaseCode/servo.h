#if 0
/// Copyright (c) 2008 Titan Robotics Club. All rights are reserved.
///
/// <module name="servo.h" />
///
/// <summary>
///     This module contains the definition of the servo motor functions.
/// </summary>
///
/// <remarks>
///     Author: Paul Malmsten (pmalmsten@gmail.com)
///             Michael Tsang (mikets@hotmail.com)
///     Date:   15-Mar-2008
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _servo_h
#define _servo_h

//
// Exported macros.
//

/// <summary>
///     This macro applies the PID algorithm to calculate the new drive
///     value and normalize it to the PWM value and adjust the motor direction.
/// </summary>
///
/// <param name="Servo">
///     Points to the SERVO structure.
/// </param>
/// <param name="lPos">
///     Specifies the current position. This is ignore if fPosPid is FALSE.
/// </param>
/// <param name="lVel">
///     Specifies the current velocity.
/// </param>
/// <param name="fPosPid">
///     If TRUE, apply position PID, otherwise apply velocity PID.
/// </param>
///
/// <returns> Returns the new drive value. </returns>

#define ServoPidDrive(Servo,lPos,lVel,fPosPid)  \
                (ServoPid(Servo,lPos,lVel,fPosPid)*(Servo)->iMotorDir + 127)

//
// Exported type definitions.
//

/// <summary>
///     This structure contains the PID control constants.
/// </summary>
typedef struct _PidParam
{
    /// <summary>
    ///     Position gain constant.
    /// </summary>
    int         Kp;
    /// <summary>
    ///     Velocity gain constant.
    /// </summary>
    int         Kv;
    /// <summary>
    ///     Integral gain constant.
    /// </summary>
    int         Ki;
    /// <summary>
    ///     Output scale factor.
    /// </summary>
    int         Ko;
} PIDPARAM, *PPIDPARAM;

/// <summary>
///     This structure contains the servo control data of a motor.
/// </summary>
typedef struct _Servo
{
    /// <summary>
    ///     Points to the PIDPARAM structure.
    /// </summary>
    PPIDPARAM   PidParam;
    /// <summary>
    ///     Specifies the encoder channel.
    /// </summary>
    BYTE        bEncoder;
    /// <summary>
    ///     Stores the position setpoint.
    /// </summary>
    long        lPosSp;
    /// <summary>
    ///     Stores the velocity setpoint in clicks/tick.
    /// </summary>
    int         iVelSp;
    /// <summary>
    ///     Stores the Integral Error value.
    /// </summary>
    long        lIntErr;
    /// <summary>
    ///     Stores the velocity limit in clicks/tick.
    /// </summary>
    int         iVelLimit;
    /// <summary>
    ///     Stores the drive output value.
    /// </summary>
    int         iMotorDir;
} SERVO, *PSERVO;

/// <summary>
///     This structure describes the driving system.
/// </summary>
typedef struct _Drive
{
    /// <summary>
    ///     Points to the left motor servo structure.
    /// </summary>
    PSERVO      Left;
    /// <summary>
    ///     Points to the right motor servo structure.
    /// </summary>
    PSERVO      Right;
    /// <summary>
    ///     Points to the turn servo structure.
    /// </summary>
    PSERVO      Turn;
    /// <summary>
    ///     Specifies the robot's acceleration rate in clicks/(ticks^2).
    /// </summary>
    int         iAccelRate;
    /// <summary>
    ///     Specifies the robot's deceleration rate in clicks/(ticks^2).
    /// </summary>
    int         iDecelRate;
} DRIVE, *PDRIVE;

/// <summary>
///     This structure contains the MoveTo state machine info.
/// </summary>
typedef struct _MoveTo
{
    /// <summary>
    ///     Stores the state of the state machine.
    /// </summary>
    BYTE        bState;
    /// <summary>
    ///     Stores the target heading.
    /// </summary>
    long        lTargetHeading;
    /// <summary>
    ///     Stores the target position.
    /// </summary>
    long        lTargetPos;
    /// <summary>
    ///     Stores the turning velocity.
    /// </summary>
    int         iTurnVel;
    /// <summary>
    ///     Stores the driving velocity.
    /// </summary>
    int         iDriveVel;
    /// <summary>
    ///     Stores the ending velocity.
    /// </summary>
    int         iEndVel;
    /// <summary>
    ///     Stores the ending velocity.
    /// </summary>
    int         iTargetVel;
    /// <summary>
    ///     Stores the current target velocity for PID calculation.
    /// </summary>
    long        lTurnThreshold;
    /// <summary>
    ///     Stores the angle tolerance for which it will be
    ///     considered heading on target if we don't exceed it.
    /// </summary>
    long        lAngleTolerance;
    /// <summary>
    ///     Stores the end zone position where we need to change
    ///     the velocity to EndVel allowing the distance to
    ///     accelerate or decelerate.
    /// </summary>
    long        lEndZonePos;
} MOVETO, *PMOVETO;

//
// Exported function prototypes.
//
void
ServoInitialize(
    __out PSERVO   Servo,
    __in PPIDPARAM PidParam,
    __in BYTE      bEncoder,
    __in int       iVelLimit,
    __in int       iMotorDir
    );
    
int
ServoPid(
    __inout PSERVO Servo,
    __in long      lPos,
    __in long      lVel,
    __in BOOL      fPosPid
    );

void
ServoInitMoveTo(
    __out PMOVETO MoveTo,
    __in PDRIVE   Drive,
    __in long     lHeading,
    __in long     lDistance,
    __in int      iTurnVel,
    __in int      iDriveVel,
    __in int      iEndVel,
    __in long     lTurnThreshold,
    __in long     lAngleTolerance
    );

BOOL
ServoMoveTo(
    __inout PMOVETO MoveTo,
    __in    PDRIVE  Drive
    );
    
void
ServoPrintPidParam(
    __in PPIDPARAM PidParam
    );

void
ServoPrintData(
    __in PSTR   pszName,
    __in PSERVO Servo
    );

#endif
