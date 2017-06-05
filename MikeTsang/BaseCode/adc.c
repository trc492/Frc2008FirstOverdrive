#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="adc.c" />
///
/// <summary>
///     This module contains all A/D converter functions.
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
///     10-Jul-2005 0.1 RKW - Original code.
///     17-Jul-2005 0.2 RKW - Added x128 and x256 oversampling options.
///     13-Dec-2005 0.3 RKW - Altered code to use the "special event trigger"
///                           functionality of the CCP2 hardware to initiate
///                           ADC conversions. This was formally done using
///                           timer 2.
///     10-Jan-2006 0.4 RKW - Ported to PIC18F8722, which required going back
///                           to using timer 2 to initiate an analog to digital
///                           conversion due to a bug in the PIC18F8722 design.
///                           Modified #pragma interruptlow statement to
///                           include .tmpdata section.
///     03-Feb-2008 0.5 MTS - Made the module more object oriented.
/// </remarks>
#endif

#include "basecode.h"
#include "adc.h"

//
// Local constants.
//

/// <summary>
///     ADC reference voltage (Vref+) in unit of mV.
/// </summary>
#define VREF_POS_MV             5000L
/// <summary>
///     ADC ground voltage (Vref-) in unit of mV.
/// </summary>
#define VREF_NEG_MV             0L
/// <summary>
///     ADC reference voltage range in unit of mV.
/// </summary>
#define VREF_RANGE_MV           (VREF_POS_MV - VREF_NEG_MV)

//
// Local type definitions.
//

/// <summary>
///     This structure stores information about an A/D converter channel.
/// </summary>
typedef struct _ADCChannel
{
    /// <summary>
    ///     Stores the A/D channel number (0-based).
    /// </summary>
    BYTE            bChannel;
    /// <summary>
    ///     Stores the accumulated value of the A/D channel for oversampling.
    /// </summary>
    volatile DWORD  dwAccum;
    /// <summary>
    ///     Stores the final value of the A/D channel.
    /// </summary>
    volatile WORD   wValue;
} ADC_CHANNEL, *PADC_CHANNEL;

/// <summary>
///     This structure stores information about all the A/D converter channels.
/// </summary>
typedef struct _ADC
{
    /// <summary>
    ///     Stores the A/D converter sampling rate in Hz.
    /// </summary>
    WORD            wSampleRate;
    /// <summary>
    ///     Stores the A/D oversampling rate.
    /// </summary>
    WORD            wSamplesPerUpdate;
    /// <summary>
    ///     Stores the A/D oversampling value range.
    /// </summary>
    WORD            wValueRange;
    /// <summary>
    ///     Stores the A/D oversampling divisor.
    /// </summary>
    BYTE            bValueDivisor;
    /// <summary>
    ///     Stores the number of A/D channels in used.
    /// </summary>
    BYTE            bNumChannels;
    /// <summary>
    ///     Stores the number of samples accumulated so far for each channel.
    /// </summary>
    volatile WORD   wSamples;
    /// <summary>
    ///     Stores the number of oversampled updates.
    /// </summary>
    volatile BYTE   bUpdateCount;
    /// <summary>
    ///     Stores the next channel index for sampling.
    /// </summary>
    volatile BYTE   bChannelIndex;
    /// <summary>
    ///     Stores the information of each A/D converter channel.
    /// </summary>
    ADC_CHANNEL     Channels[NUM_ADC_CHANNELS];
} ADC, *PADC;

//
// Local data.
//
static ADC Adc = {0};

//
// Local function prototypes.
//
static void
Timer2Initialize(
    __in WORD wSampleReate
    );

static void
Timer2Disable(
    void
    );

static int
ADCIndex(
    __in BYTE bChannel
    );

