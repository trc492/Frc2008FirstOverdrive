#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="console.c" />
///
/// <summary>
///     This module implements a debugger console.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   16-Feb-2008
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "serial_ports.h"
#include "console.h"

//
// Local constants.
//

/// <summary>
///     Specifies the maximum length of the console command buffer.
/// </summary>
#define CMD_BUFF_LEN                    64
/// <summary>
///     Specifies the maximum number of console command arguments.
/// </summary>
#define MAX_ARGS                        8

//
// Local type definitions.
//

/// <summary>
///     This structure stores information about the console serial port.
/// </summary>
typedef struct _Console
{
    /// <summary>
    ///     Stores the serial port number used by the console (0-based).
    /// </summary>
    BYTE        bSerialPort;
    /// <summary>
    ///     Stores the number of characters in the command buffer.
    /// </summary>
    BYTE        bCmdLen;
    /// <summary>
    ///     Command buffer.
    /// </summary>
    char        CmdBuff[CMD_BUFF_LEN];
    /// <summary>
    ///     Specifies the number of entries in the BaseCmdTable.
    /// </summary>
    BYTE        bNumBaseCmds;
    /// <summary>
    ///     Stores the pointer to the Base Command table.
    /// </summary>
    PCMDENTRY   BaseCmdTable;
    /// <summary>
    ///     Specifies the number of entries in the ProjCmdTable.
    /// </summary>
    BYTE        bNumProjCmds;
    /// <summary>
    ///     Stores the pointer to the Proj Command table.
    /// </summary>
    PCMDENTRY   ProjCmdTable;
} CONSOLE, *PCONSOLE;

//
// Local function prototypes.
//
static void
ConsoleParseCommand(
    __inout PSTR pszCmd,
    __in BYTE    bCmdLen
    );
    
static BOOL
ConsoleProcessCmd(
    __in PSTR      pszCmd,
    __in PCMDENTRY CmdTable,
    __in BYTE      bNumCmds,
    __in PSTR     *apszArgs,
    __in BYTE      bcArgs
    );
    
static BOOL
ConsoleParseCmdArgs(
    __in PARGDEF ArgDefs,
    __in PSTR   *apszArgs,
    __in BYTE    bcArgs
    );
    
static void
ConsolePrintCmdHelp(
    __in PCMDENTRY pCmdEntry
    );

static PSTR
CopyPgmString(
    __out PSTR           pszStr,
    __in BYTE            bLen,
    __in const rom char *pszPgmStr
    );
    
//
// Local data.
//
static CONSOLE Console = {SERIAL_PORT_ONE, 0};

/// <summary>
///     This function initializes the console using the given serial port.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies which serial port to get the console input from.
/// </param>
/// <param name="BaseCmdTable">
///     Points to the Base Command table.
/// </param>
/// <param name="bNumBaseCmds">
///     Specifies the number of command entries in the BaseCmdTable.
/// </param>
/// <param name="ProjCmdTable">
///     Points to the Proj Command table.
/// </param>
/// <param name="bNumProjCmds">
///     Specifies the number of command entries in the ProjCmdTable.
/// </param>
///
/// <returns> None. </returns>

void
ConsoleInitialize(
    __in BYTE      bSerialPort,
    __in PCMDENTRY BaseCmdTable,
    __in BYTE      bNumBaseCmds,
    __in PCMDENTRY ProjCmdTable,
    __in BYTE      bNumProjCmds
    )
{
    Console.bSerialPort = bSerialPort;
    Console.bCmdLen = 0;
    Console.bNumBaseCmds = bNumBaseCmds;
    Console.BaseCmdTable = BaseCmdTable;
    Console.bNumProjCmds = bNumProjCmds;
    Console.ProjCmdTable = ProjCmdTable;
    return;
}       //ConsoleIntitialize

/// <summary>
///     This function processes the serial data as console commands.
/// </summary>
///
/// <returns> Always returns TRUE so it will be called next time. </returns>

