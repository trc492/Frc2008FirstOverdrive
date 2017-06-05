#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="console.h" />
///
/// <summary>
///     This module contains the definitions of the debugger console.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   16-Feb-2008
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _console_h
#define _console_h

//
// Exported constants.
//

#define MAX_CMD_LEN                     16
#define MAX_HELPMSG_LEN                 64

/// <summary>
///     This console key will enter the console mode.
/// </summary>
#define CMD_ATTENTION                   '\r'

/// <summary>
///     Specifies the argument is a string.
/// </summary>
#define ARGTYPE_STRING                  1
/// <summary>
///     Specifies the argument is numeric value.
/// </summary>
#define ARGTYPE_NUM                     2
/// <summary>
///     Specifies the argument is to enable or disable the given bits of a flag variable.
/// </summary>
#define ARGTYPE_FLAGS                   3
/// <summary>
///     Specifies the caller is handling the argument parsing.
/// </summary>
#define ARGTYPE_CUSTOM                  4

/// <summary>
///     The data flag specifies the data is an 8-bit byte.
/// </summary>
#define DATAF_CHAR                      0
/// <summary>
///     The data flag specifies the data is a 16-bit word.
/// </summary>
#define DATAF_SHORT                     1
/// <summary>
///     The data flag specifies the data is a 32-bit dword.
/// </summary>
#define DATAF_LONG                      2

//
// Exported type definitions.
//

/// <summary>
///     This function callback is called to process the command.
/// </summary>
typedef void (*PFNCMD)(struct _CmdEntry *CmdEntry);

/// <summary>
///     This function callback is called to parse custom arguments.
/// </summary>
typedef BOOL (*PFNPARSEARG)(struct _ArgDef *ArgDef, PSTR **papszArgs, BYTE *pbcArgs);

/// <summary>
///     This structure is used to describe a command argument.
/// </summary>
typedef struct _ArgDef
{
    /// <summary>
    ///     Specifies the argument type (see ARGTYPE_* flags for details).
    /// </summary>
    BYTE            bType;
    /// <summary>
    ///     Points to a variable to hold the parsed data.
    /// <para>
    ///       ARGTYPE_STRING: pvData points to a string pointer.
    /// </para>
    /// <para>
    ///       ARGTYPE_NUM: pvData points to a variable to hold the numeric value.
    /// </para>
    /// <para>
    ///       ARGTYPE_FLAGS: pvData points to a variable containing the flag bits.
    /// </para>
    /// <para>
    ///       ARGTYPE_CUSTOM: pvData points to the function for parsing the arguments.
    /// </para>
    /// </summary>
    void           *pvData;
    /// <summary>
    ///     Specifies the parameter for parsing the argument.
    /// <para>
    ///       ARGTYPE_STRING: none.
    /// </para>
    /// <para>
    ///       ARGTYPE_NUM: specifies the size of the value variable.
    /// </para>
    /// <para>
    ///       ARGTYPE_FLAGS: specifies the bit mask of the flag variable.
    /// </para>
    /// <para>
    ///       ARGTYPE_CUSTOM: none.
    /// </para>
    /// </summary>
    BYTE            bParam;
    /// <summary>
    ///     Points to the help message for the argument.
    /// </summary>
    const rom char *pszArgHelp;
} ARGDEF, *PARGDEF;
    
/// <summary>
///     This structure is used to describe a command.
/// </summary>
typedef struct _CmdEntry
{
    /// <summary>
    ///     Specifies the command string.
    /// </summary>
    const rom char *pszCmdString;
    /// <summary>
    ///     Specifies the number of arguments for the command.
    /// </summary>
    BYTE            bNumArgDefs;
    /// <summary>
    ///     Points to the table of argument definitions.
    /// </summary>
    PARGDEF         ArgDefs;
    /// <summary>
    ///     Points to callback function to process the command.
    /// </summary>
    PFNCMD          pfnCmd;
    /// <summary>
    ///     Points to the help message for the command.
    /// </summary>
    const rom char *pszCmdHelp;
} CMDENTRY, *PCMDENTRY;

//
// Exported function prototypes.
//
void
ConsoleInitialize(
    __in BYTE      bSerialPort,
    __in PCMDENTRY BaseCmdTable,
    __in BYTE      bNumBaseCmds,
    __in PCMDENTRY ProjCmdTable,
    __in BYTE      bNumProjCmds
    );

BOOL
ConsoleProcessData(
    void
    );

void
ConsolePrintHelp(
    __in PCMDENTRY pCmdEntry
    );
    
void
ConsoleQuit(
    __in PCMDENTRY pCmdEntry
    );

// The following functions are in basecmd.c.
void
BaseCmdConsoleInit(
    __in BYTE      bSerialPort,
    __in PCMDENTRY ProjCmdTable,
    __in BYTE      bNumCmds
    );

BOOL
BaseCmdConsoleTrace(
    void
    );
    
#endif
