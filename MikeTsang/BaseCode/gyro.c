#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="gyro.c" />
///
/// <summary>
///     This module contains all gyro functions.
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
///	21-Nov-2004 0.1 RKW - Original code.
///	12-Jan-2005 0.2 RKW - Altered Get_Gyro_Rate() and Get_Gyro_Angle() to
///                     use long integers for internal calculations, allowing
///                     larger numerators and denominators in the
///                     GYRO_RATE_SCALE_FACTOR and GYRO_ANGLE_SCALE_FACTOR
///                     #defines.
///	12-Jan-2005 0.2 RKW - GYRO_RATE_SCALE_FACTOR and GYRO_ANGLE_SCALE_FACTOR
///	                #defines added for Analog Devices' ADXRS401, ADXRS150
///                     and ADXRS300 gyros.
///	16-Jan-2005 0.3 RKW - Using preprocessor directives, added the ability
///	                to select the gyro type, angular units, sample rate and
///	                number of averaged samples per update.
///	21-Jan-2005 0.3 RKW - Added scaling factors for the BEI GyroChip.
///	30-Jan-2005 0.4 RKW - Revised the way bias calculations are done.
///	                Instead of using only one data set as a bias, multiple
///	                sample sets can now be averaged over a much longer
///                     period of time to derive the gyro bias. Updated
///                     documentation.
///	04-Sep-2005 0.5 RKW - Significant overhaul of gyro code to strip-out
///                     ADC-specific code and use new adc.c/.h interface.
///                     Added deadband option.
///	21-Nov-2005 0.5 RKW - Added support for Murata's ENV-05D gyro.
///	10-Jan-2006 0.5 RKW - Verified code works on PIC18F8722.
///     03-Feb-2008 0.5 MTS - Made the module more object oriented.
/// </remarks>
#endif

#include "basecode.h"
#include "adc.h"
#include "gyro.h"

//
// Local constants.
//

/// <summary>
///     Specifies the duration of gyro calibration in system ticks.
/// </summary>
#define GYRO_CAL_TICKS          300

// Gyro bFlags values.
/// <summary>
///     This flag indicates the gyro sensor has been calibrated.
/// </summary>
#define GYROF_CALIBRATED        0x01
/// <summary>
///     This flag indicates the gyro sensor is being calibrated.
/// </summary>
#define GYROF_CALIBRATING       0x80

//
// Local type definitions.
//

/// <summary>
///     This structure stores the states of the gyro sensor.
/// </summary>
typedef struct _Gyro
{
    /// <summary>
    ///     Stores flag values (see GYROF_* flags).
    /// </summary>
    BYTE    bFlags;
    /// <summary>
    ///     Specifies the ADC channel used by the gyro sensor.
    /// </summary>
    BYTE    bADCChannel;
    /// <summary>
    ///     Stores the gyro sensitivity value in unit of 0.1 degree/sec/volt.
    /// </summary>
    WORD    wSensitivity;
    /// <summary>
    ///     For optimum performance, you'll need to calibrate the scaling
    ///     factor to match that of your gyro's. One way to calibrate your
    ///     gyro is to mount it very securely to a hefty, square or
    ///     rectangular object. Mounting the gyro to a hefty object will
    ///     help dampen higher frequency vibrations that can adversly effect
    ///     your measurements. Place the mounted gyro against another
    ///     square object and start the included demonstration software.
    ///     To get good results, the mount must be absolutely still during
    ///     calibration. If the angle drifts rapidly while the mounted gyro
    ///     is motonless, you need to recalibrate. Again, gyros are
    ///     very sensitive and must be still during calibration. Once
    ///     the gyro is running with little drift, rotate the mount 180
    ///     degrees and note the reported angle. Ideally, reported angle
    ///     should be 1800. For every tenth of a percent that the angle
    ///     is high, decrease wScaleFactor by one. Conversly, for every
    ///     tenth of a percent low, increase wScaleFactor by one. Repeat
    ///     until you're satisfied with the accuracy.
    /// </summary>
    WORD    wScaleFactor;
    /// <summary>
    ///     Stores the noise margin value.
    /// </summary>
    WORD    wNoiseMargin;
    /// <summary>
    ///     Stores the range of the A/D converter used.
    /// </summary>
    WORD    wADCValueRange;
    /// <summary>
    ///     Stores the A/D update rate.
    /// </summary>
    WORD    wADCUpdateRate;
    /// <summary>
    ///     Stores the calibrated bias value.
    /// </summary>
    WORD    wBias;
    /// <summary>
    ///     Stores the calibrated noise threshold.
    /// </summary>
    WORD    wNoiseThreshold;
    /// <summary>
    ///     Stores the gyro rate value.
    /// </summary>
    int     iRate;
    /// <summary>
    ///     Stores the gyro angle value.
    /// </summary>
    long    lAngle;
} GYRO, *PGYRO;

