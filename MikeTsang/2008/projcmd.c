#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="projcmd.c" />
///
/// <summary>
///     This module contains the project specific console command functions.
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
PrintDrivePid(
    __in PCMDENTRY pCmdEntry
    );
    
static void
PrintServoData(
    __in PCMDENTRY pCmdEntry
    );

//
// Local data.
//
static ARGDEF ArgPidKp = {ARGTYPE_NUM, &g_DrivePid.Kp, DATAF_SHORT, "Constant Kp."};
static ARGDEF ArgPidKv = {ARGTYPE_NUM, &g_DrivePid.Kv, DATAF_SHORT, "Constant Kv."};
static ARGDEF ArgPidKi = {ARGTYPE_NUM, &g_DrivePid.Ki, DATAF_SHORT, "Constant Ki."};
static ARGDEF ArgPidKo = {ARGTYPE_NUM, &g_DrivePid.Ko, DATAF_SHORT, "Constant Ko."};
static CMDENTRY ProjCmdTable[] =
{
    {"pd", 0, NULL, PrintDrivePid, "Print Drive PID parameters."},
    {"pv", 0, NULL, PrintServoData, "Print Servo data."},
    {"sp", 1, &ArgPidKp, NULL, "Set PID constant Kp."},
    {"sv", 1, &ArgPidKv, NULL, "Set PID constant Kv."},
    {"si", 1, &ArgPidKi, NULL, "Set PID constant Ki."},
    {"so", 1, &ArgPidKo, NULL, "Set PID constant Ko."}
};

/// <summary>
///     This function initialize the command console.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to use for the command console.
/// </param>
///
/// <returns> None. </returns>

void
ProjCmdConsoleInit(
    __in BYTE bSerialPort
    )
{
    BaseCmdConsoleInit(bSerialPort, ProjCmdTable, ARRAYSIZE(ProjCmdTable));
    return;
}       //ProjCmdConsoleInit

/// <summary>
///     This function processes the project specific periodic traces.
/// </summary>
///
/// <returns> None. </returns>

void
ProjCmdConsoleTrace(
    void
    )
{
    if (g_dwSysFlags & SYSF_TRACE_ON)
    {
        if (g_dwProjFlags & PROJF_SERVO)
        {
            ServoPrintData(" Left", &g_LeftMotor);
            ServoPrintData("Right", &g_LeftMotor);
        }
    }
        
    return;
}       //ProjCmdConsoleTrace

/// <summary>
///     This function prints the PID control parameters.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintDrivePid(
    __in PCMDENTRY pCmdEntry
    )
{
    ServoPrintPidParam(&g_DrivePid);
    return;
}       //PrintDrivePid

/// <summary>
///     This function prints the servo motor data.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

static void
PrintServoData(
    __in PCMDENTRY pCmdEntry
    )
{
    ServoPrintData(" Left", &g_LeftMotor);
    ServoPrintData("Right", &g_RightMotor);
    return;
}       //PrintServoData

