#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="adc.h" />
///
/// <summary>
///     This module contains the definitions of the A/D converter.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on code by: R. Kevin Watson (kevinw@jpl.nasa.gov)
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _adc_h
#define _adc_h

//
// Constants.
//
#define MAX_ADC_CHANNELS                16

#define ADC_SAMPLE_RATE_200HZ           200
#define ADC_SAMPLE_RATE_400HZ           400
#define ADC_SAMPLE_RATE_800HZ           800
#define ADC_SAMPLE_RATE_1600HZ          1600
#define ADC_SAMPLE_RATE_3200HZ          3200
#define ADC_SAMPLE_RATE_6400HZ          6400
#define ADC_SAMPLE_RATE_DEFAULT         ADC_SAMPLE_RATE_200HZ

#define ADC_MASK_1                      0x0001
#define ADC_MASK_2                      0x0002
#define ADC_MASK_3                      0x0004
#define ADC_MSAK_4                      0x0008
#define ADC_MASK_5                      0x0010
#define ADC_MASK_6                      0x0020
#define ADC_MASK_7                      0x0040
#define ADC_MASK_8                      0x0080
#define ADC_MASK_9                      0x0100
#define ADC_MASK_10                     0x0200
#define ADC_MASK_11                     0x0400
#define ADC_MASK_12                     0x0800
#define ADC_MASK_13                     0x1000
#define ADC_MASK_14                     0x2000
#define ADC_MASK_15                     0x4000
#define ADC_MASK_16                     0x8000

//
// Exported function prototypes.
//
void
ADCInitialize(
    __in WORD wSampleRate,
    __in WORD wSamplesPerUpdate,
    __in WORD wChannelMask
    );

void
ADCDisable(
    void
    );

WORD
ADCGetValue(
    __in BYTE bChannel
    );

WORD
ADCValueToMillivolt(
    __in WORD wADCValue
    );

WORD
ADCGetValueRange(
    void
    );

WORD
ADCGetUpdateRate(
    void
    );

BYTE
ADCGetUpdateCount(
    void
    );

void
ADCResetUpdateCount(
    void
    );

void
ADCPrintData(
    __in BOOL fVerbose
    );
    
void
ADCIntHandler(
    void
    );

void
ADCTimer2IntHandler(
    void
    );

#endif