/// <summary>
///     This structure stores the calibration states of the gyro sensor.
/// </summary>
typedef struct _GyroCal
{
    /// <summary>
    ///     Stores the minimum noise value.
    /// </summary>
    WORD    wNoiseMinValue;
    /// <summary>
    ///     Stores the maximum noise value.
    /// </summary>
    WORD    wNoiseMaxValue;
    /// <summary>
    ///     Stores the system time stamp of the previous sample.
    /// </summary>
    DWORD   dwPrevTicks;
    /// <summary>
    ///     Stores the number of system ticks left for exiting calibration.
    /// </summary>
    WORD    wCalTicksLeft;
    /// <summary>
    ///     Stores the accumulated value for averaging the bias.
    /// </summary>
    DWORD   dwAccum;
    /// <summary>
    ///     Stores the number of samples taken for calibration.
    /// </summary>
    WORD    wSamples;
} GYROCAL, *PGYROCAL;

//
// Local data.
//
static GYRO    Gyro = {0};
static GYROCAL GyroCal = {0};

/// <summary>
///     This function initializes the gyro component.
/// </summary>
///
/// <param name="bADCChannel">
///     Specifies the ADC channel the gyro is hooked up to.
/// </param>
/// <param name="wSensitivity">
///     Specifies the gyro sensitivity in the unit of 0.1 degree/sec/volt or
///     milliradians/sec/volt. The unit of sensitivity affects the unit
///     returned by GyroGetRate.
/// </param>
/// <param name="wScaleFactor">
///     Specifies the gyro scale factory. This is a manual calibration
///     constant. By default it starts with a value of 1000.
/// </param>
/// <param name="wNoiseMargin">
///     Specifies the noise margin on top of the calibrated noise threshold.
/// </param>
///
/// <returns> None. </returns>

void
GyroInitialize(
    __in BYTE bADCChannel,
    __in WORD wSensitivity,
    __in WORD wScaleFactor,
    __in WORD wNoiseMargin
    )
{
    Gyro.bADCChannel = bADCChannel;
    Gyro.wSensitivity = wSensitivity;
    Gyro.wScaleFactor = wScaleFactor;
    Gyro.wNoiseMargin = wNoiseMargin;
    Gyro.wADCValueRange = ADCGetValueRange();
    Gyro.wADCUpdateRate = ADCGetUpdateRate();
    Gyro.iRate = 0;
    Gyro.lAngle = 0;
    if (!(Gyro.bFlags & GYROF_CALIBRATED))
    {
        //
        // The gyro hasn't been calibrated before, let's calibrate it.
        // Make sure the robot is absolutely still and free of virbation
        // (i.e. compressor is off).
        //
        GyroCalibrate();
    }

    return;
}       //GyroInitialize

/// <summary>
///     This function initialize the gyro for calibration.
/// </summary>
///
/// <returns> None. </returns>
///
/// <remarks>
///     IMPORTANT: Only call this function when your robot will be
///     absolutely still and free of vibration (e.g. the air compressor
///     is off) until the call to GyroStopCal is made.
/// </remarks>

void
GyroCalibrate(
    void
    )
{
    if (!(Gyro.bFlags & GYROF_CALIBRATING))
    {
        printf("\r\nStarting gyro calibration, please do not move the robot until done...\r\n");
        GyroCal.dwAccum = 0;
        GyroCal.wSamples = 0;
        GyroCal.wNoiseMinValue = 0xffff;
        GyroCal.wNoiseMaxValue = 0;
        GyroCal.wCalTicksLeft = GYRO_CAL_TICKS;
        GyroCal.dwPrevTicks = g_dwCurrentTicks;
        Gyro.wBias = 0;
        Gyro.wNoiseThreshold = 0;
        Gyro.bFlags |= GYROF_CALIBRATING;
    }

    return;
}       //GyroCalibrate

