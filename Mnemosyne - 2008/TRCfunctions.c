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
//extern PosServo Elev;

extern long timerTicks;
/*
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
}*/
 
int doVelPid(VelServo *p, long encoder, int motor_direction)
{
	long encVel = 0;
	int velErr,Drive;
	
	//if(p->Mode)
	//	p->Vsetpoint = ((p->Psetpoint - encoder)*p->Kp)/p->Ko;		//Position Ctrl Loop; sent to Velocity Ctrl Loop

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

int turn()
{
	//We'll start with 180 degrees for now
	static int state = 0;
	static int targetDeg = -1800;
	static int errDeg;
	
	switch(state)
	{
		case 0:
			Left.Vsetpoint = (Right.Vsetpoint * 66) / 100;
			state++;
		break;
		
		//Smart, smooth turn
#ifdef SMART_TURN
		case 1:
			if(abs(Left.vel - Left.Vsetpoint) <= 2)
			{
				errDeg = Get_Gyro_Angle(); 				//Compensate for decceleration/acceleration time, in degrees of error
				state++;
			}
		break;

		case 2:
			if(Get_Gyro_Angle() <= targetDeg - errDeg)	//Subtract error
			{
				Left.Vsetpoint = Right.Vsetpoint;
				state++;
			}
		break;

		case 3:
			if(Get_Gyro_Angle() <= targetDeg)
				return 1;								//I'm done
		break;	
#endif


		//Stupid Turning
#ifndef SMART_TURN
		case 1:
			if(Get_Gyro_Angle() < -1500)
			{
				Left.Vsetpoint = Right.Vsetpoint;
				return 1;
			}
		break;
#endif
	

	}		
	return 0;				//Default response; not done turning
}

void Launcher_State_Machine(void)
{
	static char launcher_safety_change = 0;
	static char armedBefore = 0;						//We're not armed at the beginning, no need to disarm
	static char state2 = 0;
	static char prevState;
	static long prevTmr;
	static char state;
	
	static char launcher_safety = 1;					//Start with safety activated

	if(LAUNCHER_SW_SAFETY && !launcher_safety_change)	//Safety switch debounce
	{
		launcher_safety = !launcher_safety;
		launcher_safety_change = 1;
	} else if(!LAUNCHER_SW_SAFETY) {
		launcher_safety_change = 0;
	}

	if(launcher_safety)									//Force disarm if safed
		state = LAUNCHER_DISARM;

	switch(state)
	{
		case LAUNCHER_DISARM:							//Return the arm to a known, disarmed position (remember-no absolute sensors)
			switch(state2)
			{
				case 0:
					LAUNCHER = LAUNCHER_REV;
					
					if(LAUNCHER_LIM)
						state2++;

					if(!armedBefore)
						state2 = 4;	
				break;
				
				case 1:
					LAUNCHER = STOP;

					if(Launcher_Release())
						state2++;
				break;

				case 2:
					prevTmr = timerTicks;
					LAUNCHER = LAUNCHER_FWD;
					state2++;
				break;

				case 3:
					if((timerTicks - prevTmr) > LAUNCHER_ARM_TIME)
					{
						LAUNCHER = STOP;
						state2++;
					}
				break;

				case 4:
					LAUNCHER = STOP;
					if(!launcher_safety)
						state = LAUNCHER_ARMING;
				break;
			}	

			Pwm2_red = 0;
		break;
		
		case LAUNCHER_ARMING:							//Partially arm, then fully arm the launcher
			armedBefore = 1;
			switch(state2)
			{
				case 0:
					prevTmr = timerTicks;
					LAUNCHER = LAUNCHER_REV;
					state2++;
				break;
		
				case 1:
					if((timerTicks - prevTmr) > LAUNCHER_PREARM_TIME)
					{
						printf("Launcher Stopped");
						LAUNCHER = STOP;				//Launcher Victor must be set to BRAKE
					
						if(LAUNCHER_SW_FIRE)
						{
							LAUNCHER = LAUNCHER_REV;
							state2++;
						}
					}
				break;
					
				case 2:
					if(LAUNCHER_LIM)
					{
						prevTmr = timerTicks;
						LAUNCHER = LAUNCHER_FWD;
						state2++;
					}
				break;

				case 3:
					if((timerTicks - prevTmr) >= LAUNCHER_ARM_TIME)
					{
						LAUNCHER = STOP;
						state = LAUNCHER_ARMED;
					}
				break;
			}	
			
			if(timerTicks % 4 == 0)
				Pwm2_red = !Pwm2_red;					//Cool blink
		break;

		case LAUNCHER_ARMED:							//Watch for fire command, launch, pause, then reload
			switch(state2)
			{
				case 0:
					if(LAUNCHER_SW_FIRE)
					{
						if(Launcher_Release())
						{
							prevTmr = timerTicks;
							state2++;
						}
					}
				break;

				case 1:
					if((timerTicks - prevTmr) > 40)
						state = LAUNCHER_ARMING;
				break;
			}

			Pwm2_red = 1;
		break;
	}

	if(prevState != state)								//I changed states, lets reset state2
		state2 = 0;
	
	prevState = state;
	Pwm2_green = launcher_safety;						//Make the OI look cool

	if(timerTicks % 10 == 0)
		printf("State:%d State2:%d launchLim:%d\r\n", state, state2, LAUNCHER_LIM );
}

int Launcher_Release()			
{
	static char state = 0;
	static long prevTmr;

	switch(state)
	{
		case 0:
			if(LAUNCHER_LIM)							//Am I locked? Begin unlocking
			{
				LAUNCHER_RELEASE = LAUNCHER_RELEASE_REV;
				state++;
			}
		break;
		
		case 1:
			if(!LAUNCHER_LIM)							//Fired? Then begin locking
			{
				LAUNCHER_RELEASE = LAUNCHER_RELEASE_FWD;
				prevTmr = timerTicks;
				state++;
			}
		break;

		case 2:
			if(timerTicks - prevTmr > 40)				//Wait a sec to stop the motor
			{			
				LAUNCHER_RELEASE = STOP;
				return 1;
			}
		break;
	}

	return 0;
}

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