/// <summary>
///     This function initializes the ADC hardware.
/// </summary>
///
/// <param name="wSampleRate">
///     Specifies the sample rates of all ADC channels.
/// </param>
/// <param name="wSamplesPerUpdate">
///     Specifies the number of samples per update (oversampling rate).
///     Must be a power of 2 from 1 to 256.
/// </param>
/// <param name="wChannelMask">
///     Specifies the bit map of in-use ADC channels.
/// </param>
///
/// <returns> None. </returns>
///
/// <remarks>
///     Number of ADC samples that will be averaged for each update. More will
///     generally give you more resolution and less noise (see chart below),
///     but your update rate will decrease proportionately.
///
/// <para>
///     Samples/Update  ResolutionBits  ValueRange  Volt/Bit
/// </para>
/// <para>
///     ==============  ==============  ==========  ========
/// </para>
/// <para>
///           1               10           0-1023    4.88 mV
/// </para>
/// <para>
///           2               10           0-1023    4.88 mV
/// </para>
/// <para>
///           4               11           0-2047    2.44 mV
/// </para>
/// <para>
///           8               11           0-2047    2.44 mV
/// </para>
/// <para>
///          16               12           0-4095    1.22 mV
/// </para>
/// <para>
///          32               12           0-4095    1.22 mV
/// </para>
/// <para>
///          64               13           0-8191     610 uV
/// </para>
/// <para>
///         128               13           0-8191     610 uV
/// </para>
/// <para>
///         256               14           0-16383    305 uV
/// </para>
/// </remarks>

void
ADCInitialize(
    __in WORD wSampleRate,
    __in WORD wSamplesPerUpdate,
    __in WORD wChannelMask
    )
{
    BYTE i;

    Adc.wSampleRate = wSampleRate;
    Adc.wSamplesPerUpdate = wSamplesPerUpdate;
    switch (wSamplesPerUpdate)
    {
        case 1:
            //
            // 10-bit effective resolution.
            //
            Adc.wValueRange = 1024;
            Adc.bValueDivisor = 0;
            break;

        case 2:
            //
            // 10-bit effective resolution.
            //
            Adc.wValueRange = 1024;
            Adc.bValueDivisor = 1;
            break;

        case 4:
        default:
            //
            // 11-bit effective resolution.
            //
            Adc.wValueRange = 2048;
            Adc.bValueDivisor = 1;
            break;

        case 8:
            //
            // 11-bit effective resolution.
            //
            Adc.wValueRange = 2048;
            Adc.bValueDivisor = 2;
            break;

        case 16:
            //
            // 12-bit effective resolution.
            //
            Adc.wValueRange = 4096;
            Adc.bValueDivisor = 2;
            break;

        case 32:
            //
            // 12-bit effective resolution.
            //
            Adc.wValueRange = 4096;
            Adc.bValueDivisor = 3;
            break;

        case 64:
            //
            // 13-bit effective resolution.
            //
            Adc.wValueRange = 8192;
            Adc.bValueDivisor = 3;
            break;

        case 128:
            //
            // 13-bit effective resolution.
            //
            Adc.wValueRange = 8192;
            Adc.bValueDivisor = 4;
            break;

        case 256:
            //
            // 14-bit effective resolution.
            //
            Adc.wValueRange = 16384;
            Adc.bValueDivisor = 4;
            break;
    }

    Adc.wSamples = 0;
    Adc.bUpdateCount = 0;
    Adc.bChannelIndex = 0;
    Adc.bNumChannels = 0;
    for (i = 0; i < MAX_ADC_CHANNELS; ++i)
    {
        //
        // While there is an active ADC channel and the array still have
        // room, we will initialize the channel entry.
        //
        if ((wChannelMask & (1 << i)) &&
            (Adc.bNumChannels < NUM_ADC_CHANNELS))
        {
            //
            // This channel is in used.
            //
            Adc.Channels[Adc.bNumChannels].bChannel = i;
            Adc.Channels[Adc.bNumChannels].dwAccum = 0;
            Adc.Channels[Adc.bNumChannels].wValue = 0;
            Adc.bNumChannels++;
        }
    }
    
    for (i = Adc.bNumChannels; i < NUM_ADC_CHANNELS; ++i)
    {
        //
        // Init unused entries.
        //
        Adc.Channels[i].bChannel = 0;
        Adc.Channels[i].dwAccum = 0;
        Adc.Channels[i].wValue = 0;
    }

    if (Adc.bNumChannels > 0)
    {
        //
        // Enable the ADC hardware and select the first active ADC channel.
        //
        ADCON0 = (BYTE)((Adc.Channels[0].bChannel << 2) | 0b00000001);
        //
        // Enable all 16 analog inputs. The VCFG0 and VCFG1 bits are also
        // set to select Vdd and Vss as the reference voltages.
        //
        ADCON1 = 0;
        //
        // Select Fosc/64 as the ADC Conversion Clock and right justify
        // data in the conversion result register.
        //
        ADCON2 = 0b10000110;
        //
        // ADC interrupt is low priority.
        //
        IPR1bits.ADIP = 0;
        //
        // To prevent a spurious interrupt, make sure the interrupt flag is
        // reset.
        //
        PIR1bits.ADIF = 0;
        //
        // Enable the ADC interrupt.
        //
        PIE1bits.ADIE = 1;
        //
        // Initialize and start timer 2 which is responsible for starting
        // analog to digital conversions.
        //
        Timer2Initialize(wSampleRate);
    }

    return;
}       //ADCInitialize