/// <summary>
///     This function returns the current angular rate of change of the
///     given gyro in unit of 0.1 deg/sec or milliradians/sec depending
///     on the gyro sensitivity unit passed to the GyroInitialize function.
/// </summary>
///
/// <returns> Returns the gyro rate. </returns>

int
GyroGetRate(
    void
    )
{
    return (int)((((long)Gyro.iRate*Gyro.wSensitivity*5L)/Gyro.wADCValueRange)
                 *Gyro.wScaleFactor/1000);
}       //GyroGetRate

/// <summary>
///     This function returns the current heading angle of the given gyro
///     in unit of 0.1 degrees or milliradians depending on the gyro sensitivity
///     unit passed to the GyroInitialize function.
/// </summary>
///
/// <returns> Returns the gyro heading. </returns>

long
GyroGetAngle(
    void
    )
{
    return (((Gyro.lAngle*Gyro.wSensitivity*5L)/
             (Gyro.wADCValueRange*Gyro.wADCUpdateRate))*
            Gyro.wScaleFactor/1000);
}       //GyroGetRate

/// <summary>
///     This function returns the calibrated gyro bias.
/// </summary>
///
/// <returns> Returns the gyro bias. </returns>

WORD
GyroGetBias(
    void
    )
{
    return Gyro.wBias;
}       //GyroGetBias

/// <summary>
///     This function manually sets the gyro bias.
/// </summary>
///
/// <param name="wBias">
///     Specifies the new gyro bias.
/// </param>
///
/// <returns> None. </returns>

void
GyroSetBias(
    __in WORD wBias
    )
{
    Gyro.wBias = wBias;
    return;
}       //GyroSetBias

/// <summary>
///     This function resets the gyro angle.
/// </summary>
///
/// <returns> None. </returns>

void
GyroResetAngle(
    void
    )
{
    Gyro.lAngle = 0L;
    return;
}       //GyroResetAngle

/// <summary>
///     This function prints the gyro states.
/// </summary>
///
/// <returns> None. </returns>

void
GyroPrintData(
    void
    )
{
    printf("Gyro: ADC=%d, Sensitivity=%d, ScaleFactor=%d, NoiseMargin=%d\r\n",
           Gyro.bADCChannel, Gyro.wSensitivity, Gyro.wScaleFactor, Gyro.wNoiseMargin);
    printf("      Bias=%d, NoiseThreshold=%d, Rate=%d, Angle=%ld\r\n",
           Gyro.wBias, Gyro.wNoiseThreshold, GyroGetRate(), GyroGetAngle());
    return;
}       //GyroPrintData

/// <summary>
///     This function process the gyro data.
/// </summary>
///
/// <returns> None. </returns>

void
GyroProcessData(
    void
    )
{
    if (Gyro.bFlags & GYROF_CALIBRATING)
    {
        WORD wElapsedTicks = (WORD)(g_dwCurrentTicks - GyroCal.dwPrevTicks);

        GyroCal.dwPrevTicks = g_dwCurrentTicks;
        if (GyroCal.wCalTicksLeft > wElapsedTicks)
        {
            WORD wValue = ADCGetValue(Gyro.bADCChannel);

            GyroCal.wCalTicksLeft -= wElapsedTicks;
            if (wValue < GyroCal.wNoiseMinValue)
            {
                GyroCal.wNoiseMinValue = wValue;
            }

            if (wValue > GyroCal.wNoiseMaxValue)
            {
                GyroCal.wNoiseMaxValue = wValue;
            }

            GyroCal.dwAccum += wValue;
            GyroCal.wSamples++;
        }
        else
        {
            printf("Gyro calibration is done.\r\n");
            Gyro.bFlags &= ~GYROF_CALIBRATING;
            Gyro.wBias = GyroCal.dwAccum/GyroCal.wSamples;
            Gyro.wNoiseThreshold = (GyroCal.wNoiseMaxValue -
                                    GyroCal.wNoiseMinValue)/2 +
                                   Gyro.wNoiseMargin;
            Gyro.bFlags |= GYROF_CALIBRATED;
        }
    }
    else
    {
        Gyro.iRate = ADCGetValue(Gyro.bADCChannel) - Gyro.wBias;
        //
        // Update reported gyro rate and angle only if measured gyro rate
        // lies outside of the noise band.
        //
        if (abs(Gyro.iRate) > Gyro.wNoiseThreshold)
        {
            Gyro.lAngle += (long)Gyro.iRate;
        }
        else
        {
            Gyro.iRate = 0;
        }
    }

    return;
}       //GyroProcessData

