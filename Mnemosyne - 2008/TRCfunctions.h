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
//#define ELEV_MOTOR_DIR  		-1					//1 is forward, -1 is backward


//Base Preferences
//#define SMART_TURN								//Uncomment for proportional acceleration out of turn
//#define DRIVE_POSITION_MODE						//Uncomment to enable position control; Vsetpoint will be overwritten!
//#define DRIVE_DEBUG								//Uncomment to print Vel/Pos loop values


//Accessory Drive Aliases
#define FORK					pwm02
#define COMP_FWD				relay1_fwd
#define COMP_REV				relay1_rev
#define LAUNCHER				pwm01
#define LAUNCHER_FWD			255
#define LAUNCHER_REV			0
#define LAUNCHER_RELEASE		pwm03
#define	LAUNCHER_RELEASE_FWD	120
#define	LAUNCHER_RELEASE_REV	150

#define STOP					127

//Sensor Aliases
#define	LAUNCHER_LIM			(!rc_dig_in15)
//#define IR_BOARD				rc_dig_in14

#define GYRO_BIAS   			963					//Set for Mnemosyne; used when GYRO_CAL is undefined
//#define GYRO_CAL 									//Uncomment to calibrate gyro bias; saves time starting the robot when unset

#define COMP_PRESSURE			rc_dig_in18


//Operator Interface Aliases
#define RAW_X					p2_x
#define RAW_Y					p1_y

#define	RAW_FORK				p3_y
#define LAUNCHER_SW_FIRE		p3_sw_trig
#define LAUNCHER_SW_SAFETY		p3_sw_aux1

#define SPEED_OFFSET			-3
#define SPEED_DB				9

#define YAW_OFFSET 				-6
#define YAW_DB					12

#define FORK_OFFSET 			-5
#define FORK_DB					8


//Launcher States
#define LAUNCHER_ARMING			0
#define LAUNCHER_ARMED			1
#define LAUNCHER_DISARM			2

//Launcher Settings
#define LAUNCHER_ARM_TIME		200					//Timer ticks; 4 seconds
#define LAUNCHER_PREARM_TIME	200					//Timer ticks; 1 second



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

