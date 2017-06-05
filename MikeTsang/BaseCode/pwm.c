#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="pwm.c" />
///
/// <summary>
///     This module contains all pwm functions.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on code by: R. Kevin Watson (kevinw@jpl.nasa.gov)
///     Environment: MPLAB
///
///     You are free to use this source code for any non-commercial
///     use. Please do not make copies of this source code, modified
///     or un-modified, publicly available on the internet or
///     elsewhere without permission. Thanks.
///
///     Update history:
///     Date        Rev Description
///     ====        === ===========
///     29-Dec-2006 0.1 RKW - Original code.
/// </remarks>
#endif

#include "basecode.h"
#include "pwm.h"

/// <summary>
///     This function initializes the pwm related hardware. It replaces
///     IFI's Generate_Pwms() function.
/// </summary>
///
/// <returns> None. </returns>

void
PWMInitialize(
    __in 
    void
    )
{
    // Select 16-bit read/writes to timer 3.
    T3CONbits.RD16 = 1;
    // CCP2 through CCP5 will use timer 3 for compare mode.
    T3CONbits.T3CCP1 = 1;
    T3CONbits.T3CCP2 = 0;
    // Use a 1:1 prescale for timer 3.
    T3CONbits.T3CKPS1 = 0;
    T3CONbits.T3CKPS0 = 0;
    // Use the internal 10MHz clock for timer 3.
    // Each timer "tick" equals 100ns.
    T3CONbits.TMR3CS = 0;
    // Disable the timer 3 overflow interrupt.
    PIE2bits.TMR3IE = 0;
    // Disable the CCP interrupts.
    PIE2bits.CCP2IE = 0;
    PIE3bits.CCP3IE = 0;
    PIE3bits.CCP4IE = 0;
    PIE3bits.CCP5IE = 0;
    
    return;
}       //PWMInitialize

/// <summary>
///     This function sets the pwm values for channel 13-16.
/// </summary>
///
/// <param name="bPWM13">
///     Specifies the position/velocity value for PWM channel 13.
/// </param>
///
/// <param name="bPWM14">
///     Specifies the position/velocity value for PWM channel 14.
/// </param>
///
/// <param name="bPWM15">
///     Specifies the position/velocity value for PWM channel 15.
/// </param>
///
/// <param name="bPWM16">
///     Specifies the position/velocity value for PWM channel 16.
/// </param>
///
/// <returns> None. </returns>

void PWMSetValues(
    __in BYTE bPWM13,
    __in BYTE bPWM14,
    __in BYTE bPWM15,
    __in BYTE bPWM16
    )
{
    int iPWM13, iPWM14, iPWM15, iPWM16;

    // Stop timer 3.
    T3CONbits.TMR3ON = 0;
    // Initialize timer 3 register to zero.
    TMR3H = 0;
    TMR3L = 0;
    // Reset CCP hardware.
    CCP2CON = 0;
    CCP3CON = 0;
    CCP4CON = 0;
    CCP5CON = 0;
    // Calculate the number of 100 ns timer ticks.
    // Needed to match the desired PWM pulse width.
    iPWM13 = (PWM_13_GAIN*((int)bPWM13 - 127)) + PWM_13_CENTER;
    iPWM14 = (PWM_14_GAIN*((int)bPWM14 - 127)) + PWM_14_CENTER;
    iPWM15 = (PWM_15_GAIN*((int)bPWM15 - 127)) + PWM_15_CENTER;
    iPWM16 = (PWM_16_GAIN*((int)bPWM16 - 127)) + PWM_16_CENTER;

    // Load the CCP compare registers.
    CCPR2L = LOBYTE((WORD)iPWM13);
    CCPR2H = HIBYTE((WORD)iPWM13);

    CCPR3L = LOBYTE((WORD)iPWM14);
    CCPR3H = HIBYTE((WORD)iPWM14);

    CCPR4L = LOBYTE((WORD)iPWM15);
    CCPR4H = HIBYTE((WORD)iPWM15);

    CCPR5L = LOBYTE((WORD)iPWM16);
    CCPR5H = HIBYTE((WORD)iPWM16);

    // Disable all interrupts to prevent an interrupt routine
    // from executing after the CCP hardware is initialized
    // and the moment the timer is started.
    INTCONbits.GIEH = 0;

    // Setup CCP hardware for compare mode (each PWM output
    // transitions from low to high at this point).
    CCP2CON = 0x09;
    CCP3CON = 0x09;
    CCP4CON = 0x09;
    CCP5CON = 0x09;

    // Start timer 3.
    T3CONbits.TMR3ON = 1;

    //Enable interrupts.
    INTCONbits.GIEH = 1;

    return;
}       //PWMSetValues

/// <summary>
///     This function prints the state of the PWMs.
/// </summary>
///
/// <returns> None. </returns>

void
PWMPrintData(
    void
    )
{
    #define CHANNELS_PER_ROW     8
    BYTE i;

    for (i = 0; i < MAX_RC_PWM_CHANNELS; ++i)
    {
        if (i%CHANNELS_PER_ROW == 0)
        {
            if (i != 0)
            {
                printf("\r\n");
            }
            printf("PWM[%02d-%02d]:", i, i + CHANNELS_PER_ROW - 1);
        }
        printf(" %03d", txdata.rc_pwm[i]);
    }
    printf("\r\n");

    return;
}       //PWMPrintData

