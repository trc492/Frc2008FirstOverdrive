#if 0
/// Copyright (c) 2008 Titan Robotics Club. All rights are reserved.
///
/// <module name="project.c" />
///
/// <summary>
///     This module contains the main project code.
/// </summary>
///
/// <remarks>
///     Author: Paul Malmsten (pmalmsten@gmail.com)
///             Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "serial_ports.h"

//
// Global data.
//
DWORD g_dwProjFlags = 0;

// +=====================================+
// |  Customizable project global data.  |
// +=====================================+

// Kp, Kv, Ki, Ko
PIDPARAM    g_DrivePid = {7, 130, 10, 100};
PIDPARAM    g_TurnPid = {1, 1, 1, 1};
SERVO       g_LeftMotor = {NULL};
SERVO       g_RightMotor = {NULL};
SERVO       g_TurnServo = {NULL};
DRIVE       g_Drive = {&g_LeftMotor, &g_RightMotor, &g_TurnServo,
                       DRIVE_ACCEL_RATE, DRIVE_DECEL_RATE};

/// <summary>
///     This function does project specific initialization.
/// </summary>
///
/// <returns> None. </returns>

void
ProjInitialize(
    void
    )
{
    static char szOrg[] = ORGANIZATION;
    static char szProject[] = PROJECT;
    //
    // Initialize the serial ports first so that the debug console is on-line.
    //
    Serial1Initialize(BAUD_115200,
                      ENABLE_SERIAL_RX | ENABLE_SERIAL_TX |
                      ENABLE_SERIAL_STDOUT);
    Serial2Initialize(BAUD_115200,
                      ENABLE_SERIAL_RX | ENABLE_SERIAL_TX);
#ifdef _USE_CONSOLE_
    ProjCmdConsoleInit(SERIAL_PORT_ONE);
#endif

    //
    // +==================================+
    // |  Customizable code starts here.  |
    // +==================================+
    //
    
    // Initialize all RC I/O.
    // Set all digital I/O channels as input (no output channels).
    // Set PWM channels 13-16 to user_ccp.
    //
    RCIOInitialize(0);
    
    // Initialize PWM and relays.
    PWMInitialize();
    Setup_PWM_Output_Type(USER_CCP,USER_CCP,USER_CCP,USER_CCP);
    relay4_fwd = 0;     //???MTS: What is relay 4?
    relay4_rev = 1;
    
    //
    // Initialize RC devices.
    //
    
    // A/D converters:
    // Sampling rate = 200 Hz, 4x oversampling.
    // Initialize ADC channels 1 & 2.
    ADCInitialize(ADC_SAMPLE_RATE_200HZ, 4, ADC_MASK_1 | ADC_MASK_2);
    
    // Initialize encoders 1 & 2.
    // Set encoder velocity interval at 2 system ticks.
    EncoderInitialize(ENCODER_MASK_1 | ENCODER_MASK_2, ENCODER_MASK_2, 2);
    
    // Initialize gyro.
    // Note: Gyro depends on ADC, so ADC init must come first.
    GyroInitialize(GYRO_ADC_CHANNEL,
                   GYRO_SENSITIVITY,
                   GYRO_SCALE_FACTOR,
                   GYRO_NOISE_MARGIN);

    //
    // Initialize OI devices.
    //
    
    // Initialize joystick 1: Y (Drive forward/backward).
    // Initialize joystick 2: X (Drive left/right).
    // Initialize joystick 3: Y (Elevator up/down).
    InputInitialize(PORTF_ENABLE_Y, PORTF_ENABLE_X, PORTF_ENABLE_Y, 0);
    
    // Initialize the motors.
    ServoInitialize(&g_LeftMotor,
                    &g_DrivePid,
                    ENCODER_LEFT,
                    DRIVE_VEL_LIMIT,
                    L_MOTOR_DIR);
    ServoInitialize(&g_RightMotor,
                    &g_DrivePid,
                    ENCODER_RIGHT,
                    DRIVE_VEL_LIMIT,
                    R_MOTOR_DIR);
    ServoInitialize(&g_TurnServo,
                    &g_TurnPid,
                    0,                  //not used.
                    TURN_VEL_LIMIT,
                    1);

    //
    // +================================+
    // |  Customizable code ends here.  |
    // +================================+
    //

    //
    // Update OI with the configuration.
    // Note: DO NOT CHANGE!
    //
    Putdata(&txdata);

    //
    // Inform Master uP we are ready.
    // Note: DO NOT CHANGE!
    //
    User_Proc_Is_Ready();
    printf("\r\n%s (%d) %s: Version %d.%d.%d\r\n",
           szProject, YEAR, szOrg,
           VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

    return;
}       //ProjInitialize

/// <summary>
///     This function performs project data processing. This is where the
///     main work is done.
/// </summary>
///
/// <returns> None. </returns>

void
ProjProcessData(
    void
    )
{
    //
    // +==================================+
    // |  Customizable code starts here.  |
    // +==================================+
    //
    
    //Encoder Velocity Calculation; Oversampling
    static char samplesCount = 0;       //Samples out of 2
    static char firstRunFlag = 1;
    static char compMode = 0;           //Enable Compressor Mode
    int x, y, z;                        //Joystick vars

    x = JOYSTICK_X;
    y = JOYSTICK_Y;
    z = JOYSTICK_Z;
    x = (x*120)/100;    //??? MTS: should check against limit.
    
    // Encoder velocity calculation: oversampling.
    if (++samplesCount == 2)
    {
        //Velocity setting: percentage from inputs*velocity limit -> setpoint
        g_LeftMotor.iVelSp  = (g_LeftMotor.iVelLimit*(y - x))/127;
        g_RightMotor.iVelSp = (g_RightMotor.iVelLimit*(y + x))/127;

        L_MOTOR_1 = L_MOTOR_2 = ServoPidDrive(&g_LeftMotor,
                                              0,
                                              EncoderGetVelocity(ENCODER_LEFT),
                                              FALSE);
        R_MOTOR_1 = R_MOTOR_2 = ServoPidDrive(&g_RightMotor,
                                              0,
                                              EncoderGetVelocity(ENCODER_RIGHT),
                                              FALSE);
        samplesCount = 0;
    }
    
    Dislodger_Fwd = !PneumaticSwitch;
    Dislodger_Rev = 0;
    
    // Pneumatic state machine.
    if (firstRunFlag)
    {
        //Compressor Mode: disables motors if aux1 switch on port 3 joystick
        //is held on reset
        if (p3_sw_aux1)
        {
             compMode = 1;
             printf("Compressor Mode -- Motors Disabled, Charging Tanks");
             Pwm1_red = 1;
        }
        firstRunFlag = 0;
    }
    
    //Compressor Mode: disable motors
    if (compMode)
    {
        pwm01 = pwm10 = pwm11 =
        R_MOTOR_1 = R_MOTOR_2 = L_MOTOR_1 = L_MOTOR_2 = 127;
    }
    
    //
    // +================================+
    // |  Customizable code ends here.  |
    // +================================+
    //
    
    PWMSetValues(pwm13,pwm14,pwm15,pwm16);
    Putdata(&txdata);
    
    return;
}       //ProjProcessData

/// <summary>
///     This function performs the autonomous operation.
/// </summary>
///
/// <returns> None. </returns>

void
ProjAutonomousCode(
    void
    )
{
    //
    // +==================================+
    // |  Customizable code starts here.  |
    // +==================================+
    //
    static char state = 0;
    static unsigned long prevTmr = 0;

    Dislodger_Fwd = !PneumaticSwitch; //Power pump only if pressure switch is off.
    Dislodger_Rev = 0;

    if (rc_dig_in16)    //???MTS: what is in16?
    {
        relay4_fwd = 1;
        relay4_rev = 0;

        //Left.velLim = Right.velLim = 25;		//Slow us down to dislodge safely
    }

    switch (state)
    {
        case 0:
            //printf("State1\r\n");
            if (g_dwCurrentTicks > 160)
            {
                g_LeftMotor.iVelSp = g_RightMotor.iVelSp = 35;
                if ((EncoderGetCount(ENCODER_LEFT) +
                     EncoderGetCount(ENCODER_RIGHT))/2 > 2900)
                {
                    state++;
                }
            }
            break;

        case 1:
            //printf("State2\r\n");
            g_LeftMotor.iVelSp = 14;
            g_RightMotor.iVelSp = 26;
            if (GyroGetAngle() < -1050)
            {
                state++;
            }
            break;

        case 2:
            g_LeftMotor.iVelSp = g_RightMotor.iVelSp = 50;
            break;
    } 
    /*
    switch(state)
    {
        case 0:
            Left.Vsetpoint = Right.Vsetpoint = 20;
            if ((EncoderGetCount(ENCODER_LEFT) +
                 EncoderGetCount(ENCODER_RIGHT))/2 > 226)
            {
                state++;
            }
            break;

        case 1:
            Left.Vsetpoint = Right.Vsetpoint = 0;
            break;
    }*/

    if (g_dwCurrentTicks - prevTmr > 1)	//52.4ms Timer aea
    {
        prevTmr = g_dwCurrentTicks;
        L_MOTOR_1 = L_MOTOR_2 = ServoPidDrive(&g_LeftMotor,
                                              0,
                                              EncoderGetVelocity(ENCODER_LEFT),
                                              FALSE);
        R_MOTOR_1 = R_MOTOR_2 = ServoPidDrive(&g_RightMotor,
                                              0,
                                              EncoderGetVelocity(ENCODER_RIGHT),
                                              FALSE);
    }
    
    //
    // +================================+
    // |  Customizable code ends here.  |
    // +================================+
    //
    
    PWMSetValues(pwm13,pwm14,pwm15,pwm16);
    Putdata(&txdata);

    return;
}       //ProjAutonomousCode

/// <summary>
///     This function processes local I/O data that requires real-time
///     attention.
/// </summary>
///
/// <returns> None. </returns>

void
ProjProcessLocalIO(
    void
    )
{
    //
    // +==================================+
    // |  Customizable code starts here.  |
    // +==================================+
    //
    
    // new ADC data available?
    if (ADCGetUpdateCount())
    {
        GyroProcessData();
        ADCResetUpdateCount();
    }
    
    //
    // +================================+
    // |  Customizable code ends here.  |
    // +================================+
    //
    
    return;
}       //ProjProcessLocalIO

