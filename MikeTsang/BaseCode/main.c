#if 0
/// <module name="main.c" />
///
/// <summary>
///     This module contains the main program loop.
/// </summary>
///
/// <remarks>
///     Modified: Michael Tsang (mikets@hotmail.com)
///     Date:     03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#ifdef _USE_CONSOLE_
#include "console.h"
#endif

//
// Global data.
//
rx_data_record  rxdata;         //DO NOT CHANGE!
tx_data_record  txdata;         //DO NOT CHANGE!
packed_struct   statusflag;     //DO NOT CHANGE!

DWORD   g_dwCurrentTicks = 0;
DWORD   g_dwLostPackets = 0;
BYTE    g_bTraceInterval = DEF_TRACE_INTERVAL;
DWORD   g_dwSysFlags = 0;
DWORD   g_dwFastLoops = 0;

//
// Local data.
//
static PFNHOOK      PreProcessFuncs[MAX_PREPROCESS_HOOKS];
static PFNHOOK      PostProcessFuncs[MAX_POSTPROCESS_HOOKS];
static PFNHOOK      CalibrationFuncs[MAX_CALIBRATION_HOOKS];
static HOOKTABLE    PreProcessHooks = {PreProcessFuncs, MAX_PREPROCESS_HOOKS, 0};
static HOOKTABLE    PostProcessHooks = {PostProcessFuncs, MAX_POSTPROCESS_HOOKS, 0};
static HOOKTABLE    CalibrationHooks = {CalibrationFuncs, MAX_CALIBRATION_HOOKS, 0};

//
// Local function prototypes.
//
static void
InitHookTable(
    __in PHOOKTABLE HookTable
    );
    
static void
ProcessHooks(
    __in PHOOKTABLE HookTable
    );
    
static void
BaseInitialization(
    void
    );
    
/// <summary>
///     This is the program entry point.
/// </summary>
///
/// <returns> None. </returns>

void
main(
    void
    )
{
    BYTE bPrevPacketNum = 0;
    BYTE bDeltaPackets;

#ifdef UNCHANGEABLE_DEFINITION_AREA
    IFI_Initialization();               //DO NOT CHANGE!
#endif
    //
    // Base code initialization. This contains common initializations
    // that are not project specific.
    //
    BaseInitialization();
    //
    // Project specific initialization. Customizable function in project.c.
    //
    ProjInitialize();
    
    statusflag.NEW_SPI_DATA = 0;        //DO NOT CHANGE!
    //
    // This loop will repeat indefinitely.
    //
    while (1)
    {
#ifdef _SIMULATOR
        statusflag.NEW_SPI_DATA = 1;
#endif
        if (statusflag.NEW_SPI_DATA)
        {
            //
            // This block of code only execute every 26.2 ms because
            // that's how fast the Master uP gives us data.
            //
            
            // Get input data from Operator Interface.
            Getdata(&rxdata);
            
            // Determine if we have lost packets.
            bDeltaPackets = (BYTE)(rxdata.packet_num - bPrevPacketNum);
            bPrevPacketNum = rxdata.packet_num;
            g_dwLostPackets += bDeltaPackets - 1;

            ProcessHooks(&PreProcessHooks);
            
            if (!autonomous_mode)       //DO NOT CHANGE!
            {
                // Process input data from OI (e.g. apply calibration).
                InputProcessData();
                
                if (g_dwSysFlags & SYSF_CALIBRATION_MODE)
                {
                    ProcessHooks(&CalibrationHooks);
                    //
                    // Check if all the calibration hooks are done.
                    //
                    if (CalibrationHooks.bNumFuncs == 0)
                    {
                        g_dwSysFlags &= ~SYSF_CALIBRATION_MODE;
                    }
                }
                else
                {
                    //
                    // Customizable function in project.c.
                    //
                    ProjProcessData();
                }
            }
            else
            {
                //
                // Customizable function in project.c.
                //
                ProjAutonomousCode();
            }
            
            ProcessHooks(&PostProcessHooks);
        }
        //
        // Code below runs in a tight loop and is much fast.
        //
        // Customizable function in project.c.
        //
        g_dwFastLoops++;
        ProjProcessLocalIO();
    }
}       //main

