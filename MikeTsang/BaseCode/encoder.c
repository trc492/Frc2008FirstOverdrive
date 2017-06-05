#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="encoder.c" />
///
/// <summary>
///     This module contains all encoder functions.
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
///     20-Dec-2003 0.1 RKW - Original code.
///     18-Feb-2004 0.2 RKW - Reassigned the encoder digital inputs to run
///                           on the FRC robot controller too.
///     01-Jan-2005 0.3 RKW - Get_Left_Encoder_Count(),
///                           Get_Right_Encoder_Count(),
///                           Reset_Left_Encoder_Count() and
///                           Reset_Right_Encoder_Count() functions added.
///     01-Jan-2005 0.3 RKW - Renamed Int_1_Handler() and Int_2_Handler() to
///                           Left_Encoder_Int_Handler() and
///                           Right_Encoder_Int_Handler respectively.
///     01-Jan-2005 0.3 RKW - Altered the interrupt service routines to easily
///                           flip the direction the encoders count by altering
///                           the RIGHT_ENCODER_TICK_DELTA and
///                           LEFT_ENCODER_TICK_DELTA #defines found in
///                           encoder.h
///     06-Jan-2005 0.4 RKW - Rebuilt with C18 version 2.40.
///     17-Dec-2005 0.5 RKW - Added code to accomodate four more encoder on
///                           interrupts 3 through 6. These additional encoder
///                           inputs are optimized for position control.
///     13-Jan-2006 0.5 RKW - Verified code works properly on new PIC18F8722-
///                           based robot controllers.
///     03-Feb-2008 0.6 MTS - Made the module more object oriented.
/// </remarks>
#endif

#include "basecode.h"
#include "encoder.h"

//
// Local type definitions.
//

/// <summary>
///     This structure stores the states of an encoder channel.
/// </summary>
typedef struct _EncChannel
{
    /// <summary>
    ///     Stores the encoder channel number (0-based).
    /// </summary>
    BYTE            bChannel;
    /// <summary>
    ///     Stores the phase A pin state. This field is only used for
    ///     encoder channels 3-6.
    /// </summary>
    BYTE            bState;
    /// <summary>
    ///     Specifies if the encoder direction is reversed.
    /// </summary>
    BOOL            fReverse;
    /// <summary>
    ///     Stores the previous encoder count. This field is used for
    ///     calculating the velocity by calculating the difference
    ///     between the current and previous encoder counts.
    /// </summary>
    volatile long   lPrevCount;
    /// <summary>
    ///     Stores the current encoder count.
    /// </summary>
    volatile long   lCount;
    /// <summary>
    ///     Stores the calculated velocity for the current system timer tick.
    /// </summary>
    volatile long   lVelocity;
} ENCCHANNEL, *PENCCHANNEL;

/// <summary>
///     This structure stores everything related to the encoders.
/// </summary>
typedef struct _Encoder
{
    /// <summary>
    ///     Stores the number of encoder channels in used.
    /// </summary>
    BYTE            bNumChannels;
    /// <summary>
    ///     Stores the interval in system timer ticks for sampling the encoder
    ///     velocity.
    /// </summary>
    BYTE            bVelInterval;
    /// <summary>
    ///     Stores the previous PortB state.
    /// </summary>
    BYTE            bOldPortB;
    /// <summary>
    ///     Stores the states of each encoder channel.
    /// </summary>
    ENCCHANNEL      Channels[NUM_ENCODERS];
} ENCODER, *PENCODER;

//
// Local data.
//
static ENCODER Encoder = {0};

//
// Local function prototypes.
//
static void
Encoder3_6Int(
    __in BYTE bChannel,
    __in BYTE bPhaseBPin,
    __in BYTE bState
    );

static void
EncoderInterrupt(
    __in BYTE bChannel,
    __in BYTE fEnable
    );

static int
EncoderIndex(
    __in BYTE bChannel
    );

/// <summary>
///     This function initializes the specified encoders.
/// </summary>
///
/// <param name="bChannelMask">
///     Specifies the bit map of in-use encoder channels.
/// </param>
/// <param name="bReverseMask">
///     Specifies the bit map of channels that are reversed.
/// </param>
/// <param name="bVelInterval">
///     Specifies the sampling interval of the encoder velocity.
/// </param>
///
/// <returns> None. </returns>

