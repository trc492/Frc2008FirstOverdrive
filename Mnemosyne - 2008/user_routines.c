/*******************************************************************************
* FILE NAME: user_routines.c <FRC VERSION>
*
* DESCRIPTION:
*  This file contains the default mappings of inputs  
*  (like switches, joysticks, and buttons) to outputs on the RC.  
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
*******************************************************************************/
       
#include <stdio.h>

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "serial_ports.h"
#include "pwm.h"
#include "camera.h"
#include "tracking.h"
#include "terminal.h"

#include "adc.h"
#include "encoder.h"
#include "gyro.h"

#include "TRCfunctions.h"


extern unsigned char aBreakerWasTripped;
extern unsigned long timerTicks;



/*** DEFINE USER VARIABLES AND INITIALIZE THEM HERE ***/
/* EXAMPLES: (see MPLAB C18 User's Guide, p.9 for all types)
unsigned char wheel_revolutions = 0; (can vary from 0 to 255)
unsigned int  delay_count = 7;       (can vary from 0 to 65,535)
int           angle_deviation = 142; (can vary from -32,768 to 32,767)
unsigned long very_big_counter = 0;  (can vary from 0 to 4,294,967,295)
*/
//		

//			     Mode        velLim,vel(ro),Kp,Kvp, Ki, Ko				(ro) = read only
VelServo Left  = {0, 0,0,0,0,0, 70,   0,    7, 130, 10, 100, 127}; 
VelServo Right = {0, 0,0,0,0,0, 70,   0,    7, 130, 10, 100, 127};

/*******************************************************************************
* FUNCTION NAME: Limit_Switch_Max
* PURPOSE:       Sets a PWM value to neutral (127) if it exceeds 127 and the
*                limit switch is on.
* CALLED FROM:   this file
* ARGUMENTS:     
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     switch_state   unsigned char    I    limit switch state
*     *input_value   pointer           O   points to PWM byte value to be limited
* RETURNS:       void
*******************************************************************************/
void Limit_Switch_Max(unsigned char switch_state, unsigned char *input_value)
{
  if (switch_state == CLOSED)
  { 
    if(*input_value > 127)
      *input_value = 127;
  }
}


/*******************************************************************************
* FUNCTION NAME: Limit_Switch_Min
* PURPOSE:       Sets a PWM value to neutral (127) if it's less than 127 and the
*                limit switch is on.
* CALLED FROM:   this file
* ARGUMENTS:     
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     switch_state   unsigned char    I    limit switch state
*     *input_value   pointer           O   points to PWM byte value to be limited
* RETURNS:       void
*******************************************************************************/
void Limit_Switch_Min(unsigned char switch_state, unsigned char *input_value)
{
  if (switch_state == CLOSED)
  { 
    if(*input_value < 127)
      *input_value = 127;
  }
}


/*******************************************************************************
* FUNCTION NAME: Limit_Mix
* PURPOSE:       Limits the mixed value for one joystick drive.
* CALLED FROM:   Default_Routine, this file
* ARGUMENTS:     
*     Argument             Type    IO   Description
*     --------             ----    --   -----------
*     intermediate_value    int    I    
* RETURNS:       unsigned char
*******************************************************************************/
unsigned char Limit_Mix (int intermediate_value)
{
  static int limited_value;
  
  if (intermediate_value < 2000)
  {
    limited_value = 2000;
  }
  else if (intermediate_value > 2254)
  {
    limited_value = 2254;
  }
  else
  {
    limited_value = intermediate_value;
  }
  return (unsigned char) (limited_value - 2000);
}


/*******************************************************************************
* FUNCTION NAME: User_Initialization
* PURPOSE:       This routine is called first (and only once) in the Main function.  
*                You may modify and add to this function.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void User_Initialization (void)
{
  //Set_Number_of_Analog_Channels(SIXTEEN_ANALOG);    /* DO NOT CHANGE! */ /*Dumped for adc.c support--read the instructions*/

