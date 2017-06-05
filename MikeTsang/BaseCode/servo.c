#if 0
/// Copyright (c) 2008 Titan Robotics Club. All rights are reserved.
///
/// <module name="servo.c" />
///
/// <summary>
///     This module contains the robot drive functions.
/// </summary>
///
/// <remarks>
///     Author: Paul Malmsten (pmalmsten@gmail.com)
///             Michael Tsang (mikets@hotmail.com)
///     Date:   15-Mar-2008
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "servo.h"

//
// Local constants.
//

// MoveTo state machine: state values.
#define MSTATE_MOVE_NORMAL      0
#define MSTATE_MOVE_ENDZONE     1

/// <summary>
///     This function initializes a servo motor structure.
/// </summary>
///
/// <param name="Servo">
///     Points to the SERVO structure to be initialized.
/// </param>
/// <param name="PidParam">
///     Points to the PIDPARAM structure for use with this Servo.
/// </param>
/// <param name="bEncoder">
///     Specifies the encoder channel.
/// </param>
/// <param name="iVelLimit">
///     Specifies velocity limit. If 0, it means no limit.
/// </param>
/// <param name="iMotorDir">
///     Specifies the direction of the motor.
/// </param>
///
/// <returns> None. </returns>

void
ServoInitialize(
    __out PSERVO   Servo,
    __in PPIDPARAM PidParam,
    __in BYTE      bEncoder,
    __in int       iVelLimit,
    __in int       iMotorDir
    )
{
    Servo->PidParam = PidParam;
    Servo->bEncoder = bEncoder;
    Servo->lPosSp = 0;
    Servo->iVelSp = 0;
    Servo->lIntErr = 0;
    Servo->iVelLimit = iVelLimit;
    Servo->iMotorDir = iMotorDir;
    return;
}       //ServoInitialize

/// <summary>
///     This function applies the PID algorithm to calculate the new drive
///     value.
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

int
ServoPid(
    __inout PSERVO Servo,
    __in long      lPos,
    __in long      lVel,
    __in BOOL      fPosPid
    )
{
    int iVelErr;
    int iDrive;

    if (fPosPid)
    {
        Servo->iVelSp = ((Servo->lPosSp - lPos)*Servo->PidParam->Kp)/
                        Servo->PidParam->Ko;
    }
    
    //Enforce Veloctiy Limit
    if (Servo->iVelLimit != 0)
    {
        if (Servo->iVelSp > Servo->iVelLimit)
        {
            Servo->iVelSp = Servo->iVelLimit;
        }
        else if (Servo->iVelSp < -Servo->iVelLimit)
        {
            Servo->iVelSp = -Servo->iVelLimit;
        }
    }

    //Velocity Error
    iVelErr = Servo->iVelSp - (int)lVel;
    
    //Velocity control loop: ((VErr + (Ki*IErr)/Ko)*Kv)/Ko
    iDrive = ((iVelErr +
               (Servo->PidParam->Ki*Servo->lIntErr)/Servo->PidParam->Ko)*
              Servo->PidParam->Kv)/Servo->PidParam->Ko;

    //Limit drive output
    if (iDrive > 127)
    {
        iDrive = 127;
    }
    else if (iDrive < -127)
    {
        iDrive = -127;
    }
    else
    {
        Servo->lIntErr += iVelErr;
    }

    return iDrive;
}       //ServoPid

/// <summary>
///     This function initializes the MOVETO structure for autonomous drive.
/// </summary>
///
/// <param name="MoveTo">
///     Points to the MOVETO structure to be initialized.
/// </param>
/// <param name="Drive">
///     Points to the DRIVE structure that contains the motor information.
/// </param>
/// <param name="lHeading">
///     Specifies the absolute heading we should be at after the state machine
///     ended.
/// </param>
/// <param name="lDistance">
///     Specifies the absolute distance we should be at after the state machine
///     ended.
/// </param>
/// <param name="iTurnVel">
///     Specifies the turning velocity.
/// </param>
/// <param name="iDriveVel">
///     Specifies the velocity when driving in a straight line.
/// </param>
/// <param name="iEndVel">
///     Specifies the ending velocity when the state machine ended.
/// </param>
/// <param name="lTurnThreshold">
///     Specifies the amount of turn we need to exceed before we classify
///     it being a turn. This is to determine if we need to slow down to
///     turn velocity. If we are just adjusting our heading when driving
///     a straight line, that's not turning and we don't need to slow down.
/// </param>
/// <param name="lAngleTolerance">
///     Specifies the amount of angle tolerance we allow comparing to the
///     target heading. This is to prevent the oscillation caused by minor
///     angle variation.
/// </param>
///
/// <returns> None. </returns>

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
    )
{
    int iVelDiff;
    int iTimeDiff;
    
    MoveTo->lTargetHeading = lHeading;
    MoveTo->lTargetPos = (EncoderGetCount(Drive->Left->bEncoder) +
                          EncoderGetCount(Drive->Right->bEncoder))/2 +
                         lDistance;
    MoveTo->iTurnVel = iTurnVel;
    MoveTo->iDriveVel = iDriveVel;
    MoveTo->iEndVel = iEndVel;
    MoveTo->lTurnThreshold = lTurnThreshold;
    MoveTo->lAngleTolerance = lAngleTolerance;
    
    iVelDiff = iEndVel - iDriveVel;
    if (iVelDiff < 0)
    {
        //
        // We will decelerate.
        // Note: iDecelRate is negative by definition.
        //
        iTimeDiff = iVelDiff/Drive->iDecelRate;
    }
    else
    {
        //
        // We will accelerate.
        //
        iTimeDiff = iVelDiff/Drive->iAccelRate;
    }
    //
    // Calculate the end zone position where we need to change to End Velocity.
    // Accel or Decel distance = ((Vd + Ve)/2)*(Accel or Decel period)
    // Pos = TargetPos - Accel or Decel distance
    //
    MoveTo->lEndZonePos = MoveTo->lTargetPos -
                          ((iDriveVel + iEndVel)*iTimeDiff)/2;
    MoveTo->bState = MSTATE_MOVE_NORMAL;
    
    return;
}       //ServoInitMoveTo