BOOL
ConsoleProcessData(
    void
    )
{
    BYTE c;
    static PSTR pszPrompt = "\r\nConsole-> ";

    while (SerialGetByteCount(Console.bSerialPort) > 0)
    {
        c = SerialReadData(Console.bSerialPort);
        if (g_dwSysFlags & SYSF_CONSOLE_MODE)
        {
            if ((c == '\r') || (c == '\n'))
            {
                if (Console.bCmdLen > 0)
                {
                    //
                    // We have a completed command line, go process it.
                    //
                    Console.CmdBuff[Console.bCmdLen] = '\0';
                    printf("\r\n");
                    ConsoleParseCommand(Console.CmdBuff, Console.bCmdLen);
                    Console.bCmdLen = 0;
                }
                
                if (g_dwSysFlags & SYSF_CONSOLE_MODE)
                {
                    //
                    // If we are still in console mode, display the prompt again.
                    //
                    printf(pszPrompt);
                }
            }
            else if (c == '\b')
            {
                //
                // We have a backspace, erase one character.
                //
                if (Console.bCmdLen > 0)
                {
                    Console.bCmdLen--;
                    SerialWriteData(Console.bSerialPort, c);
                    SerialWriteData(Console.bSerialPort, ' ');
                    SerialWriteData(Console.bSerialPort, c);
                }
                else
                {
                    SerialWriteData(Console.bSerialPort, '\a');
                }
            }
            else if ((c < ' ') || (c > '~') ||
                     (Console.bCmdLen >= CMD_BUFF_LEN - 1))
            {
                //
                // Invalid character, or we don't have enough space.
                //
                SerialWriteData(Console.bSerialPort, '\a');
            }
            else
            {
                //
                // Store the character into the command buffer.
                //
                SerialWriteData(Console.bSerialPort, c);
                Console.CmdBuff[Console.bCmdLen] = c;
                Console.bCmdLen++;
            }
        }
        else if (c == CMD_ATTENTION)
        {
            g_dwSysFlags |= SYSF_CONSOLE_MODE;
            Console.bCmdLen = 0;
            printf(pszPrompt);
        }
    }

    return TRUE;
}       //ConsoleProcessData

/// <summary>
///     This function prints the help message for all console commands.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

void
ConsolePrintHelp(
    __in PCMDENTRY pCmdEntry
    )
{
    BYTE i;

    printf("Commands:\r\n");
    for (i = 0; i < Console.bNumBaseCmds; ++i)
    {
        ConsolePrintCmdHelp(&Console.BaseCmdTable[i]);
    }
    for (i = 0; i < Console.bNumProjCmds; ++i)
    {
        ConsolePrintCmdHelp(&Console.ProjCmdTable[i]);
    }
    
    return;
}       //ConsolePrintHelp

/// <summary>
///     This function terminates console mode.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry (not used).
/// </param>
///
/// <returns> None. </returns>

void
ConsoleQuit(
    __in PCMDENTRY pCmdEntry
    )
{
    printf("Exit console!\r\n");
    g_dwSysFlags &= ~SYSF_CONSOLE_MODE;
    
    return;
}       //ConsoleQuit

/// <summary>
///     This function parses the console command.
/// </summary>
///
/// <param name="pszCmd">
///     Points to command buffer.
/// </param>
/// <param name="bCmdLen">
///     Specifies the length of the command in the buffer.
/// </param>
///
/// <returns> None. </returns>

