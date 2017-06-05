#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="encoder.h" />
///
/// <summary>
///     This module contains the definitions of the encoder device.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on code by: R. Kevin Watson (kevinw@jpl.nasa.gov)
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _encoder_h
#define _encoder_h

//
// Constants.
//
#define MAX_ENCODERS                    6

#define ENCODER_MASK_1                  0x01
#define ENCODER_MASK_2                  0x02
#define ENCODER_MASK_3                  0x04
#define ENCODER_MASK_4                  0x08
#define ENCODER_MASK_5                  0x10
#define ENCODER_MASK_6                  0x20
#define ENCODER_CHANNEL_MASK            (ENCODER_MASK_1 |   \
                                         ENCODER_MASK_2 |   \
                                         ENCODER_MASK_3 |   \
                                         ENCODER_MASK_4 |   \
                                         ENCODER_MASK_5 |   \
                                         ENCODER_MASK_6)

// Digital input pin(s) assigned to the encoder's phase-B output. Make sure
// this pin is configured as an input in user_routines.c/User_Initialization().
#define ENCODER_1_PHASE_B_PIN	        rc_dig_in11
#define ENCODER_2_PHASE_B_PIN	        rc_dig_in12
#define ENCODER_3_PHASE_B_PIN	        rc_dig_in13
#define ENCODER_4_PHASE_B_PIN	        rc_dig_in14
#define ENCODER_5_PHASE_B_PIN	        rc_dig_in15
#define ENCODER_6_PHASE_B_PIN	        rc_dig_in16

//
// Exported function prototypes.
//

void
EncoderInitialize(
    __in BYTE bChannelMask,
    __in BYTE bReverseMask,
    __in BYTE bVelInterval
    );

long
EncoderGetCount(
    __in BYTE bChannel
    );

void
EncoderResetCount(
    __in BYTE bChannel
    );

long
EncoderGetVelocity(
    __in BYTE bChannel
    );
    
void
EncoderPrintData(
    __in BOOL fVerbose
    );
    
void
EncoderTimerTick(
    void
    );
    
void
Encoder1_2IntHandler(
    __in BYTE bChannel,
    __in BYTE bPhaseBPin
    );

void
Encoder3_6IntHandler(
    __in BYTE bPortB
    );

#endif
