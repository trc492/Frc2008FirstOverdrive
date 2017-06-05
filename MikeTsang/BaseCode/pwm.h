#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="pwm.h" />
///
/// <summary>
///     This module contains the definitions of the pwm device.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on code by: R. Kevin Watson (kevinw@jpl.nasa.gov)
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _PWM_H
#define _PWM_H

// These values define how much each respective PWM output
// pulse will increase or decrease relative to the center/
// neutral pulse width (defined below) for each PWM step.
// The default value of fifty provides for a pulse width
// range of 1.28 ms (256*0.000005=0.00128), which is the
// same provided by Generate_Pwms(). If you're using
// servos, you should consider decreasing the gain to
// 40 or less.
#define PWM_13_GAIN             50      // 5.0 microseconds per step
#define PWM_14_GAIN             50      // 5.0 microseconds per step
#define PWM_15_GAIN             50      // 5.0 microseconds per step
#define PWM_16_GAIN             50      // 5.0 microseconds per step

// These values define how wide each respective center/
// neutral pulse is with a PWM input value of 127. The
// default value of 15000 provides for a pulse width of
// 1.5 ms, which is the same provided by Generate_Pwms().
#define PWM_13_CENTER           15000   // 1.5 milliseconds
#define PWM_14_CENTER           15000   // 1.5 milliseconds
#define PWM_15_CENTER           15000   // 1.5 milliseconds
#define PWM_16_CENTER           15000   // 1.5 milliseconds

//
// Exported function prototypes.
//
void
PWMInitialize(
    void
    );

void PWMSetValues(
    __in BYTE bPWM13,
    __in BYTE bPWM14,
    __in BYTE bPWM15,
    __in BYTE bPWM16
    );

void
PWMPrintData(
    void
    );
    
#endif