/// <summary>
///     This function disables the ADC hardware.
/// </summary>
///
/// <returns> None. </returns>

void
ADCDisable(
    void
    )
{
    //
    // Disable timer 2.
    //
    Timer2Disable();
    //
    // Disable ADC interrupt.
    //
    PIE1bits.ADIE = 0;
    //
    // Disable the ADC hardware and select ADC channel 0.
    //
    ADCON0 = 0b00000000;

    return;
}       //ADCDisable

/// <summary>
///     This function returns the last ADC value of a given channel.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the ADC channel. Channel number is 0-based.
/// </param>
///
/// <returns> Returns ADC value of the channel. </returns>

WORD
ADCGetValue(
    __in BYTE bChannel
    )
{
    WORD wADCValue = 0;
    int index = ADCIndex(bChannel);

    if (index >= 0)
    {
        //
        // Disable ADC interrupt.
        //
        PIE1bits.ADIE = 0;
        wADCValue = Adc.Channels[index].wValue;
        //
        // Enable ADC interrupt.
        //
        PIE1bits.ADIE = 1;
    }

    return wADCValue;
}       //ADCGetValue

/// <summary>
///     This function converts the ADC value into mV value.
/// </summary>
///
/// <param name="wADCValue">
///     Specifies the ADC value.
/// </param>
///
/// <returns> Returns a mV value. </returns>

WORD
ADCValueToMillivolt(
    __in WORD wADCValue
    )
{
    return (WORD)(((DWORD)wADCValue*VREF_RANGE_MV)/Adc.wValueRange);
}       //ADCValueToMillivolt

/// <summary>
///     This function returns the ADC value range.
/// </summary>
///
/// <returns> Returns ADC value range. </returns>

WORD
ADCGetValueRange(
    void
    )
{
    return Adc.wValueRange;
}       //ADCGetValueRange

/// <summary>
///     This function returns the ADC update rate.
/// </summary>
///
/// <returns> Returns ADC update rate. </returns>

WORD
ADCGetUpdateRate(
    void
    )
{
    return Adc.wSampleRate/(Adc.wSamplesPerUpdate*Adc.bNumChannels);
}       //ADCGetUpdateRate

/// <summary>
///     This function returns the ADC update count.
/// </summary>
///
/// <returns> Returns the ADC update count. </returns>

