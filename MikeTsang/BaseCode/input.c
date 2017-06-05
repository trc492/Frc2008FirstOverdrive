#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="input.c" />
///
/// <summary>
///     This module contains all functions concerning input data.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang
///     Date:   03-Feb-2008
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "input.h"

//
// Local constants.
//

/// <summary>
///     The maximum number of joystick ports on the Operating Interface.
/// </summary>
#define MAX_JOYSTICK_PORTS              4
/// <summary>
///     The number of samples used for each joystick calibration.
/// </summary>
#define MAX_INPUTCAL_SAMPLES            20
/// <summary>
///     Noise margin value added to calibrated noise range.
/// </summary>
#define JOYSTICK_NOISE_MARGIN           5

// OiCalData.bCalFlags values.

/// <summary>
///     All inputs have been calibrated.
/// </summary>
#define CALF_CALIBRATED                 0x01
/// <summary>
///     Inputs are being calibrated.
/// </summary>
#define CALF_CALIBRATING                0x80

//
// Local type definitions.
//

/// <summary>
///     This structure stores the temporary data during input calibration.
/// </summary>
typedef struct _OiCalData
{
    /// <summary>
    ///     Indicates whether we are in calibration mode or we are done
    ///     with calibration.
    /// </summary>
    BYTE    bCalFlags;
    /// <summary>
    ///     Stores the current calibrating port index.
    /// </summary>
    BYTE    bCalPortIndex;
    /// <summary>
    ///     Stores the number of calibration sample accumulated so far.
    /// </summary>
    int     icCalSamples;
    /// <summary>
    ///     Accumulates the input X value for averaging.
    /// </summary>
    long    lxAccum;
    /// <summary>
    ///     Accumulates the input Y value for averaging.
    /// </summary>
    long    lyAccum;
    /// <summary>
    ///     Accumulates the input Wheel value for averaging.
    /// </summary>
    long    lwheelAccum;
    /// <summary>
    ///     Accumulates the input Aux value for averaging.
    /// </summary>
    long    lauxAccum;
    /// <summary>
    ///     Stores the minimum noise value for X.
    /// </summary>
    BYTE    bxNoiseMin;
    /// <summary>
    ///     Stores the maximum noise value for X.
    /// </summary>
    BYTE    bxNoiseMax;
    /// <summary>
    ///     Stores the minimum noise value for Y.
    /// </summary>
    BYTE    byNoiseMin;
    /// <summary>
    ///     Stores the maximum noise value for Y.
    /// </summary>
    BYTE    byNoiseMax;
    /// <summary>
    ///     Stores the minimum noise value for Wheel.
    /// </summary>
    BYTE    bwheelNoiseMin;
    /// <summary>
    ///     Stores the maximum noise value for Wheel.
    /// </summary>
    BYTE    bwheelNoiseMax;
    /// <summary>
    ///     Stores the minimum noise value for Aux.
    /// </summary>
    BYTE    bauxNoiseMin;
    /// <summary>
    ///     Stores the maximum noise value for Aux.
    /// </summary>
    BYTE    bauxNoiseMax;
} OICALDATA, *POICALDATA;

//
// Exported data.
//
INPUTDATA      g_InputData = {0};

//
// Local data.
//
static BYTE *xPorts[MAX_JOYSTICK_PORTS] = {&p1_x, &p2_x, &p3_x, &p4_x};
static BYTE *yPorts[MAX_JOYSTICK_PORTS] = {&p1_y, &p2_y, &p3_y, &p4_y};
static BYTE *wheelPorts[MAX_JOYSTICK_PORTS] = {&p1_wheel, &p2_wheel, &p3_wheel, &p4_wheel};
static BYTE *auxPorts[MAX_JOYSTICK_PORTS] = {&p1_aux, &p2_aux, &p3_aux, &p4_aux};
static OICALDATA OiCalData = {0};

//
// Local function prototypes
//
static void
InitializePort(
    __in BYTE bPort,
    __in BYTE bPortFlags
    );