/// <summary>
///     This function implements the state machine that turns the robot
///     to the target heading and drives it to the target position.
/// </summary>
///
/// <param name="MoveTo">
///     Points to the MOVETO structure that contains the target and state
///     information.
/// </param>
/// <param name="Drive">
///     Points to the DRIVE structure that contains the motor information.
/// </param>
///
/// <returns> Returns FALSE when the state machine is done. </returns>

BOOL
ServoMoveTo(
    __inout PMOVETO MoveTo,
    __in    PDRIVE  Drive
    )
{
    BOOL rc = TRUE;
    long lPos;
    long lHeading;
    
    lPos = (EncoderGetCount(Drive->Left->bEncoder) +
            EncoderGetCount(Drive->Right->bEncoder))/2;
    lHeading = GyroGetAngle();
    switch (MoveTo->bState)
    {
        case MSTATE_MOVE_NORMAL:
            if (lPos >= MoveTo->lEndZonePos)
            {
                //
                // We crossed to the end zone, set TargetVel to EndVel.
                //
                MoveTo->iTargetVel = MoveTo->iEndVel;
                MoveTo->bState = MSTATE_MOVE_ENDZONE;
            }
            else if (abs(MoveTo->lTargetHeading - lHeading) >
                     MoveTo->lTurnThreshold)
            {
                //
                // This is a turn, let's set the TargetVel to TurnVel.
                //
                MoveTo->iTargetVel = MoveTo->iTurnVel;
            }
            else
            {
                //
                // We are driving in a straight line, set TargetVel
                // to DriveVel.
                //
                MoveTo->iTargetVel = MoveTo->iDriveVel;
            }
            //
            // Let it fall through.
            //
        case MSTATE_MOVE_ENDZONE:
            if (lPos >= MoveTo->lTargetPos)
            {
                //
                // We reached target, done!
                //
                if (MoveTo->iTargetVel == 0)
                {
                    //
                    // Make sure if we want off, we will have off before
                    // we quit.
                    //
                    L_MOTOR_1 = L_MOTOR_2 = R_MOTOR_1 = R_MOTOR_2 = 127;
                }
                rc = FALSE;
            }
            //
            // Now let's determine the wheel vel differential.
            //
            else
            {
                if (abs(MoveTo->lTargetHeading - lHeading) <
                    MoveTo->lAngleTolerance)
                {
                    //
                    // We are pretty much on target in terms of heading.
                    //
                    Drive->Left->iVelSp = MoveTo->iTargetVel;
                    Drive->Right->iVelSp = MoveTo->iTargetVel;
                }
                else
                {
                    int iVelDiff;
                
                    Drive->Turn->lPosSp = MoveTo->lTargetHeading;
                    iVelDiff = ServoPid(Drive->Turn,
                                        lHeading,
                                        GyroGetRate(),
                                        TRUE)/2;
                    Drive->Left->iVelSp = MoveTo->iTargetVel - iVelDiff;
                    Drive->Right->iVelSp = MoveTo->iTargetVel + iVelDiff;
                }

                L_MOTOR_1 = L_MOTOR_2 =
                    ServoPidDrive(Drive->Left,
                                  0,
                                  EncoderGetVelocity(Drive->Left->bEncoder),
                                  FALSE);
                R_MOTOR_1 = R_MOTOR_2 =
                    ServoPidDrive(Drive->Right,
                                  0,
                                  EncoderGetVelocity(Drive->Right->bEncoder),
                                  FALSE);
            }
            break;
            
        default:
            printf("Assert: invalid MoveTo state %d.\r\n", MoveTo->bState);
            rc = FALSE;
            break;
    }
    
    return rc;
}       //ServoMoveTo

/// <summary>
///     This function calibrates the servo by determining the acceleration and deceleration rates.
/// </summary>
///
/// <param name="Drive">
///     Points to the DRIVE structure that contains the motor information.
/// </param>
///
/// <returns> Returns FALSE when the state machine is done. </returns>

BOOL
ServoCalibrate(
    __out PDRIVE Drive
    )
{
    BOOL rc = TRUE;
    
    
    
    return rc;
}       //ServoCalibrate

/// <summary>
///     This function prints the PID control parameters.
/// </summary>
///
/// <param name="PidParam">
///     Points to the PIDPARAM structure.
/// </param>
///
/// <returns> None. </returns>

void
ServoPrintPidParam(
    __in PPIDPARAM PidParam
    )
{
    printf("PID: Kp=%d, Kv=%d, Ki=%d, Ko=%d\r\n",
           PidParam->Kp, PidParam->Kv, PidParam->Ki, PidParam->Ko);
    return;
}       //ServoPrintPidParam

/// <summary>
///     This function prints the servo motor data.
/// </summary>
///
/// <param name="pszName">
///     Points to the servo motor name string.
/// </param>
/// <param name="Servo">
///     Points to the SERVO structure.
/// </param>
///
/// <returns> None. </returns>

void
ServoPrintData(
    __in PSTR   pszName,
    __in PSERVO Servo
    )
{
    printf("%s: PSp=%ld, VSp=%d, IntErr=%ld, VelLim=%d, MotorDir=%d\r\n",
           pszName, Servo->lPosSp, Servo->iVelSp, Servo->lIntErr,
           Servo->iVelLimit, Servo->iMotorDir);
    return;
}       //ServoPrintData