BYTE
ADCGetUpdateCount(
    void
    )
{
    BYTE bUpdateCount;
    //
    // Disable ADC interrupt
    //
    PIE1bits.ADIE = 0;

    bUpdateCount = Adc.bUpdateCount;
    //
    // Enable ADC interrupt;
    //
    PIE1bits.ADIE = 1;

    return bUpdateCount;
}       //ADCGetUpdateCount

/// <summary>
///     This function resets the ADC update count.
/// </summary>
///
/// <returns> None. </returns>

void
ADCResetUpdateCount(
    void
    )
{
    //
    // Disable ADC interrupt
    //
    PIE1bits.ADIE = 0;

    Adc.bUpdateCount = 0;
    //
    // Enable ADC interrupt;
    //
    PIE1bits.ADIE = 1;

    return ;
}       //ADCResetUpdateCount

/// <summary>
///     This function prints the state of the ADC channels.
/// </summary>
///
/// <param name="fVerbose">
///     Specifies whether to print verbose information.
/// </param>
///
/// <returns> None. </returns>

void
ADCPrintData(
    __in BOOL fVerbose
    )
{
    BYTE i;

    if (fVerbose)
    {
        printf("ADC: SampleRate=%d, Samples/Update=%d, ValueRange=%d, ValueDiv=%d\r\n",
               Adc.wSampleRate, Adc.wSamplesPerUpdate, Adc.wValueRange, Adc.bValueDivisor);
    }
 
    for (i = 0; i < Adc.bNumChannels; ++i)
    {
        printf("ADC[%d]: Channel=%d, Accum=%ld, Value=%d\r\n",
               i, Adc.Channels[i].bChannel, Adc.Channels[i].dwAccum, Adc.Channels[i].wValue);
    }

    return;
}       //ADCPrintData

/// <summary>
///     This function handles ADC interrupts.
/// </summary>
///
/// <returns> None. </returns>

void
ADCIntHandler(
    void
    )
{
    WORD wADCValue;
    //
    // Get conversion result.
    //
    wADCValue = ADRESH;
    wADCValue <<= 8;
    wADCValue |= ADRESL;
    //
    // Add the ADC data to the appropriate accumulator.
    //
    Adc.Channels[Adc.bChannelIndex].dwAccum += (DWORD)wADCValue;
    //
    // Increment to the next channel index.
    //
    Adc.bChannelIndex++;
    //
    // Check if we need to wrap around to zero.
    //
    if (Adc.bChannelIndex >= Adc.bNumChannels)
    {
        Adc.bChannelIndex = 0;
        Adc.wSamples++;
    }
    //
    // Select next ADC channel. This also starts the process whereby the ADC
    // sample and hold capacitor is allowed to start charging which must be
    // completed before the next A/D conversion can be started.
    //
    ADCON0 = (BYTE)((Adc.Channels[Adc.bChannelIndex].bChannel << 2) | 0b00000001);
    //
    // Check to see if we've got a full sample set.
    //
    if (Adc.wSamples >= Adc.wSamplesPerUpdate)
    {
        BYTE i;
        //
        // Update the channel array.
        //
        for (i = 0; i < Adc.bNumChannels; ++i)
        {
            Adc.Channels[i].wValue = Adc.Channels[i].dwAccum >> Adc.bValueDivisor;
            //
            // Reset the accumulator.
            //
            Adc.Channels[i].dwAccum = 0;
        }
        //
        // Indicate a fresh sample set is available.
        //
        Adc.bUpdateCount++;
        //
        // Start a fresh sample set.
        //
        Adc.wSamples = 0;
    }

    return;
}       //ADCIntHandler

/// <summary>
///     This function handles timer 2 interrupts.
/// </summary>
///
/// <returns> None. </returns>

void
ADCTimer2IntHandler(
    void
    )
{
    //
    // Start a new A/D conversion.
    //
    ADCON0bits.GO = 1;

    return;
}       //ADCTimer2IntHandler

/// <summary>
///     This function initializes timer 2 that is responsible for
///     periodically starting A/D conversions.
/// </summary>
///
/// <param name="wSampleRate">
///     Specifies the sample rates of all ADC channels.
/// </param>
///
/// <returns> None. </returns>