/// <summary>
///     This function initializes the input data. This includes calibrating
///     all in-use analog channels. The Operator Interface has 4 joystick
///     ports and each has 4 analog channels totalling 16 analog channels.
///     This function allows specifying which joystick ports and which analog
///     channels are in used so that only those channels are initialized
///     and calibrated.
/// </summary>
///
/// <param name="bPort1Flags">
///     Specifies which analog channels of joystick 1 are in use.
/// </param>
/// <param name="bPort2Flags">
///     Specifies which analog channels of joystick 2 are in use.
/// </param>
/// <param name="bPort3Flags">
///     Specifies which analog channels of joystick 3 are in use.
/// </param>
/// <param name="bPort4Flags">
///     Specifies which analog channels of joystick 4 are in use.
/// </param>
///
/// <returns> None. </returns>

void
InputInitialize(
    __in BYTE bPort1Flags,
    __in BYTE bPort2Flags,
    __in BYTE bPort3Flags,
    __in BYTE bPort4Flags
	)
{
    BYTE i;

    g_InputData.bNumPorts = 0;
    if (bPort1Flags != 0)
    {
        //
        // Joystick 1 is in use, initialize it for calibration.
        //
        InitializePort(0, bPort1Flags);
    }

    if (bPort2Flags != 0)
    {
        //
        // Joystick 2 is in use, initialize it for calibration.
        //
        InitializePort(1, bPort2Flags);
    }

    if (bPort3Flags != 0)
    {
        //
        // Joystick 3 is in use, initialize it for calibration.
        //
        InitializePort(2, bPort3Flags);
    }

    if (bPort4Flags != 0)
    {
        //
        // Joystick 4 is in use, initialize it for calibration.
        //
        InitializePort(3, bPort4Flags);
    }

    //
    // We also need to initialize the unused entries, if any.
    //
    for (i = g_InputData.bNumPorts; i < NUM_JOYSTICKS; ++i)
    {
        g_InputData.Joysticks[i].bPort = 0;
        g_InputData.Joysticks[i].bPortFlags = 0;
    }

    if (!(OiCalData.bCalFlags & CALF_CALIBRATED))
    {
        //
        // Input has not been calibrated yet, let's initialize calibration
        // data and start the calibration cycle.
        //
        InputCalibrate();
    }

    return;
}       //InputInitialize

/// <summary>
///     This function initializes for joystick calibration.
/// </summary>
///
/// <returns> None. </returns>

void
InputCalibrate(
    void
    )
{
    printf("\r\nStarting input controller calibration, please do not touch the controllers until done...\r\n");
    OiCalData.bCalPortIndex = 0;    //Start with entry 0.
    OiCalData.icCalSamples = 0;
    OiCalData.lxAccum =
    OiCalData.lyAccum =
    OiCalData.lwheelAccum =
    OiCalData.lauxAccum = 0;
    OiCalData.bxNoiseMin =
    OiCalData.byNoiseMin =
    OiCalData.bwheelNoiseMin =
    OiCalData.bauxNoiseMin = 255;
    OiCalData.bxNoiseMax =
    OiCalData.byNoiseMax =
    OiCalData.bwheelNoiseMax =
    OiCalData.bauxNoiseMax = 0;
    OiCalData.bCalFlags = CALF_CALIBRATING;

    return;
}       //InputCalibrate

/// <summary>
///     This function processes the input data from the Operator Interface.
///     If we are in calibration mode, it will perform the calibration cycle.
///     Otherwise, it will adjust the input data according to the calibration
///     data.
/// </summary>
///
/// <returns> None. </returns>
///
/// <remarks>
///     After the Robot Controller is reset, the initialization routine should
///     initialize the InputData structure specifying which joystick ports are
///     in use and which analog channels of each port are in use. Then it
///     should set the OiCalData.bCalFlags to start the calibration
///     cycles.
/// </remarks>