void
EncoderInitialize(
    __in BYTE bChannelMask,
    __in BYTE bReverseMask,
    __in BYTE bVelInterval
    )
{
    BYTE i;
    BOOL fEncoder_3_6_Inited = FALSE;

    Encoder.bNumChannels = 0;
    Encoder.bVelInterval = bVelInterval;
    Encoder.bOldPortB = 0xff;
    for (i = 0; i < MAX_ENCODERS; ++i)
    {
        //
        // While there is an active ADC channel and the array still have
        // room, we will initialize the channel entry.
        //
        if ((bChannelMask & (1 << i)) &&
            (Encoder.bNumChannels < NUM_ENCODERS))
        {
            //
            // This channel is in used.
            //
            Encoder.Channels[Encoder.bNumChannels].bChannel = i;
            Encoder.Channels[Encoder.bNumChannels].bState = 0;
            Encoder.Channels[Encoder.bNumChannels].fReverse = (bReverseMask & (1 << i)) != 0;
            Encoder.Channels[Encoder.bNumChannels].lPrevCount = 0;
            Encoder.Channels[Encoder.bNumChannels].lCount = 0;
            Encoder.Channels[Encoder.bNumChannels].lVelocity = 0;
            Encoder.bNumChannels++;
            switch (i)
            {
                case 0:
                    //
                    // Encoder 1.
                    //

                    // Make sure interrupt 1 is configured as an input.
                    TRISBbits.TRISB2 = 1;
                    // Interrupt 1 is low priority.
                    INTCON3bits.INT2IP = 0;
                    // Trigger on rising edge.
                    INTCON2bits.INTEDG2 = 1;
                    // Make sure the interrupt flag is reset before enabling.
                    INTCON3bits.INT2IF = 0;
                    // Enable interrupt 1.
                    INTCON3bits.INT2IE = 1;
                    break;

                case 1:
                    //
                    // Encode 2.
                    //

                    // Make sure interrupt 2 is configured as an input.
                    TRISBbits.TRISB3 = 1;
                    // Interrupt 2 is low priority.
                    INTCON2bits.INT3IP = 0;
                    // Trigger on rising edge.
                    INTCON2bits.INTEDG3 = 1;
                    // Make sure the interrupt flag is reset before enabling.
                    INTCON3bits.INT3IF = 0;
                    // Enable interrupt 2.
                    INTCON3bits.INT3IE = 1;
                    break;

                case 2:
                case 3:
                case 4:
                case 5:
                    //
                    // Encoder 3-6.
                    //
                    if (!fEncoder_3_6_Inited)
                    {
                        // Make sure interrupts 3-6 are configured as inputs.
                        TRISBbits.TRISB4 = 1;
                        TRISBbits.TRISB5 = 1;
                        TRISBbits.TRISB6 = 1;
                        TRISBbits.TRISB7 = 1;
                        // Interrupts 3-6 are low priority.
                        INTCON2bits.RBIP = 0;
                        // Before enabling interrupts 3-6, take a snapshot of
                        // port B so that we'll know which interrupt pin
                        // changed state the next time through. The state
                        // of port b is saved in this variable each time the
                        // interrupt handler for interrupts 3 through 6 is
                        // called. This variable should be initialized to the
                        // current state of port B just before enabling
                        // interrupts 3 through 6.
                        Encoder.bOldPortB = PORTB;
                        // Make sure the interrupt flag is reset before
                        // enabling.
                        INTCONbits.RBIF = 0;
                        // Enable interrupts 3 through 6
                        INTCONbits.RBIE = 1;
                        fEncoder_3_6_Inited = TRUE;
                    }
                    break;
            }
        }
    }
    
    for (i = Encoder.bNumChannels; i < NUM_ENCODERS; ++i)
    {
        //
        // Init unused entries.
        //
        Encoder.Channels[i].bChannel = 0;
        Encoder.Channels[i].bState = 0;
        Encoder.Channels[i].fReverse = FALSE;
        Encoder.Channels[i].lPrevCount = 0;
        Encoder.Channels[i].lCount = 0;
        Encoder.Channels[i].lVelocity = 0;
    }

    return;
}       //EncoderInitialize

/// <summary>
///     This function gets the current count of the given encoder.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to get its count.
/// </param>
///
/// <returns> Returns the encoder count. </returns>