static void
Timer2Initialize(
    __in WORD wSampleRate
    )
{
    switch (wSampleRate)
    {
        case ADC_SAMPLE_RATE_6400HZ:
            //
            //  Use a 1:4 prescaler and 1:11 postscaler.
            //
            T2CON = 0b01010000;
            //
            // Count to 140 before rolling over and gnerating
            // an interrupt (142.05 - 2 is ideal).
            //
            PR2 = 140;
            break;

        case ADC_SAMPLE_RATE_3200HZ:
            //
            //  Use a 1:1 prescaler and 1:11 postscaler.
            //
            T2CON = 0b01010001;
            //
            // Count to 69 before rolling over and gnerating
            // an interrupt (71.02 - 2 is ideal).
            //
            PR2 = 69;
            break;

        case ADC_SAMPLE_RATE_1600HZ:
            //
            //  Use a 1:4 prescaler and 1:11 postscaler.
            //
            T2CON = 0b01010001;
            //
            // Count to 140 before rolling over and gnerating
            // an interrupt (142.05 - 2 is ideal).
            //
            PR2 = 140;
            break;

        case ADC_SAMPLE_RATE_800HZ:
            //
            //  Use a 1:16 prescaler and 1:11 postscaler.
            //
            T2CON = 0b01010010;
            //
            // Count to 69 before rolling over and gnerating
            // an interrupt (71.02 - 2 is ideal).
            //
            PR2 = 69;
            break;

        case ADC_SAMPLE_RATE_400HZ:
            //
            //  Use a 1:16 prescaler and 1:11 postscaler.
            //
            T2CON = 0b01010010;
            //
            // Count to 140 before rolling over and gnerating
            // an interrupt (142.05 - 2 is ideal).
            //
            PR2 = 140;
            break;

        case ADC_SAMPLE_RATE_200HZ:
        default:
            //
            // If non-supported rate is specified, default to a 200 Hz
            // ADC sample rate.
            //
            // Use a 1:16 prescaler and 1:14 postscaler.
            //
            T2CON = 0b01101010;
            //
            // Count to 221 before rolling over and generating
            // an interrupt (223.21 - 2 is ideal)
            //
            PR2 = 221;
            break;
    }
    //
    // Make sure timer 2 register starts at zero.
    //
    TMR2 = 0x00;
    //
    // Timer 2 interrupt is low priority.
    //
    IPR1bits.TMR2IP = 0;
    //
    // To prevent spurious interrupt, make sure the interrupt flag is reset.
    //
    PIR1bits.TMR2IF = 0;
    //
    // Enable timer 2 interrupt.
    //
    PIE1bits.TMR2IE = 1;
    //
    // Enable timer 2.
    //
    T2CONbits.TMR2ON = 1;

    return;
}       //Timer2Initialize

/// <summary>
///     This function disables the timer 2 hardware.
/// </summary>
///
/// <returns> None. </returns>

static void
Timer2Disable(
    void
    )
{
    //
    // Disable timer 2 interrupt.
    //
    PIE1bits.TMR2IE = 0;
    //
    // Disable timer 2.
    //
    T2CONbits.TMR2ON = 0;

    return;
}       //Timer2Disable

/// <summary>
///     This function finds the ADC channel from the ADC array and
///     returns the index to it.
/// </summary>
///
/// <param name="bChannel">
///     Specifies the ADC channel to look for.
/// </param>
///
/// <returns> Success: Returns the channel index. </returns>
/// <returns> Failure: Returns -1. </returns>

static int
ADCIndex(
    __in BYTE bChannel
    )
{
    int index = -1;
    BYTE i;

    for (i = 0; i < Adc.bNumChannels; ++i)
    {
        if (Adc.Channels[i].bChannel == bChannel)
        {
            index = i;
            break;
        }
    }

    return index;
}       //ADCIndex