/* FIRST: Set up the I/O pins you want to use as digital INPUTS. */
  digital_io_01 = digital_io_02 = digital_io_03 = digital_io_04 = INPUT;
  digital_io_05 = digital_io_06 = digital_io_07 = digital_io_08 = INPUT;
  digital_io_09 = digital_io_10 = digital_io_11 = digital_io_12 = INPUT;
  digital_io_13 = digital_io_14 = digital_io_15 = digital_io_16 = INPUT;
  digital_io_17 = INPUT;
  digital_io_18 = INPUT;  /* Used for pneumatic pressure switch. */
    /* 
     Note: digital_io_01 = digital_io_02 = ... digital_io_04 = INPUT; 
           is the same as the following:

           digital_io_01 = INPUT;
           digital_io_02 = INPUT;
           ...
           digital_io_04 = INPUT;
    */

/* SECOND: Set up the I/O pins you want to use as digital OUTPUTS. */
  //digital_io_17 = OUTPUT;    /* Example - Not used in Default Code. */

/* THIRD: Initialize the values on the digital outputs. */
  //rc_dig_out17 = 0;

/* FOURTH: Set your initial PWM values.  Neutral is 127. */
  pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127;
  pwm09 = pwm10 = pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127;

/* FIFTH: Set your PWM output types for PWM OUTPUTS 13-16.
  /*   Choose from these parameters for PWM 13-16 respectively:               */
  /*     IFI_PWM  - Standard IFI PWM output generated with Generate_Pwms(...) */
  /*     USER_CCP - User can use PWM pin as digital I/O or CCP pin.           */
//  Setup_PWM_Output_Type(IFI_PWM,IFI_PWM,IFI_PWM,IFI_PWM);

  // changed so PWM() can control PWM outputs 13 through 16
  Setup_PWM_Output_Type(USER_CCP,USER_CCP,USER_CCP,USER_CCP);

  /* 
     Example: The following would generate a 40KHz PWM with a 50% duty cycle on the CCP2 pin:

         CCP2CON = 0x3C;
         PR2 = 0xF9;
         CCPR2L = 0x7F;
         T2CON = 0;
         T2CONbits.TMR2ON = 1;

         Setup_PWM_Output_Type(USER_CCP,IFI_PWM,IFI_PWM,IFI_PWM);
  */

  /* Add any other initialization code here. */
  Initialize_ADC();
  Initialize_Encoders();
  Initialize_Gyro();

  // initialize and start 26.2ms timer for autonomous mode playback
  T1CON = 0x30; /* 1:8 Prescale */
  TMR1H = 0x7F; /* Pre-load TMR1 to overflow after 26.2ms */
  TMR1L = 0xFF;
  T1CONbits.TMR1ON = 1; /* Turn timer on */

  IPR1bits.TMR1IP = 0; /* Set Timer1 Interrupt to low priority */
  PIE1bits.TMR1IE = 1; /* Interrupt when Timer1 overflows */
  INTCONbits.GIEL = 1; /* Enable Global Low Priority Interrupts */

  // initialize the CCP PWM hardware
  Initialize_PWM();

  // initialize the serial ports
  Init_Serial_Port_One();
  Init_Serial_Port_Two();


#ifdef TERMINAL_SERIAL_PORT_1    
  stdout_serial_port = SERIAL_PORT_ONE;
#endif

#ifdef TERMINAL_SERIAL_PORT_2    
  stdout_serial_port = SERIAL_PORT_TWO;
#endif


  Putdata(&txdata);            /* DO NOT CHANGE! */

//  ***  IFI Code Starts Here***
//
//  Serial_Driver_Initialize();
//
//  printf("IFI 2006 User Processor Initialized ...\r");  /* Optional - Print initialization message. */

  User_Proc_Is_Ready();         /* DO NOT CHANGE! - last line of User_Initialization */

  printf("---TRC Robot Code 2008 Ver. 0.1.2 Initialized---\r\n");
}

