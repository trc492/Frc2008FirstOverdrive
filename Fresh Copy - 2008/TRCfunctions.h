/*******************************************************************************
* FILE NAME: TRCfunctions.h
*
* AUTHOR: Paul Malmsten, 2008
*
* DESCRIPTION: 
*  This is the include file which corresponds to TRCfunctions.c
*  It contains some aliases and function prototypes used in those files.
*
*******************************************************************************/
#include "user_routines.h"

//Base Drive Aliases
#define L_CIM_1					pwm15
#define L_CIM_2					pwm16
#define L_ENC_COUNT				Get_Encoder_1_Count()

#define R_CIM_1					pwm13
#define R_CIM_2					pwm14
#define R_ENC_COUNT 			Get_Encoder_2_Count()

#define R_MOTOR_DIR 			1					//1 is forward, -1 is backward     
#define L_MOTOR_DIR				-1					//1 is forward, -1 is backward

//Base Preferences
//#define DRIVE_POSITION_MODE						//Uncomment to enable position control; Vsetpoint will be overwritten!
//#define DRIVE_DEBUG								//Uncomment to print Vel/Pos loop values

//Accessory Drive Aliases
#define COMP_FWD				relay1_fwd
#define COMP_REV				relay1_rev

#define STOP					127

//Sensor Aliases
#define COMP_PRESSURE			rc_dig_in18

#define GYRO_BIAS   			963					//Set for Mnemosyne; used when GYRO_CAL is undefined
//#define GYRO_CAL 									//Uncomment to calibrate gyro bias; saves time starting the robot when unset

//Operator Interface Aliases
#define RAW_X					p2_x
#define RAW_Y					p1_y

#define SPEED_OFFSET			-3
#define SPEED_DB				9

#define YAW_OFFSET 				-6
#define YAW_DB					12

#ifdef DRIVE_POSITION_MODE
	Left.Mode = Right.Mode = 1;								//Set to position mode
#endif

int doPid(PosServo *p, long pos, int motor_direction);		//Supplied with position (PID)
int doVelPid(VelServo *p, long vel, int motor_direction);	//Supplied with velocity (Position/Velocity cascade loop)
int encoder_check(void);
int deadBandInput(int in, int deadband);
int abs(int val);
int turn(void);
void Launcher_State_Machine(void);
int Launcher_Release(void);	
void configTerminal(void);

