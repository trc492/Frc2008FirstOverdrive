/*******************************************************************************
* FILE NAME: TRCfunctions.h
*
* AUTHOR: Paul Malmsten, 2009
*
* DESCRIPTION: 
*  This is the include file which corresponds to TRCfunctions.c
*  It contains some aliases and function prototypes used in those files.
*
*******************************************************************************/
#include "user_routines.h"

//--Drive Configuration
#define DRIVE_DEBUG										//Uncomment to print Vel/Pos loop values
#define L_CIM_1					pwm15
#define L_CIM_2					pwm16
#define L_ENC_COUNT				Get_Encoder_2_Count()

#define R_CIM_1					pwm13
#define R_CIM_2					pwm14
#define R_ENC_COUNT 			Get_Encoder_1_Count()

#define R_MOTOR_DIR 			1					//1 is forward, -1 is backward     
#define L_MOTOR_DIR				1					//1 is forward, -1 is backward

#define WHL_SIZE				6					//Wheel Diameter
#define GEAR_RAITO				11					//In format of x:1 where x is GEAR_RAITO
#define PI_NUMER				31415
#define PI_DENOM				10000
#define WHL_CIRCUM				(WHL_SIZE * PI_NUMER)/PI_DENOM
#define ENCODER_CPR				64

#define INCHESTOCLICKS(x)		((x/WHL_CIRCUM)*GEAR_RAITO)*ENCODER_CPR

//--Input Configuration
#define SPEED					p1_y
#define SPEED_OFFSET			-3
#define SPEED_DB				9

#define YAW						p2_x
#define YAW_OFFSET 				-10
#define YAW_DB					10

#define KPUP					p2_sw_trig
#define KPDN					p2_sw_top
#define KDUP					p2_sw_aux1
#define KDDN					p2_sw_aux2

//--Gyro Configuration: gyro.h
#define GYRO_BIAS   963		//Set for Mnemosyne
#define GYRO_CAL 			//Saves time when starting the robot (don't have to recalulate gyro bias)
#define GYRO_CHANNEL 1

//--Encoder Configuration: encoder.h
//Dump whatever you don't need
#define ENABLE_ENCODER_1		//Left Side
#define ENABLE_ENCODER_2		//Right Side
//#define ENABLE_ENCODER_3
//#define ENABLE_ENCODER_4

//--ADC Configuration: adc.h
// Number of ADC channels to cycle through. This value must be a value 
// between one and fourteen or sixteen (fifteen is not an option). Make sure
// each of these analog channels is defined as an input in user_routines.c/
// User_Initialization() if you're using the EDU-RC.
#define NUM_ADC_CHANNELS 2										//Hey, you! Increase me for more analog inputs!

//--Camera Configuration: camera.h
// If your camera's serial port is attached to the the robot controller's
// programming serial port, uncomment the "#define CAMERA_SERIAL_PORT_1" 
// line. Otherwise, if your camera is attached to the TTL serial port, 
// uncomment the "#define CAMERA_SERIAL_PORT_2" line.
// #define CAMERA_SERIAL_PORT_1
#define CAMERA_SERIAL_PORT_2

int doPid(PosServo *p, long pos, int motor_direction);		//Supplied with position (PID)
int doVelPid(VelServo *p, long vel, int motor_direction);	//Supplied with velocity (Position/Velocity cascade loop)
int encoder_check(void);
int deadBandInput(int in, int deadband);
int lim(int input, int limit);
int abs(int val);
int turn(int degrees, int vel);
int checkDistance(long dist, int db);
void configTerminal(void);

