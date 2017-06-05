/*******************************************************************************
* FILE NAME: TRCfunctions.c
*
* AUTHOR: Paul Malmsten, 2008
*
* DESCRIPTION:
*  This file contains all of the extra functions the Titan Robotics Club has
*  added. Feel free to mess with it.
*
*******************************************************************************/
#include <stdio.h>
#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "serial_ports.h"
#include "encoder.h"
#include "adc.h"
#include "gyro.h"

#include "TRCfunctions.h"

extern VelServo Left, Right;

int doVelPid(VelServo *p, long encoder, int motor_direction)
{
	long vel;
	int velErr,Drive,Kp;
	
	/*if(p->mode == 1)																//Position mode?
		p->Vsetpoint = ((p->Psetpoint - encoder)*p->Kpp)/p->Ko;								//Velocity based on position error
	*/
	p->Vsetpoint = lim(p->Vsetpoint, p->velLim);									//Velocity Limit
	
	vel = (encoder - p->prevEnc);
	velErr = p->Vsetpoint - vel;													//Velocity Error
	Kp = (abs(vel) > p->velThresh) ? p->Kvphs:p->Kvpls;									//Are we going slowly or quickly?
	Drive = (velErr*Kp + (velErr - p->prevErr)*p->Kd + p->Ierr*p->Kvi)/p->Ko;		//PID Algorithm
																						//Remember: no p-> in front of Kp	

	Drive = lim(Drive, p->tqLim);													//Torgue Limit
	Drive += (vel * p->bEMF)/p->Ko;													//bEMF Compensation
	Drive = lim(Drive, 127);														//Saturation Limit

	if(abs(Drive) < 127)															//Saturation check for integration
		p->Ierr += velErr;

#ifdef DRIVE_DEBUG
	printf("PSet:%ld VSet:%d velErr:%d prevErr:%d Kd:%d Enc:%ld Drive:%d Perr:%ld\r\n", p->Psetpoint, p->Vsetpoint, velErr,p->prevErr, ((velErr - p->prevErr)*p->Kd)/p->Ko, encoder, Drive*motor_direction, (p->Psetpoint - encoder));
#endif

	p->prevErr = velErr;
	p->prevEnc = encoder;
	return ((Drive*motor_direction)+127);								//Compensate for direction 
}

int deadBandInput(int in, int deadBand)
{
	if(abs(in - 127) <= deadBand)
		return 0;
	else
		if((in - 127) > 125)
			return -(in - 127); //Joysticks want to flip direction when > 125
		return in - 127;
}

int abs(int val)
{
	return (val >= 0)?val:-val;
}

int lim(int input, int limit)		//Limits input value to a range inside + and - limit
{
	if(input > limit)
		return limit;
	else if(input < -limit)
		return -limit;
	else
		return input;
}

int turn(int degrees, int vel)		
{
	int error;
	error = degrees - Get_Gyro_Angle();

	error = lim(error, 20);

	Left.Vsetpoint  = vel + error;
	Right.Vsetpoint = vel - error;

	if(abs(error) < 10)
		return 1;
	else 
		return 0;
}

int checkDistance(long dist, int db)
{
	if((Get_Encoder_1_Count() + Get_Encoder_2_Count())/2 - dist > db)
		return 1;
	else
		return 0;
}

void configTerminal(void)
{
	unsigned char character;
	static char debugMode = 0;
	static char debugChar = 'c';
	static char delayCount = 0;
	static char KpSpeed = 0;
	static int *KpL = &Left.Kvpls;
	static int *KpR = &Right.Kvpls;

	while(Serial_Port_One_Byte_Count() > 0)
	{
		character = Read_Serial_Port_One();
		switch(character)
		{
			case 'p':
				(*KpL)++;
				(*KpR)++;
				printf("Kvp:%d Kd:%d Ko:%d\r\n", *KpL, Left.Kd, Left.Ko);
			break;

			case 'o':
				(*KpL)--;
				(*KpR)--;
				printf("Kvp:%d Kd:%d Ko:%d\r\n", *KpL, Left.Kd, Left.Ko);
			break;

			case 'd':
				Left.Kd  += 5;
				Right.Kd += 5;
				printf("Kvp:%d Kd:%d Ko:%d\r\n", *KpL, Left.Kd, Left.Ko);
			break;

			case 's':
				Left.Kd  -= 5;
				Right.Kd -= 5;
				printf("Kvp:%d Kd:%d Ko:%d\r\n", *KpL, Left.Kd, Left.Ko);
			break;

			case 'l':
				printf("Kvp:%d Kd:%d Ko:%d\r\n", *KpL, Left.Kd, Left.Ko);
			break;

			case 'q':
				KpSpeed = !KpSpeed;
				if(KpSpeed)
				{
					KpL = &Left.Kvphs;
					KpR = &Right.Kvphs;
				} else {
					KpL = &Left.Kvpls;
					KpR = &Right.Kvpls;
				}
				printf("Kvp:%d Kd:%d Ko:%d\r\n", *KpL, Left.Kd, Left.Ko);
			break;
			
			case 'c':
				if(debugMode)
					debugMode = 0;
				else
					debugMode = 1;
			break;
		}

		if(character != '\r\n' && character != 'c')
			debugChar = character;
	}

	if(debugMode)
	{
		switch(debugChar)
		{	
			case 'm':
				printf("Enc1:%ld Enc2:%ld LVSet:%d RVSet:%d LDr:%d RDr:%d\r\n", Get_Encoder_1_Count(), Get_Encoder_2_Count(), Left.Vsetpoint, Right.Vsetpoint, pwm15, pwm13);
			break;
		
			case 'j':
				printf("p1x:%d p1y:%d p2x:%d p2y:%d p3x%d p3y%d\r\n", p1_x, p1_y, p2_x, p2_y, p3_x, p3_y);
			break;

			case 'q':
				printf("trig:%d top:%d aux1:%d aux2:%d\r\n", p3_sw_trig, p3_sw_top, p3_sw_aux1, p3_sw_aux2);
			break;

			case 'w':
				printf("Relay2Fwd:%d Relay2Rev:%d Relay3Fwd:%d Relay3Rev:%d p3_sw_trig:%d p3_sw_top:%d\r\n", relay2_fwd, relay2_rev, relay3_fwd, relay3_rev, p3_sw_trig, p3_sw_top);
			break;

			case 'g':
				if(++delayCount == 10)
				{
					printf("GBias:%d GRate:%d GAngle:%ld\r\n", Get_Gyro_Bias(), Get_Gyro_Rate(), Get_Gyro_Angle());
					delayCount = 0;
				}
			break;

			default:
				printf("Nothing to print\r\n");
				debugMode = 0;
			break;
		}
	}
}