void
InputProcessData(
    void
    )
{
    BYTE i;
    BYTE bPort;

    if (!(OiCalData.bCalFlags & CALF_CALIBRATING))
    {
        //
        // We are not in calibration mode.
        //
        for (i = 0; i < g_InputData.bNumPorts; ++i)
        {
            bPort = g_InputData.Joysticks[i].bPort;

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_X)
            {
                g_InputData.Joysticks[i].ix =
                    (int)(*xPorts[bPort]) - g_InputData.Joysticks[i].ixBias;
                if (abs(g_InputData.Joysticks[i].ix) <
                    g_InputData.Joysticks[i].ixNoiseThreshold)
                {
                    g_InputData.Joysticks[i].ix = 0;
                }
                else if (g_InputData.Joysticks[i].ix < -127)
                {
                    g_InputData.Joysticks[i].ix = -127;
                }
                else if (g_InputData.Joysticks[i].ix > 127)
                {
                    g_InputData.Joysticks[i].ix = 127;
                }
            }

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_Y)
            {
                g_InputData.Joysticks[i].iy =
                    (int)(*yPorts[bPort]) - g_InputData.Joysticks[i].iyBias;
                if (abs(g_InputData.Joysticks[i].iy) <
                    g_InputData.Joysticks[i].iyNoiseThreshold)
                {
                    g_InputData.Joysticks[i].iy = 0;
                }
                else if (g_InputData.Joysticks[i].iy < -127)
                {
                    g_InputData.Joysticks[i].iy = -127;
                }
                else if (g_InputData.Joysticks[i].iy > 127)
                {
                    g_InputData.Joysticks[i].iy = 127;
                }
            }

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_WHEEL)
            {
                g_InputData.Joysticks[i].iwheel =
                    (int)(*wheelPorts[bPort]) -
                    g_InputData.Joysticks[i].iwheelBias;
                if (abs(g_InputData.Joysticks[i].iwheel) <
                    g_InputData.Joysticks[i].iwheelNoiseThreshold)
                {
                    g_InputData.Joysticks[i].iwheel = 0;
                }
                else if (g_InputData.Joysticks[i].iwheel < -127)
                {
                    g_InputData.Joysticks[i].iwheel = -127;
                }
                else if (g_InputData.Joysticks[i].iwheel > 127)
                {
                    g_InputData.Joysticks[i].iwheel = 127;
                }
            }

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_AUX)
            {
                g_InputData.Joysticks[i].iaux =
                    (int)(*auxPorts[bPort]) -
                    g_InputData.Joysticks[i].iauxBias;
                if (abs(g_InputData.Joysticks[i].iaux) <
                    g_InputData.Joysticks[i].iauxNoiseThreshold)
                {
                    g_InputData.Joysticks[i].iaux = 0;
                }
                else if (g_InputData.Joysticks[i].iaux < -127)
                {
                    g_InputData.Joysticks[i].iaux = -127;
                }
                else if (g_InputData.Joysticks[i].iaux > 127)
                {
                    g_InputData.Joysticks[i].iaux = 127;
                }
            }
        }
    }
    else
    {
        //
        // We are in calibration mode and we are calibrating joystick entry
        // OiCalData.bCalPortIndex.
        //
        i = OiCalData.bCalPortIndex;
        bPort = g_InputData.Joysticks[i].bPort;

        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_X)
        {
            OiCalData.lxAccum += (long)(*xPorts[bPort]);
            if (*xPorts[bPort] < OiCalData.bxNoiseMin)
            {
                OiCalData.bxNoiseMin = *xPorts[bPort];
            }
            if (*xPorts[bPort] > OiCalData.bxNoiseMax)
            {
                OiCalData.bxNoiseMax = *xPorts[bPort];
            }
        }

        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_Y)
        {
            OiCalData.lyAccum += (long)(*yPorts[bPort]);
            if (*yPorts[bPort] < OiCalData.byNoiseMin)
            {
                OiCalData.byNoiseMin = *yPorts[bPort];
            }
            if (*yPorts[bPort] > OiCalData.byNoiseMax)
            {
                OiCalData.byNoiseMax = *yPorts[bPort];
            }
        }

        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_WHEEL)
        {
            OiCalData.lwheelAccum += (long)(*wheelPorts[bPort]);
            if (*wheelPorts[bPort] < OiCalData.bwheelNoiseMin)
            {
                OiCalData.bwheelNoiseMin = *wheelPorts[bPort];
            }
            if (*wheelPorts[bPort] > OiCalData.bwheelNoiseMax)
            {
                OiCalData.bwheelNoiseMax = *wheelPorts[bPort];
            }
        }

        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_AUX)
        {
            OiCalData.lauxAccum += (long)(*auxPorts[bPort]);
            if (*auxPorts[bPort] < OiCalData.bauxNoiseMin)
            {
                OiCalData.bauxNoiseMin = *auxPorts[bPort];
            }
            if (*auxPorts[bPort] > OiCalData.bauxNoiseMax)
            {
                OiCalData.bauxNoiseMax = *auxPorts[bPort];
            }
        }

        OiCalData.icCalSamples++;
        if (OiCalData.icCalSamples >= MAX_INPUTCAL_SAMPLES)
        {
            printf("Joystick[%d]:", bPort);
            //
            // We are done calibrating this port, move to the next one.
            //
            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_X)
            {
                g_InputData.Joysticks[i].ixBias = (int)
                    (OiCalData.lxAccum/OiCalData.icCalSamples);
                g_InputData.Joysticks[i].ixNoiseThreshold =
                    (OiCalData.bxNoiseMax - OiCalData.bxNoiseMin)/2 +
                    JOYSTICK_NOISE_MARGIN;
                printf(" XBias=%d,XNoiseThreshold=%d",
                       g_InputData.Joysticks[i].ixBias,
                       g_InputData.Joysticks[i].ixNoiseThreshold);
            }

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_Y)
            {
                g_InputData.Joysticks[i].iyBias = (int)
                    (OiCalData.lyAccum/OiCalData.icCalSamples);
                g_InputData.Joysticks[i].iyNoiseThreshold =
                    (OiCalData.byNoiseMax - OiCalData.byNoiseMin)/2 +
                    JOYSTICK_NOISE_MARGIN;
                printf(" YBias=%d,YNoiseThreshold=%d",
                       g_InputData.Joysticks[i].iyBias,
                       g_InputData.Joysticks[i].iyNoiseThreshold);
            }

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_WHEEL)
            {
                g_InputData.Joysticks[i].iwheelBias = (int)
                    (OiCalData.lwheelAccum/OiCalData.icCalSamples);
                g_InputData.Joysticks[i].iwheelNoiseThreshold =
                    (OiCalData.bwheelNoiseMax - OiCalData.bwheelNoiseMin)/2 +
                    JOYSTICK_NOISE_MARGIN;
                printf(" WheelBias=%d,WheelNoiseThreshold=%d",
                       g_InputData.Joysticks[i].iwheelBias,
                       g_InputData.Joysticks[i].iwheelNoiseThreshold);
            }

            if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_AUX)
            {
                g_InputData.Joysticks[i].iauxBias = (int)
                    (OiCalData.lauxAccum/OiCalData.icCalSamples);
                g_InputData.Joysticks[i].iauxNoiseThreshold =
                    (OiCalData.bauxNoiseMax - OiCalData.bauxNoiseMin)/2 +
                    JOYSTICK_NOISE_MARGIN;
                printf(" AuxBias=%d,AuxNoiseThreshold=%d",
                       g_InputData.Joysticks[i].iauxBias,
                       g_InputData.Joysticks[i].iauxNoiseThreshold);
            }
            printf("\r\n");

            OiCalData.icCalSamples = 0;
            OiCalData.lxAccum =
            OiCalData.lyAccum =
            OiCalData.lwheelAccum =
            OiCalData.lauxAccum = 0;
            OiCalData.bxNoiseMin =
            OiCalData.byNoiseMin =
            OiCalData.bwheelNoiseMin =
            OiCalData.bauxNoiseMin = 255;
            OiCalData.bxNoiseMax =
            OiCalData.byNoiseMax =
            OiCalData.bwheelNoiseMax =
            OiCalData.bauxNoiseMax = 0;

            OiCalData.bCalPortIndex++;
            if (OiCalData.bCalPortIndex >= g_InputData.bNumPorts)
            {
                OiCalData.bCalFlags = CALF_CALIBRATED;
                printf("Input calibration is done.\r\n");
                InputPrintData();
            }
        }
    }

    return;
}       //InputGetData