static void
ConsoleParseCommand(
    __inout PSTR pszCmd,
    __in BYTE    bCmdLen
    )
{
    static char szSep[] = " \t";
    static PSTR apszArgs[MAX_ARGS];
    BYTE bcArgs = 0;
    PSTR psz;

    //
    // Parse command line into arguments.
    //
    psz = strtok(pszCmd, szSep);
    while ((psz != NULL) && (bcArgs < MAX_ARGS))
    {
        apszArgs[bcArgs] = psz;
        bcArgs++;
        psz = strtok(NULL, szSep);
    }
    
    if (bcArgs > 0)
    {
        BYTE i;
        static char szCmd[MAX_CMD_LEN];
        
        //
        // Convert the strings to lower case so that we can
        // do case insensitive compare.
        //
        strlwr(apszArgs[0]);
        if (!ConsoleProcessCmd(apszArgs[0],
                               Console.ProjCmdTable,
                               Console.bNumProjCmds,
                               &apszArgs[1],
                               bcArgs - 1))
        {
            if (!ConsoleProcessCmd(apszArgs[0],
                                   Console.BaseCmdTable,
                                   Console.bNumBaseCmds,
                                   &apszArgs[1],
                                   bcArgs - 1))
            {
                static char szCmd[MAX_CMD_LEN];
                //
                // Expecting "Help" is the first command in the base table.
                //
                CopyPgmString(szCmd, MAX_CMD_LEN, Console.BaseCmdTable[0].pszCmdString);
                printf("Error: Invalid command '%s'. Type '%s' for a list of valid commands.\r\n",
                       apszArgs[0], szCmd);
            }
        }
    }
    
    return;
}       //ConsoleParseCommand

/// <summary>
///     This function looks up the command and process it.
/// </summary>
///
/// <param name="pszCmd">
///     Points to command buffer.
/// </param>
/// <param name="CmdTable">
///     Points to the command table to lookup the command.
/// </param>
/// <param name="bNumCmds">
///     Specifies the size of the command table.
/// </param>
/// <param name="apszArgs">
///     Points to the command arguments.
/// </param>
/// <param name="bcArgs">
///     Specifies the number of command arguments.
/// </param>
///
/// <returns> None. </returns>

static BOOL
ConsoleProcessCmd(
    __in PSTR      pszCmd,
    __in PCMDENTRY CmdTable,
    __in BYTE      bNumCmds,
    __in PSTR     *apszArgs,
    __in BYTE      bcArgs
    )
{
    BOOL fFound = FALSE;
    BYTE i;
    static char szCmd[MAX_CMD_LEN];
    
    for (i = 0; i < bNumCmds; ++i)
    {
        CopyPgmString(szCmd, MAX_CMD_LEN, CmdTable[i].pszCmdString);
        strlwr(szCmd);
        if (strcmp(pszCmd, szCmd) == 0)
        {
            fFound = TRUE;
            if ((bcArgs  != CmdTable[i].bNumArgDefs) ||
                (bcArgs > 0) &&
                !ConsoleParseCmdArgs(CmdTable[i].ArgDefs,
                                     apszArgs,
                                     bcArgs))
            {
                printf("Error: Invalid syntax.\r\n");
                ConsolePrintCmdHelp(&CmdTable[i]);
            }
            else if (CmdTable[i].pfnCmd != NULL)
            {
                CmdTable[i].pfnCmd(&CmdTable[i]);
            }
            break;
        }
    }
    
    return fFound;
}       //ConsoleProcessCmd

/// <summary>
///     This function parses the console command arguments.
/// </summary>
///
/// <param name="ArgDefs">
///     Points to command buffer.
/// </param>
/// <param name="apszArgs">
///     Points to the argument array.
/// </param>
/// <param name="bcArgs">
///     Specifies the number of arguments.
/// </param>
///
/// <returns> Success: Returns TRUE. </returns>
/// <returns> Failure: Returns FALSE. </returns>

