#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="inthandler.c" />
///
/// <summary>
///     This module contains the main interrupt handler/dispatcher.
/// </summary>
///
/// <remarks>
///     Modified: Michael Tsang (mikets@hotmail.com)
///     Date:     03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "serial_ports.h"

//
// Local function prototypes.
//
void
InterruptHandlerLow(
    void
    );

#pragma code InterruptVectorLow = LOW_INT_VECTOR

/// <summary>
///     This function contains the low priority interrupt vector.
/// </summary>
///
/// <returns> None. </returns>

void
InterruptVectorLow(
    void
    )
{
    _asm
        goto InterruptHandlerLow    //Jump to low priority interrupt handler
    _endasm
}       //InterruptVectorLow

#pragma code
#pragma interruptlow InterruptHandlerLow save=PROD,section(".tmpdata")

/// <summary>
///     This function handles low priority interrupts.
/// </summary>
///
/// <returns> None. </returns>
///
/// <remarks>
///     If you want to use these external low priority interrupts or any of the
///     peripheral interrupts then you must enable them in your initialization
///     routine. Innovation First, Inc. will not provide support for using these
///     interrupts, so be careful. There is great potential for glitchy code if good
///     interrupt programming practices are not followed. Especially read p. 28 of
///     the "MPLAB(R) C18 C Compiler User's Guide" for information on context saving.
/// </remarks>

void
InterruptHandlerLow(
    void
    )
{
    if (PIR1bits.TMR1IF)
    {
        //
        // Timer 1 interrupt. This is the system clock tick.
        //
        PIR1bits.TMR1IF = 0;    //Clear the Timer1 Interrupt Flag
        TMR1H = 0x7F;           //Reload Timer count
        TMR1L = 0xFF;

        g_dwCurrentTicks++;
        if (g_dwCurrentTicks%g_bTraceInterval == 0)
        {
            if (!(g_dwSysFlags & SYSF_CONSOLE_MODE))
            {
                g_dwSysFlags |= SYSF_TRACE_ON;
            }
        }
        else
        {
            g_dwSysFlags &= ~SYSF_TRACE_ON;
        }
        EncoderTimerTick();
    }
    else if (PIR1bits.RC1IF && PIE1bits.RC1IE)
    {
        //
        // Serial port 1 receive interrupt.
        //
        SerialRxIntHandler(SERIAL_PORT_ONE);
    }                              
    else if (PIR3bits.RC2IF && PIE3bits.RC2IE)
    {
        //
        // Serial port 2 receive interrupt.
        //
        SerialRxIntHandler(SERIAL_PORT_TWO);
    } 
    else if (PIR1bits.TX1IF && PIE1bits.TX1IE)
    {
        //
        // Serial port 1 transmit interrupt.
        //
        SerialTxIntHandler(SERIAL_PORT_ONE);
    }                              
    else if (PIR3bits.TX2IF && PIE3bits.TX2IE)
    {
        //
        // Serial port 2 transmit interrupt.
        //
        SerialTxIntHandler(SERIAL_PORT_TWO);
    }
    else if(PIR1bits.TMR2IF && PIE1bits.TMR2IE) // timer 2 interrupt?
    {
        //
        // Timer 2 interrupt. This is used by the A/D converters as
        // a sample rate timer.
        //
        PIR1bits.TMR2IF = 0; // clear the timer 2 interrupt flag [92]
        ADCTimer2IntHandler();
    }                     
    else if(PIR1bits.ADIF && PIE1bits.ADIE) // ADC interrupt
    {
        //
        // ADC interrupt. 
        //
        PIR1bits.ADIF = 0; // clear the ADC interrupt flag
        ADCIntHandler();
    }       
    else if (INTCON3bits.INT2IF && INTCON3bits.INT2IE) // encoder 1 interrupt?
    {
        //
        // Encoder 1 interrupt.
        //
        INTCON3bits.INT2IF = 0; // clear the interrupt flag
        Encoder1_2IntHandler(0, ENCODER_1_PHASE_B_PIN);
    }
    else if (INTCON3bits.INT3IF && INTCON3bits.INT3IE)
    {
        //
        // Encoder 2 interrupt.
        //
        INTCON3bits.INT3IF = 0; // clear the interrupt flag
        Encoder1_2IntHandler(1, ENCODER_2_PHASE_B_PIN);
    }
    else if (INTCONbits.RBIF && INTCONbits.RBIE) // encoder 3-6 interrupt?
    {
        BYTE Port_B = PORTB;
        //
        // Encode 3-6 interrupt.
        //
        INTCONbits.RBIF = 0; // clear the interrupt flag
        Encoder3_6IntHandler(Port_B);
    }
    
    return;
}       //InterruptHandlerLow
