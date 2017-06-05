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
extern long timerTicks;

//May need some serious testing before use
int doPid(PosServo *p, long pos, int motor_direction)
{
	int Error, Drive;

	if(p->Psetpoint > p->posMaxLim)
		p->Psetpoint = p->posMaxLim;
	if(p->Psetpoint < p->posMinLim)
		p->Psetpoint = p->posMinLim;
	
	Error = p->Psetpoint - pos;		//Compare against the requested velocity
	
	Drive = (p->Kp*Error + p->Kd*(Error - p->PrevErr) + p->Ki*p->Ierror)/p->Ko;  //Yay! PID! Note: PrevErr is bigger than Error when approaching setpoint; Kd is okay being added
	p->PrevErr = Error;				//Save this error for next time

    if (Drive >= 127)				//Accumulate Integral error only if not saturated; Remember this is 0 based - 127 is added later
           Drive = 127;				//Forward Limit
    else if (Drive <= -127)
        Drive = -127;				//Reverse Limit
    else
		p->Ierror += Error;			//Accumulate Error

	return (p->Drive = (Drive*motor_direction)+127); //Adjust direction
}

//Served us well, but perhaps it could use a little review
int doVelPid(VelServo *p, long encoder, int motor_direction)
{
	long encVel = 0;
	int velErr,Drive;
	
	if(p->Mode)
		p->Vsetpoint = ((p->Psetpoint - encoder)*p->Kp)/p->Ko;		//Position Ctrl Loop; sent to Velocity Ctrl Loop

	if(p->Vsetpoint > p->velLim)									//Enforce Veloctiy Limit
		p->Vsetpoint = p->velLim;
	else if (p->Vsetpoint < -p->velLim)
		p->Vsetpoint = -p->velLim;
	
	p->vel = (encoder - p->PrevEnc);								//Update velocity property

	velErr = p->Vsetpoint - p->vel;									//Velocity Error
	Drive = ((velErr + (p->Kvi*p->Ierr)/p->Ko)*p->Kvp)/p->Ko;		//Velocity Ctrl Loop: ((velErr + (Iterm/Ko))*Kvp)/Ko

	p->PrevEnc = encoder;
	
	if(Drive > 127)													//Limit drive output
		Drive = 127;
	else if (Drive < -127)
		Drive = -127;
	else
		p->Ierr += velErr;

	if(abs(p->Psetpoint - encoder) > 10)							//Report back when we're close enough to the requested position
	{
		p->PosOnTarget = 0;
	} else {
		p->PosOnTarget = 1;
	}

#ifdef DRIVE_DEBUG
	printf("PSet:%ld VSet:%d velErr:%d Enc:%ld Drive:%d Perr:%ld\r\n", p->Psetpoint, p->Vsetpoint, velErr, encoder, Drive*motor_direction, (p->Psetpoint - encoder));
#endif

	return (p->Drive = (Drive*motor_direction)+127);
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

//Needs to be rewritten; I want _exact_ turns
int turn()
{
	static int state = 0;
	static int targetDeg = -1800;
	static int errDeg;
	
	switch(state)
	{
		case 0:
			Left.Vsetpoint = (Right.Vsetpoint * 66) / 100;
			state++;
		break;
		
		
		#ifndef SMART_TURN
				case 1:
					if(Get_Gyro_Angle() < -1500)		//Arbitrary value; seems to get us to about 180 degrees on average
					{
						Left.Vsetpoint = Right.Vsetpoint;
						return 1;
					}
				break;
		#endif
	
	}		
	return 0;				//Default response; not done turning
}


//Needs to be rewritten; could be quite a bit prettier
void configTerminal(void)
{
	unsigned char character;
	static char debugMode = 0;
	static char debugChar = 'c';

	while(Serial_Port_One_Byte_Count() > 0)
	{
		character = Read_Serial_Port_One();
		switch(character)
		{
			case 'p':
				++Left.Kvp;
				++Right.Kvp;
				printf("Kvp:%d Kvi:%d Ko:%d\r\n", Left.Kvp, Left.Kvi, Left.Ko);
			break;

			case 'o':
				--Left.Kvp;
				--Right.Kvp;
				printf("Kvp:%d Kvi:%d Ko:%d\r\n", Left.Kvp, Left.Kvi, Left.Ko);
			break;

			case 'i':
				++Left.Kvi;
				++Right.Kvi;
				Left.Ierr = 0;
				Right.Ierr = 0;
				printf("Kvp:%d Kvi:%d Ko:%d\r\n", Left.Kvp, Left.Kvi, Left.Ko);
			break;

			case 'u':
				--Left.Kvi;
				--Right.Kvi;
				Left.Ierr = 0;
				Right.Ierr = 0;
				printf("Kvp:%d Kvi:%d Ko:%d\r\n", Left.Kvp, Left.Kvi, Left.Ko);
			break;

			case 'l':
				printf("Kvp:%d Kvi:%d Ko:%d\r\n", Left.Kvp, Left.Kvi, Left.Ko);
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
				printf("p1x:%d p1y:%d p2x:%d p2y:%d p3x:%d p3y:%d\r\n", p1_x, p1_y, p2_x, p2_y, p3_x, p3_y);
			break;

			case 'q':
				printf("trig:%d top:%d aux1:%d aux2:%d\r\n", p3_sw_trig, p3_sw_top, p3_sw_aux1, p3_sw_aux2);
			break;

			case 'w':
				printf("Relay2Fwd:%d Relay2Rev:%d Relay3Fwd:%d Relay3Rev:%d p3_sw_trig:%d p3_sw_top:%d\r\n", relay2_fwd, relay2_rev, relay3_fwd, relay3_rev, p3_sw_trig, p3_sw_top);
			break;

			default:
				printf("Nothing to print\r\n");
				debugMode = 0;
			break;
		}
	}

}			