static BOOL
ConsoleParseCmdArgs(
    __in PARGDEF ArgDefs,
    __in PSTR   *apszArgs,
    __in BYTE    bcArgs
    )
{
    BOOL rc = TRUE;
    static char szOn[] = "on";
    static char szOff[] = "off";
    
    while (rc && (bcArgs > 0))
    {
        switch (ArgDefs->bType)
        {
            case ARGTYPE_STRING:
                *((PSTR *)ArgDefs->pvData) = *apszArgs;
                bcArgs--;
                apszArgs++;
                break;
            
            case ARGTYPE_NUM:
                if (ArgDefs->bParam == DATAF_CHAR)
                {
                    *((char *)ArgDefs->pvData) = atob(*apszArgs);
                }
                else if (ArgDefs->bParam == DATAF_SHORT)
                {
                    *((int *)ArgDefs->pvData) = atoi(*apszArgs);
                }
                else if (ArgDefs->bParam == DATAF_LONG)
                {
                    *((long *)ArgDefs->pvData) = atol(*apszArgs);
                }
                else
                {
                    printf("Assert: Invalid argument parameter %x.",
                           ArgDefs->bParam);
                    rc = FALSE;
                }
                bcArgs--;
                apszArgs++;
                break;
            
            case ARGTYPE_FLAGS:
                strlwr(*apszArgs);
                if (strcmp(*apszArgs, szOn) == 0)
                {
                    *((char *)ArgDefs->pvData) |= ArgDefs->bParam;
                }
                else if (strcmp(*apszArgs, szOff) == 0)
                {
                    *((char *)ArgDefs->pvData) &= ~ArgDefs->bParam;
                }
                else
                {
                    printf("Error: Invalid command syntax.");
                    rc = FALSE;
                }
                bcArgs--;
                apszArgs++;
                break;
                
            case ARGTYPE_CUSTOM:
                if (ArgDefs->pvData != NULL)
                {
                    //
                    // The callback function will parse a number of arguments.
                    // When it is done, it will update apszArgs and bcArgs
                    // to reflect the number of arguments consumed.
                    //
                    rc = ((PFNPARSEARG)ArgDefs->pvData)(ArgDefs, &apszArgs, &bcArgs);
                }
                break;
            
            default:
                printf("Assert: Invalid argument type %x.",
                       ArgDefs->bType);
                rc = FALSE;
            break;
        }
        ArgDefs++;
    }
    
    return rc;
}       //ConsoleParseCmdArgs

/// <summary>
///     This function prints the help message for the console command.
/// </summary>
///
/// <param name="pCmdEntry">
///     Points to the command table entry.
/// </param>
///
/// <returns> None. </returns>

static void
ConsolePrintCmdHelp(
    __in PCMDENTRY pCmdEntry
    )
{
    BYTE i;
    static char szCmd[MAX_CMD_LEN];
    static char szHelp[MAX_HELPMSG_LEN];
    
    CopyPgmString(szCmd, MAX_CMD_LEN, pCmdEntry->pszCmdString);
    printf("%s", szCmd);
    for (i = 0; i < pCmdEntry->bNumArgDefs; ++i)
    {
        switch (pCmdEntry->ArgDefs[i].bType)
        {
            case ARGTYPE_STRING:
                printf(" <string>");
                break;
                
            case ARGTYPE_NUM:
                printf(" <num>");
                break;
                
            case ARGTYPE_FLAGS:
                printf(" on|off");
                break;
        }
    }

    if (pCmdEntry->pszCmdHelp)
    {
        CopyPgmString(szHelp, MAX_HELPMSG_LEN, pCmdEntry->pszCmdHelp);
        printf(" - %s\r\n", szHelp);
    }
    else
    {
        printf("\r\n");
    }
    
    for (i = 0; i < pCmdEntry->bNumArgDefs; ++i)
    {
        if (pCmdEntry->ArgDefs[i].pszArgHelp != NULL)
        {
            CopyPgmString(szHelp, MAX_HELPMSG_LEN, pCmdEntry->ArgDefs[i].pszArgHelp);
        }
        else
        {
            szHelp[0] = '\0';
        }
        printf("  %d: %s\r\n", i + 1, szHelp);
    }
    
    return;
}       //ConsolePrintCmdHelp

/// <summary>
///     This function copies a string from program memory to data memory.
///     String literals are allocated in program memory. A lot of library
///     functions can only access data memory.
/// </summary>
///
/// <param name="pszStr">
///     Points to string buffer in data memory.
/// </param>
/// <param name="bLen">
///     Specifies the size of the string buffer.
/// </param>
/// <param name="pszPgmStr">
///     Points to string in program memory.
/// </param>
///
/// <returns> Returns a copy of the string in data memory. </returns>

static PSTR
CopyPgmString(
    __out PSTR           pszStr,
    __in BYTE            bLen,
    __in const rom char *pszPgmStr
    )
{
    strncpypgm2ram(pszStr, pszPgmStr, bLen - 1);
    pszStr[bLen - 1] = '\0';
    
    return pszStr; 
}       //CopyPgmString
