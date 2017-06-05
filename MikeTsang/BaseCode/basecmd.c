#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="basecmd.c" />
///
/// <summary>
///     This module contains the base console command functions.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   10-Mar-2008
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "console.h"

//
// Local function prototypes.
//
static void
PrintStatus(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintRCIoData(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintPwmData(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintAdcData(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintEncoderData(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintGyroData(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintInputData(
    __in PCMDENTRY pCmdEntry
    );

static void
CalibrateGyro(
    __in PCMDENTRY pCmdEntry
    );
    
static void
CalibrateInput(
    __in PCMDENTRY pCmdEntry
    );

//
// Local data.
//
static ARGDEF ArgTraceRCIo = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_RCIO, "Turn RC I/O tracing on and off."};
static ARGDEF ArgTracePwm = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_PWM, "Turn PWM tracing on and off."};
static ARGDEF ArgTraceAdc = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_ADC, "Turn ADC tracing on and off."};
static ARGDEF ArgTraceEncoder = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_ENCODER, "Turn Encoder tracing on and off."};
static ARGDEF ArgTraceGyro = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_GYRO, "Turn Gyro tracing on and off."};
static ARGDEF ArgTraceInput = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_INPUT, "Turn Input tracing on and off."};
static ARGDEF ArgTraceServo = {ARGTYPE_FLAGS, &g_dwSysFlags, SYSF_TRACE_SERVO, "Turn servo tracing on and off."};
static ARGDEF ArgTraceInterval = {ARGTYPE_NUM, &g_bTraceInterval, DATAF_CHAR, "Interval in system ticks."};
static CMDENTRY BaseCmdTable[] =
{
    //
    // Note: "Help" must be the first command in the table.
    //       The parser is expecting it.
    //
    {"?", 0, NULL, ConsolePrintHelp, "Print this help message."},
    {"q", 0, NULL, ConsoleQuit, "Exit console mode."},
    {"ps", 0, NULL, PrintStatus, "Print system status."},
    {"pr", 0, NULL, PrintRCIoData, "Print RC I/O data."},
    {"pp", 0, NULL, PrintPwmData, "Print PWM data."},
    {"pa", 0, NULL, PrintAdcData, "Print A/D converter data."},
    {"pn", 0, NULL, PrintEncoderData, "Print Encoder data."},
    {"pg", 0, NULL, PrintGyroData, "Print Gyro data."},
    {"pi", 0, NULL, PrintInputData, "Print Input data."},
    {"tr", 1, &ArgTraceRCIo, NULL, "Trace RC I/O data."},
    {"tp", 1, &ArgTracePwm, NULL, "Trace PWM data."},
    {"ta", 1, &ArgTraceAdc, NULL, "Trace A/D converter data."},
    {"tn", 1, &ArgTraceEncoder, NULL, "Trace Encoder data."},
    {"tg", 1, &ArgTraceGyro, NULL, "Trace Gyro data."},
    {"ti", 1, &ArgTraceInput, NULL, "Trace Input data."},
    {"tv", 1, &ArgTraceServo, NULL, "Trace Servo data."},
    {"cg", 0, NULL, CalibrateGyro, "Calibrate gyro."},
    {"ci", 0, NULL, CalibrateInput, "Calibrate joysticks."},
    {"st", 1, &ArgTraceInterval, NULL, "Set Trace Interval."}
};

/// <summary>
///     This function initialize the command console.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to use for the command console.
/// </param>
/// <param name="ProjCmdTable">
///     Points to the project specific console command table.
/// </param>
/// <param name="bNumCmds">
///     Specifies the size of the command table.
/// </param>
///
/// <returns> None. </returns>

void
BaseCmdConsoleInit(
    __in BYTE      bSerialPort,
    __in PCMDENTRY ProjCmdTable,
    __in BYTE      bNumCmds
    )
{
    ConsoleInitialize(bSerialPort,
                      BaseCmdTable, ARRAYSIZE(BaseCmdTable),
                      ProjCmdTable, bNumCmds);
    return;
}       //BaseCmdConsoleInit

/// <summary>
///     This function processes the periodic traces.
/// </summary>
///
/// <returns> Always returns TRUE so it will be called next time. </returns>

BOOL
BaseCmdConsoleTrace(
    void
    )
{
    if (g_dwSysFlags & SYSF_TRACE_ON)
    {
        if (g_dwSysFlags & SYSF_TRACE_RCIO)
        {
            RCIOPrintData();
        }
        
        if (g_dwSysFlags & SYSF_TRACE_PWM)
        {
            PWMPrintData();
        }
        
        if (g_dwSysFlags & SYSF_TRACE_ADC)
        {
            ADCPrintData(FALSE);
        }
        
        if (g_dwSysFlags & SYSF_TRACE_ENCODER)
        {
            EncoderPrintData(FALSE);
        }
        
        if (g_dwSysFlags & SYSF_TRACE_GYRO)
        {
            GyroPrintData();
        }
        
        if (g_dwSysFlags & SYSF_TRACE_INPUT)
        {
            InputPrintData();
        }
        
        ProjCmdConsoleTrace();
    }
    
    return TRUE;
}       //BaseCmdConsoleTrace

/// <summary>
///     This function prints system status info.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintStatus(
    __in PCMDENTRY pCmdEntry
    )
{
    printf("[%08ld] FastLoop/Tick=%d, SysFlags=%08lx, TraceInterval=%d\r\n",
           g_dwCurrentTicks, (unsigned int)(g_dwFastLoops/g_dwCurrentTicks),
           g_dwSysFlags, g_bTraceInterval);
    printf("           LostPackets=%ld\r\n", g_dwLostPackets);
    return;
}       //PrintStatus

/// <summary>
///     This function prints the RC I/O data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintRCIoData(
    __in PCMDENTRY pCmdEntry
    )
{
    RCIOPrintData();
    return;
}       //PrintRCIoData

/// <summary>
///     This function prints the PWM data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintPwmData(
    __in PCMDENTRY pCmdEntry
    )
{
    PWMPrintData();
    return;
}       //PrintPwmData

/// <summary>
///     This function prints the A/D converter data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintAdcData(
    __in PCMDENTRY pCmdEntry
    )
{
    ADCPrintData(TRUE);
    return;
}       //PrintAdcData

/// <summary>
///     This function prints the Encoder data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintEncoderData(
    __in PCMDENTRY pCmdEntry
    )
{
    EncoderPrintData(TRUE);
    return;
}       //PrintEncoderData

/// <summary>
///     This function prints the Gyro data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintGyroData(
    __in PCMDENTRY pCmdEntry
    )
{
    GyroPrintData();
    return;
}       //PrintGyroData

/// <summary>
///     This function prints the OI input data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintInputData(
    __in PCMDENTRY pCmdEntry
    )
{
    InputPrintData();
    return;
}       //PrintInputData

/// <summary>
///     This function initiates a gyro calibration.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
CalibrateGyro(
    __in PCMDENTRY pCmdEntry
    )
{
    GyroCalibrate();
    return;
}       //CalibrateGyro

/// <summary>
///     This function initiates an input calibration.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
CalibrateInput(
    __in PCMDENTRY pCmdEntry
    )
{
    InputCalibrate();
    return;
}       //CalibrateInput