/*******************************************************************************
* FUNCTION NAME: Process_Data_From_Master_uP
* PURPOSE:       Executes every 26.2ms when it gets new data from the master 
*                microprocessor.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Process_Data_From_Master_uP(void)
{
	static unsigned char count = 0;
	static unsigned char camera_menu_active = 0;
	static unsigned char tracking_menu_active = 0;
	unsigned char terminal_char;
	unsigned char returned_value;

		//Encoder Velocity Calculation; Oversampling
	static char samplesCount = 0;		//Samples out of 2
   
    	//PID Incremented Vars
	//static int leftPwm  = 127;
	//static int rightPwm = 127;

		//Pneumatic State Machines
	static char trigState = 0;
  	static char trigChange = 0;
	static char topState = 0;
  	static char topChange = 0;
	static char auxState = 1;		//Raise Dislodge Assembly at start of match
	static char auxChange = 0;
	static char aux2State = 0;		//Manual Dislodger Control
	static char aux2Change = 0;
	static char limState = 0;
	static char limChange = 0;

	static unsigned long dislodgeTmr = 0;

		//Compressor Mode
	static char firstRunFlag = 1;
	static char compMode = 0;			//Enable Compressor Mode

		//Joystick Vars
	int x, y, z;
	
		//Some gyro bias stuff here...looks intesting
	static unsigned int i = 0;
	static unsigned int j = 0;
	int temp_gyro_rate;
	long temp_gyro_angle;
	int temp_gyro_bias;

	static unsigned int h = 0;

	// don't move this unless you know what you're doing
		//I moved it to match the gyro code
	Getdata(&rxdata);
	
  //Set this to 1 to run the gyro bias calculation code. Otherwise, set it to 0 

#ifdef GYRO_CAL
	i++;
	j++; // this will rollover every ~1000 seconds
	
	if(j == 10)
	{
		printf("\rCalculating Gyro Bias...");
	}
	
	if(j == 60)
	{
		// start a gyro bias calculation
	//	Start_Gyro_Bias_Calc();
	}
	
	if(j == 300)
	{
		// terminate the gyro bias calculation
	//	Stop_Gyro_Bias_Calc();
	
		// reset the gyro heading angle
		//Reset_Gyro_Angle();
	
	//	printf("Done\r");
	}
	
	
	if(i >= 30 && j >= 300)
	{
		temp_gyro_bias = Get_Gyro_Bias();
		temp_gyro_rate = Get_Gyro_Rate();
		temp_gyro_angle = Get_Gyro_Angle();
		
		//printf(" Factor:%d\r\n", GYRO_CAL_FACTOR);	
	
		printf(" Gyro Bias=%d\r\n", temp_gyro_bias);
		printf(" Gyro Rate=%d\r\n", temp_gyro_rate);
		printf("Gyro Angle=%d\r\n\r\n", (int)temp_gyro_angle);		
	
		i = 0;
	}

	//Set_Gyro_Bias(GYRO_BIAS);
#endif

#ifndef GYRO_CAL
	Set_Gyro_Bias(GYRO_BIAS);
#endif

	// send diagnostic information to the terminal
		//comment me to stop spamming the serial port
	//Tracking_Info_Terminal();

	// This function is responsable for camera initialization 
	// and camera serial data interpretation. Once the camera
	// is initialized and starts sending tracking data, this 
	// function will continuously update the global T_Packet_Data 
	// structure with the received tracking information.
//	Camera_Handler();

	// This function reads data placed in the T_Packet_Data
	// structure by the Camera_Handler() function and if new
	// tracking data is available, attempts to keep the center
	// of the tracked object in the center of the camera's
	// image using two servos that drive a pan/tilt platform.
	// If the camera doesn't have the object within it's field 
	// of view, this function will execute a search algorithm 
	// in an attempt to find the object.
//	Servo_Track();

	// Turn on the "Switch 3" LED on the operator interface if
	// the camera is pointed at the green light target. The
	// Switch3_LED variable is also used by the Default_Routine()
	// function below, so disable it inside Default_Routine()
	// if you want to keep this functionality. 
/*	if(Get_Tracking_State() == CAMERA_ON_TARGET)
	{
		Switch3_LED = 1;
	}
	else
	{
		Switch3_LED = 0;
	}*/

	// IFI's default routine is commented out for safety reasons
	// and because it also tries to use PWM outputs one and two,
	// which conflicts with the default assignment for the pan
	// and tilt servos.