/// <summary>
///     This function prints the input data to the debug console.
/// </summary>
///
/// <returns> None. </returns>

void
InputPrintData(
    void
    )
{
    BYTE i;

    for (i = 0; i < g_InputData.bNumPorts; ++i)
    {
        printf("Joystick[%d]:\r\n", g_InputData.Joysticks[i].bPort);
        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_X)
        {
            printf("\txBias=%03d,xNoiseThreshold=%03d,xRaw=%03d,x=%03d\r\n",
                   g_InputData.Joysticks[i].ixBias,
                   g_InputData.Joysticks[i].ixNoiseThreshold,
                   *xPorts[g_InputData.Joysticks[i].bPort],
                   g_InputData.Joysticks[i].ix);
        }
        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_Y)
        {
            printf("\tyBias=%03d,yNoiseThreshold=%03d,yRaw=%03d,y=%03d\r\n",
                   g_InputData.Joysticks[i].iyBias,
                   g_InputData.Joysticks[i].iyNoiseThreshold,
                   *yPorts[g_InputData.Joysticks[i].bPort],
                   g_InputData.Joysticks[i].iy);
        }
        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_WHEEL)
        {
            printf("\twBias=%03d,wNoiseThreshold=%03d,wRaw=%03d,w=%03d\r\n",
                   g_InputData.Joysticks[i].iwheelBias,
                   g_InputData.Joysticks[i].iwheelNoiseThreshold,
                   *wheelPorts[g_InputData.Joysticks[i].bPort],
                   g_InputData.Joysticks[i].iwheel);
        }
        if (g_InputData.Joysticks[i].bPortFlags & PORTF_ENABLE_AUX)
        {
            printf("\taBias=%03d,aNoiseThreshold=%03d,aRaw=%03d,a=%03d\r\n",
                   g_InputData.Joysticks[i].iauxBias,
                   g_InputData.Joysticks[i].iauxNoiseThreshold,
                   *auxPorts[g_InputData.Joysticks[i].bPort],
                   g_InputData.Joysticks[i].iaux);
        }
    }

    return;
}       //InputPrintData