long
EncoderGetCount(
    __in BYTE bChannel
    )
{
    long lCount = 0;
    int index = EncoderIndex(bChannel);

    if (index >= 0)
    {
        // Since we're about to access the Encoder Count variable
        // which can also be modified in the interrupt service routine,
        // we'll briefly disable the encoder's interrupt to make sure
        // that the Encoder Count variable doesn't get altered while
        // we're using it.
        EncoderInterrupt(bChannel, 0);

        lCount = Encoder.Channels[index].lCount;
        //
        // Okay, we have a local copy of the encoder count, so turn the
        // encoder's interrupt back on.
        //
        EncoderInterrupt(bChannel, 1);
    }

    return lCount;
}       //EncoderGetCount

/// <summary>
///     This function resets the current count of the given encoder.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to reset its count.
/// </param>
///
/// <returns> None. </returns>

void
EncoderResetCount(
    __in BYTE bChannel
    )
{
    int index = EncoderIndex(bChannel);

    if (index >= 0)
    {
        // Since we're about to access the Encoder Count variable
        // which can also be modified in the interrupt service routine,
        // we'll briefly disable the encoder's interrupt to make sure
        // that the Encoder Count variable doesn't get altered while
        // we're using it.
        EncoderInterrupt(bChannel, 0);

        Encoder.Channels[index].lCount = 0;
        Encoder.Channels[index].lPrevCount = 0;
        Encoder.Channels[index].lVelocity = 0;
        //
        // Okay, we have a local copy of the encoder count, so turn the
        // encoder's interrupt back on.
        //
        EncoderInterrupt(bChannel, 1);
    }

    return;
}       //EncoderResetCount

/// <summary>
///     This function gets the current velocity of the given encoder.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to get its velocity.
/// </param>
///
/// <returns> Returns the encoder velocity. </returns>

long
EncoderGetVelocity(
    __in BYTE bChannel
    )
{
    long lVelocity = 0;
    int index = EncoderIndex(bChannel);

    if (index >= 0)
    {
        lVelocity = Encoder.Channels[index].lVelocity;
    }

    return lVelocity;
}       //EncoderGetVelocity

/// <summary>
///     This function prints the state of the encoders.
/// </summary>
///
/// <param name="fVerbose">
///     Sepcifies whether to print verbose info.
/// </param>
///
/// <returns> None. </returns>

void
EncoderPrintData(
    __in BOOL fVerbose
    )
{
    BYTE i;

    if (fVerbose)
    {
        printf("Encoder: VelInterval=%d, OldPortB=%d\r\n",
               Encoder.bVelInterval, Encoder.bOldPortB);
    }
    
    for (i = 0; i < Encoder.bNumChannels; ++i)
    {
        printf("Encoder[%d]: Channel=%d, State=%d, Count=%ld, PrevCount=%ld, Velocity=%ld\r\n",
               i, Encoder.Channels[i].bChannel, Encoder.Channels[i].bState,
               Encoder.Channels[i].lCount, Encoder.Channels[i].lPrevCount,
               Encoder.Channels[i].lVelocity);
    }

    return;
}       //EncoderPrintData

/// <summary>
///     This function is called by the system timer tick handler so that we can sample the
///     encoder velocities.
/// </summary>
///
/// <returns> None. </returns>

void
EncoderTimerTick(
    void
    )
{
    BYTE i;
    long lCount;
    
    if ((Encoder.bNumChannels > 0) &&
        (g_dwCurrentTicks%Encoder.bVelInterval == 0))
    {
        for (i = 0; i < Encoder.bNumChannels; ++i)
        {
            lCount = Encoder.Channels[i].lCount;
            Encoder.Channels[i].lVelocity = lCount - Encoder.Channels[i].lPrevCount;
            Encoder.Channels[i].lPrevCount = lCount;
        }
    }
    
    return;
}       //EncoderTimerTick

/// <summary>
///     This function handles the encoder 1 interrupt. If enabled, it is
///     called when the interrupt pin changes from a logic 0 to a logic 1.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to get its cound.
/// </param>
/// <param name="bPhaseBPin">
///     Specifies the state of the phase B pin.
/// </param>
///
/// <returns> None. </returns>

void
Encoder1_2IntHandler(
    __in BYTE bChannel,
    __in BYTE bPhaseBPin
    )
{
    int index = EncoderIndex(bChannel);

    if (index >= 0)
    {
        long lDelta = Encoder.Channels[index].fReverse? -1: 1;
        
        //
        // Encoder's phase A signal just transitioned from zero to one causing
        // this interrupt service routine to be called. We know that the
        // encoder just rotated one count or "tick" so now check the logical
        // state of the phase B signal to determine which way the the encoder
        // shaft rotated.
        //
        if (bPhaseBPin == 0)
        {
            Encoder.Channels[index].lCount -= lDelta;
        }
        else
        {
            Encoder.Channels[index].lCount += lDelta;
        }
    }

    return;
}       //Encoder1_2IntHandler