//  Default_Routine();

///
///----------------------------------Mappings------------------------------
/// This is some of my (Paul) code, put here because Default_Routine() is commented out

  
  configTerminal(); //Interactive Terminal to update drive system configuration

  x = deadBandInput(RAW_X + YAW_OFFSET, YAW_DB);			//Centered at 0
  y = deadBandInput(RAW_Y + SPEED_OFFSET, SPEED_DB);		//Centered at 0

  x = (x*80)/100;											//Steering Potentcy

  z = deadBandInput(RAW_FORK + FORK_OFFSET, FORK_DB);	//Victor must be set to BRAKE
  FORK = 127 + (-z *7)/10 ;
				

  //Encoder Velocity Calculation; reduced sampling
  if(++samplesCount == 2)										
  {
		Left.Vsetpoint  = (Left.velLim*(y - x))/127; 		//Velocity setting: percentage from inputs*velocity limit -> setpoint
		Right.Vsetpoint = (Right.velLim*(y + x))/127; 	

#ifdef DRIVE_POSITION_MODE
		Left.Psetpoint  += Left.Vsetpoint;					//Uses velocity to increment position in position mode
  		Right.Psetpoint += Right.Vsetpoint;
#endif

#ifndef DRIVE_DEBUG
		L_CIM_1 = L_CIM_2 = doVelPid(&Left, L_ENC_COUNT, L_MOTOR_DIR);
#endif
        R_CIM_1 = R_CIM_2 = doVelPid(&Right, R_ENC_COUNT, R_MOTOR_DIR);

		samplesCount = 0;		
  }

  
   //pwm10 = pwm11 = doPid(&Elev, Get_ADC_Result(2), ELEV_MOTOR_DIR);

  ///Relays
  COMP_FWD = !COMP_PRESSURE;  /* Power pump only if pressure switch is off. */
  COMP_REV = 0;

  Launcher_State_Machine();   //Handles the arming, firing, and disarming of the dislodger

  Pwm1_red   = !Pwm1_green;

  if(firstRunFlag)
  {
	Pwm1_green = 0;
  	if(p3_sw_top)			  //Compressor Mode: disables motors if aux1 switch on port 3 joystick is held on reset
	{
		compMode = 1;
		printf("Compressor Mode -- Motors Disabled, Charging Tanks");
		Pwm1_green = 1;
    }

	firstRunFlag = 0;
  }

  
  //Compressor Mode: disable motors
  if(compMode)
  	pwm01 = pwm10 = pwm11 = R_CIM_1 = R_CIM_2 = L_CIM_1 = L_CIM_2 = 127;

///
///------------------------------------End Mappings-----------------------
///	

	// IFI's software based PWM pulse generator for PMW ouputs
	// 13 through 16. This has been replaced with a hardware-
	// based solution, PWM(), below.
//	Generate_Pwms(pwm13,pwm14,pwm15,pwm16);

	// see pwm_readme.txt for information about PWM();
	PWM(pwm13,pwm14,pwm15,pwm16);

	// don't move this unless you know what you're doing
	Putdata(&txdata);
}