/// <summary>
///     This function initializes the joystick port and prepare for
///     calibration. It specifies which analog channels of the joystick
///     port are in use.
/// </summary>
///
/// <param name="bPort">
///     Specifies the joystick port for calibration.
///     Note: port number is 0-based.
/// </param>
/// <param name="bPortFlags">
///     Specifies which analog channels of the joystick port should be
///     calibrated.
/// </param>
///
/// <returns> None. </returns>

static void
InitializePort(
    __in BYTE bPort,
    __in BYTE bPortFlags
    )
{
    if (g_InputData.bNumPorts < NUM_JOYSTICKS)
    {
        //
        // We still have room.
        //
        g_InputData.Joysticks[g_InputData.bNumPorts].bPort = bPort;
        g_InputData.Joysticks[g_InputData.bNumPorts].bPortFlags =
            (bPortFlags & PORTF_ENABLE_MASK);

        // If no calibration has been done yet, initialize calibration data.
        if (!(OiCalData.bCalFlags & CALF_CALIBRATED))
        {
            g_InputData.Joysticks[g_InputData.bNumPorts].ixBias =
            g_InputData.Joysticks[g_InputData.bNumPorts].iyBias =
            g_InputData.Joysticks[g_InputData.bNumPorts].iwheelBias =
            g_InputData.Joysticks[g_InputData.bNumPorts].iauxBias = 0;
            g_InputData.Joysticks[g_InputData.bNumPorts].ixNoiseThreshold =
            g_InputData.Joysticks[g_InputData.bNumPorts].iyNoiseThreshold =
            g_InputData.Joysticks[g_InputData.bNumPorts].iwheelNoiseThreshold =
            g_InputData.Joysticks[g_InputData.bNumPorts].iauxNoiseThreshold = 0;
        }
        g_InputData.bNumPorts++;
    }

    return;
}       //InitializePort
