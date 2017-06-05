#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="input.h" />
///
/// <summary>
///     This module contains the definitions of the input module.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _input_h
#define _input_h

//
// Exported constants.
//

// PortData.bPortFlag values.
#define PORTF_ENABLE_X                  0x01
#define PORTF_ENABLE_Y                  0x02
#define PORTF_ENABLE_WHEEL              0x04
#define PORTF_ENABLE_AUX                0x08
#define PORTF_ENABLE_MASK               (PORTF_ENABLE_X |       \
                                         PORTF_ENABLE_Y |       \
                                         PORTF_ENABLE_WHEEL |   \
                                         PORTF_ENABLE_AUX)

//
// Exported type definitions.
//
typedef struct _PortData
{
    BYTE    bPort;
    BYTE    bPortFlags;
    int     ixBias;
    int     iyBias;
    int     iwheelBias;
    int     iauxBias;
    int     ixNoiseThreshold;
    int     iyNoiseThreshold;
    int     iwheelNoiseThreshold;
    int     iauxNoiseThreshold;
    int     ix;
    int     iy;
    int     iwheel;
    int     iaux;
} PORTDATA, *PPORTDATA;

typedef struct _InputData
{
    BYTE     bNumPorts;
    PORTDATA Joysticks[NUM_JOYSTICKS];
} INPUTDATA, *PINPUTDATA;

//
// Exported function prototypes.
//
void
InputInitialize(
    __in BYTE bPort1Flags,
    __in BYTE bPort2Flags,
    __in BYTE bPort3Flags,
    __in BYTE bPort4Flags
	);

void
InputCalibrate(
    void
    );
    
void
InputProcessData(
    void
    );

void
InputPrintData(
    void
    );

//
// Exported data.
//
extern INPUTDATA g_InputData;

#endif
