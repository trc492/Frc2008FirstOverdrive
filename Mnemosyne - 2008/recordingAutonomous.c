/*******************************************************************************
* FILE NAME: recordingAutonomous.c
*
* AUTHOR: Paul Malmsten, 2008
*
* DESCRIPTION:
*  This file contains code to handle implement an autonomous code that can be
*  recorded from the programming port while driving normally and introduced into 
*  the code to playback those actions.
*
*  void printRecordData(void) 
*	-Prints pwm outputs and spike states to the serial port
*  void playbackRecordData(int pwmL, int pwmR, int spk1, int spk2, int spk3, int spk4, int index)
*	-Plays back recorded data at the correct rate
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

#include "recordingAutonomous.h"

void printRecordData(void);
{
	static long index = 0;

	if(timerTicks & 1)
	{
		printf("playbackRecordData(%d, %d, %d, %d, %d, %d, %ld", Left.Vsetpoint, Right.Vsetpoint, relay1_fwd, relay1_rev, relay2_fwd, relay2_rev, relay3_fwd, relay3_rev, relay4_fwd, relay4_rev, index);
		index++;
	}
}

void playbackRecordData(int velL, int velR, int r1F, int r1R, int r2F, int r2R, int r3F, int r3R, int r4F, int r4R, long index)
{
	static long index = 0;

	if(timerTicks & 1)
	{
		Left.Vsetpoint  = velL;
		Right.Vsetpoint = velR
		relay1_fwd = r1F;
		relay1_rev = r1R;
		relay2_fwd = r2F;
		relay2_rev = r2R;
		relay3_fwd = r3F;
		relay3_rev = r3R;
		relay4_fwd = r4F;
		relay4_rev = r4R;
	}
}