/// <summary>
///     This function dispatches the encoder 3-6 interrupts.
/// </summary>
///
/// <param name="bPortB">
///     Specifies the value read from Port B.
/// </param>
///
/// <returns> None. </returns>

void
Encoder3_6IntHandler(
    __in BYTE bPortB
    )
{
    BYTE bPortBDelta = bPortB^Encoder.bOldPortB;

    Encoder.bOldPortB = bPortB;
    if (bPortBDelta & 0x10)
    {
        //
        // Encoder 3 changed state.
        //
        Encoder3_6Int(2, ENCODER_3_PHASE_B_PIN, (bPortB & 0x10)? 1: 0);
    }

    if (bPortBDelta & 0x20)
    {
        //
        // Encoder 4 changed state.
        //
        Encoder3_6Int(3, ENCODER_4_PHASE_B_PIN, (bPortB & 0x20)? 1: 0);
    }

    if (bPortBDelta & 0x40)
    {
        //
        // Encoder 5 changed state.
        //
        Encoder3_6Int(4, ENCODER_5_PHASE_B_PIN, (bPortB & 0x40)? 1: 0);
    }

    if (bPortBDelta & 0x80)
    {
        //
        // Encoder 6 changed state.
        //
        Encoder3_6Int(5, ENCODER_6_PHASE_B_PIN, (bPortB & 0x80)? 1: 0);
    }

    return;
}       //Encoder3_6IntHandler

/// <summary>
///     This function handles the encoder 3 interrupt. If enabled, it is
///     called when the interrupt pin changes logical state.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to get its cound.
/// </param>
/// <param name="bPhaseBPin">
///     Specifies the state of the phase B pin.
/// </param>
/// <param name="bState">
///     Specifies the phase A pin state.
/// </param>
///
/// <returns> None. </returns>

static void
Encoder3_6Int(
    __in BYTE bChannel,
    __in BYTE bPhaseBPin,
    __in BYTE bState
    )
{
    int index = EncoderIndex(bChannel);

    if (index >= 0)
    {
        long lDelta = Encoder.Channels[index].fReverse? -1: 1;
        
        //
        // Encoder's phase A signal just changed logic level causing this
        // interrupt service routine to be called.
        //
        if (bState == 1)
        {
            // Rising-edge interrupt
            if (bPhaseBPin == 0)
            {
                // Backward
                if (Encoder.Channels[index].bState == 1)
                {
                    Encoder.Channels[index].lCount -= lDelta;
                }
            }
            else
            {
                // Forward
                if (Encoder.Channels[index].bState == 0)
                {
                    Encoder.Channels[index].lCount += lDelta;
                }
            }
        }
        else
        {
            // Falling-edge interrupt:
            //   Phase b is zero if moving forward
            //   phase b is one if moving backward
            Encoder.Channels[index].bState = bPhaseBPin;
        }
    }

    return;
}       //Encoder3_6Int

/// <summary>
///     This function enables or disable interrupt for the encoder channel.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to get its cound.
/// </param>
/// <param name="fEnable">
///     If 1, enable interrupt. If 0, disable interrupt.
/// </param>
///
/// <returns> None. </returns>

static void
EncoderInterrupt(
    __in BYTE bChannel,
    __in BYTE fEnable
    )
{
    switch (bChannel)
    {
        case 1:
            INTCON3bits.INT2IE = fEnable;
            break;

        case 2:
            INTCON3bits.INT3IE = fEnable;
            break;

        case 3:
        case 4:
        case 5:
        case 6:
            INTCONbits.RBIE = fEnable;
            break;
    }

    return;
}       //EncoderInterrupt

/// <summary>
///     This function finds the encoder channel from the Encoder array and
///     returns the index to it.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the encoder channel to look for.
/// </param>
///
/// <returns> Success: Returns the channel index. </returns>
/// <returns> Failure: Returns -1. </returns>

static int
EncoderIndex(
    __in BYTE bChannel
    )
{
    int index = -1;
    BYTE i;

    for (i = 0; i < Encoder.bNumChannels; ++i)
    {
        if (Encoder.Channels[i].bChannel == bChannel)
        {
            index = i;
            break;
        }
    }

    return index;
}       //EncoderIndex