/*******************************************************************************
* FUNCTION NAME: Default_Routine
* PURPOSE:       Performs the default mappings of inputs to outputs for the
*                Robot Controller.
* CALLED FROM:   this file, Process_Data_From_Master_uP routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Default_Routine(void)
{
  int x, y;
  
 /*---------- Analog Inputs (Joysticks) to PWM Outputs-----------------------
  *--------------------------------------------------------------------------
  *   This maps the joystick axes to specific PWM outputs.
  */
  
  	/* We don't really need this
  pwm01 = p1_y;
  pwm02 = p2_y;   
  pwm03 = p3_y;   
  pwm04 = p4_y; 
  pwm05 = p1_x;  
  pwm06 = p2_x;   
  pwm07 = p3_x;   
  pwm08 = p4_x;   
  pwm09 = p1_wheel;
  pwm10 = p2_wheel;   
  pwm11 = p3_wheel;   
  pwm12 = p4_wheel; */
 
  
 /*---------- 1 Joystick Drive ----------------------------------------------
  *--------------------------------------------------------------------------
  *  This code mixes the Y and X axis on Port 1 to allow one joystick drive. 
  *  Joystick forward  = Robot forward
  *  Joystick backward = Robot backward
  *  Joystick right    = Robot rotates right
  *  Joystick left     = Robot rotates left
  *  Connect the right drive motors to PWM13 and/or PWM14 on the RC.
  *  Connect the left  drive motors to PWM15 and/or PWM16 on the RC.
  */  

	/*Let's try it my way
  p1_x = 255 - p1_y;
  p1_y = 255 - pwm05;

  pwm13 = pwm14 = Limit_Mix(2000 + p1_y + p1_x - 127); 
  pwm15 = pwm16 = Limit_Mix(2000 + p1_y - p1_x + 127);
	*/
  
  
  
 /*---------- Buttons to Relays----------------------------------------------
  *--------------------------------------------------------------------------
  *  This default code maps the joystick buttons to specific relay outputs.  
  *  Relays 1 and 2 use limit switches to stop the movement in one direction.
  *  The & used below is the C symbol for AND                                
  */

 	/* Don't need this...
  relay1_fwd = p1_sw_trig & rc_dig_in01;  /* FWD only if switch1 is not closed. 
  relay1_rev = p1_sw_top  & rc_dig_in02;  /* REV only if switch2 is not closed. 
  relay2_fwd = p2_sw_trig & rc_dig_in03;  /* FWD only if switch3 is not closed. 
  relay2_rev = p2_sw_top  & rc_dig_in04;  /* REV only if switch4 is not closed. 
  relay3_fwd = p3_sw_trig;
  relay3_rev = p3_sw_top;
  relay4_fwd = p4_sw_trig;
  relay4_rev = p4_sw_top;
  relay5_fwd = p1_sw_aux1;
  relay5_rev = p1_sw_aux2;
  relay6_fwd = p3_sw_aux1;
  relay6_rev = p3_sw_aux2;
  relay7_fwd = p4_sw_aux1;
  relay7_rev = p4_sw_aux2; */
  
  
  /*---------- PWM outputs Limited by Limit Switches  ------------------------*/
  
  	/* Don't need this
  Limit_Switch_Max(rc_dig_in05, &pwm03);
  Limit_Switch_Min(rc_dig_in06, &pwm03);
  Limit_Switch_Max(rc_dig_in07, &pwm04);
  Limit_Switch_Min(rc_dig_in08, &pwm04);
  Limit_Switch_Max(rc_dig_in09, &pwm09);
  Limit_Switch_Min(rc_dig_in10, &pwm09);
  Limit_Switch_Max(rc_dig_in11, &pwm10);
  Limit_Switch_Min(rc_dig_in12, &pwm10);
  Limit_Switch_Max(rc_dig_in13, &pwm11);
  Limit_Switch_Min(rc_dig_in14, &pwm11);
  Limit_Switch_Max(rc_dig_in15, &pwm12);
  Limit_Switch_Min(rc_dig_in16, &pwm12);
	*/
  
  
 /*---------- ROBOT FEEDBACK LEDs------------------------------------------------
  *------------------------------------------------------------------------------
  *   This section drives the "ROBOT FEEDBACK" lights on the Operator Interface.
  *   The lights are green for joystick forward and red for joystick reverse.
  *   Both red and green are on when the joystick is centered.  Use the
  *   trim tabs on the joystick to adjust the center.     
  *   These may be changed for any use that the user desires.                       
  */	
  
  if (user_display_mode == 0) /* User Mode is Off */
    
  { /* Check position of Port 1 Joystick */
    if (p1_y >= 0 && p1_y <= 56)
    {                     /* Joystick is in full reverse position */
      Pwm1_green  = 0;    /* Turn PWM1 green LED - OFF */
      Pwm1_red  = 1;      /* Turn PWM1 red LED   - ON  */
    }
    else if (p1_y >= 125 && p1_y <= 129)
    {                     /* Joystick is in neutral position */
      Pwm1_green  = 1;    /* Turn PWM1 green LED - ON */
      Pwm1_red  = 1;      /* Turn PWM1 red LED   - ON */
    }
    else if (p1_y >= 216 && p1_y <= 255)
    {                     /* Joystick is in full forward position*/
      Pwm1_green  = 1;    /* Turn PWM1 green LED - ON  */
      Pwm1_red  = 0;      /* Turn PWM1 red LED   - OFF */
    }
    else
    {                     /* In either forward or reverse position */
      Pwm1_green  = 0;    /* Turn PWM1 green LED - OFF */
      Pwm1_red  = 0;      /* Turn PWM1 red LED   - OFF */
    }  /*END Check position of Port 1 Joystick
    
    /* Check position of Port 2 Y Joystick 
           (or Port 1 X in Single Joystick Drive Mode) */
    if (p2_y >= 0 && p2_y <= 56)
    {                     /* Joystick is in full reverse position */
      Pwm2_green  = 0;    /* Turn pwm2 green LED - OFF */
      Pwm2_red  = 1;      /* Turn pwm2 red LED   - ON  */
    }
    else if (p2_y >= 125 && p2_y <= 129)
    {                     /* Joystick is in neutral position */
      Pwm2_green  = 1;    /* Turn PWM2 green LED - ON */
      Pwm2_red  = 1;      /* Turn PWM2 red LED   - ON */
    }
    else if (p2_y >= 216 && p2_y <= 255)
    {                     /* Joystick is in full forward position */
      Pwm2_green  = 1;    /* Turn PWM2 green LED - ON  */
      Pwm2_red  = 0;      /* Turn PWM2 red LED   - OFF */
    }
    else
    {                     /* In either forward or reverse position */
      Pwm2_green  = 0;    /* Turn PWM2 green LED - OFF */
      Pwm2_red  = 0;      /* Turn PWM2 red LED   - OFF */
    }  /* END Check position of Port 2 Joystick */
    
    /* This drives the Relay 1 and Relay 2 "Robot Feedback" lights on the OI. */
    Relay1_green = relay1_fwd;    /* LED is ON when Relay 1 is FWD */
    Relay1_red = relay1_rev;      /* LED is ON when Relay 1 is REV */
    Relay2_green = relay2_fwd;    /* LED is ON when Relay 2 is FWD */
    Relay2_red = relay2_rev;      /* LED is ON when Relay 2 is REV */

    Switch1_LED = !(int)rc_dig_in01;
    Switch2_LED = !(int)rc_dig_in02;
    //Switch3_LED = !(int)rc_dig_in03;
    
  } /* (user_display_mode = 0) (User Mode is Off) */
  
  else  /* User Mode is On - displays data in OI 4-digit display*/
  {
    User_Mode_byte = backup_voltage*10; /* so that decimal doesn't get truncated. */
  }   
  
} /* END Default_Routine(); */



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