/// <summary>
///     This function registers a pre or post process hook function.
///     Pre-process hook functions will be called before the ProjProcessData
///     is called. Post-proces hook functions will be called after
///     ProjProcessData is called.
/// </summary>
///
/// <param name="HookTable">
///     Points to the Hook table to add the registered processing function.
/// </param>
/// <param name="pfnHook">
///     Specifies the hook function to be registered.
/// </param>
///
/// <returns> Success: Returns TRUE. </returns>
/// <returns> Failure: Returns FALSE (e.g. table is full). </returns>

BOOL
RegisterHook(
    __inout PHOOKTABLE HookTable,
    __in PFNHOOK       pfnHook
    )
{
    BOOL rc = FALSE;
    BYTE i;
    
    for (i = 0; i < HookTable->bNumSlots; ++i)
    {
        if (HookTable->HookFuncs[i] == NULL)
        {
            //
            // This slot is empty.
            //
            HookTable->HookFuncs[i] = pfnHook;
            HookTable->bNumFuncs++;
            rc = TRUE;
            break;
        }
    }
    
    return rc;
}       //RegisterHook

/// <summary>
///     This function intializes the hook table.
/// </summary>
///
/// <param name="HookTable">
///     Points to the Hook table.
/// </param>
///
/// <returns> None. </returns>

static void
InitHookTable(
    __in PHOOKTABLE HookTable
    )
{
    BYTE i;
    
    for (i = 0; i < HookTable->bNumSlots; ++i)
    {
        HookTable->HookFuncs[i] = NULL;
        HookTable->bNumFuncs = 0;
    }
    
    return;
}       //InitHookTable

/// <summary>
///     This function goes through the hook table and calls each hook function.
/// </summary>
///
/// <param name="HookTable">
///     Points to the Hook table.
/// </param>
///
/// <returns> None. </returns>

static void
ProcessHooks(
    __in PHOOKTABLE HookTable
    )
{
    BYTE i;
    
    for (i = 0; i < HookTable->bNumSlots; ++i)
    {
        if (HookTable->HookFuncs[i] != NULL)
        {
            if (HookTable->HookFuncs[i]() == FALSE)
            {
                //
                // This hook is done, unregister it.
                //
                HookTable->HookFuncs[i] = NULL;
                HookTable->bNumFuncs--;
            }
        }
    }
    
    return;
}       //ProcessHooks

/// <summary>
///     This function performs base code initializations which are not project specific.
/// </summary>
///
/// <returns> None. </returns>

static void
BaseInitialization(
    void
    )
{
    BYTE i;
    
    g_dwCurrentTicks = 0;
    g_dwFastLoops = 0;
    g_dwLostPackets = 0;
    InitHookTable(&PreProcessHooks);
    InitHookTable(&PostProcessHooks);
    InitHookTable(&CalibrationHooks);
#ifdef _USE_CONSOLE_
    RegisterHook(&PreProcessHooks, ConsoleProcessData);
    RegisterHook(&PostProcessHooks, BaseCmdConsoleTrace);
#endif
    //
    // Initialize Timer 1 to generate a 26.2ms tick. This is used as the system clock tick
    // and can be used anywhere time keeping is needed.
    // 10MHz clock is feed through a programmable divisor to a 16-bit down counter.
    // When the counter reaches zero, an interrupt is generated.
    // 10MHz/8/32767=38.148Hz=26.2136ms
    //
    T1CON = 0x30;               //1:8 Prescale
    TMR1H = 0x7F;               //Pre-load TMR1 to overflow after 26.2ms
    TMR1L = 0xFF;
    T1CONbits.TMR1ON = 1;       //Enable timer
    IPR1bits.TMR1IP = 0;        //Set Timer1 Interrupt to low priority
    PIE1bits.TMR1IE = 1;        //Interrupt when Timer1 overflows
    INTCONbits.GIEL = 1;        //Enable Global Low Priority Interrupts
    
    return;
}       //BaseInitialization
