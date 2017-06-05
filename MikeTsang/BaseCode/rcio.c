#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="rcio.c" />
///
/// <summary>
///     This module contains all RC I/O functions such as digital I/O
///     and relays.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   11-Mar-2008
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "rcio.h"

//
// Local function prototypes.
//
static void
RCIOPrintDigitalIo(
    __in BYTE bIoChannel,
    __in BYTE bIoConfig,
    __in BYTE bInValue,
    __in BYTE bOutValue
    );

static void
RCIOPrintRelay(
    __in BYTE bRelayChannel,
    __in BYTE bFwd,
    __in BYTE bRev
    );
    
/// <summary>
///     This function configures the RC I/O such as digital I/O and relays.
/// </summary>
///
/// <param name="dwOutChannelMask">
///     Specifies the bit mask of the digital out channels.
///     Note: The caller should OR the RC_DIGITAL_* values for this parameter.
/// </param>
///
/// <returns> None. </returns>

void
RCIOInitialize(
    __in DWORD dwOutChannelMask
    )
{
    BYTE i;
    BOOL bIsOut;
    
    for (i = 0; i < MAX_RC_DIGITAL_IOS; ++i)
    {
        bIsOut = (dwOutChannelMask & (1 << i)) != 0;
        switch (i)
        {
            case 0:
                if (bIsOut)
                {
                    digital_io_01 = OUTPUT;
                    rc_dig_out01 = 0;
                }
                else
                {
                    digital_io_01 = INPUT;
                }
                break;
                
            case 1:
                if (bIsOut)
                {
                    digital_io_02 = OUTPUT;
                    rc_dig_out02 = 0;
                }
                else
                {
                    digital_io_02 = INPUT;
                }
                break;
                
            case 2:
                if (bIsOut)
                {
                    digital_io_03 = OUTPUT;
                    rc_dig_out03 = 0;
                }
                else
                {
                    digital_io_03 = INPUT;
                }
                break;
                
            case 3:
                if (bIsOut)
                {
                    digital_io_04 = OUTPUT;
                    rc_dig_out04 = 0;
                }
                else
                {
                    digital_io_04 = INPUT;
                }
                break;
                
            case 4:
                if (bIsOut)
                {
                    digital_io_05 = OUTPUT;
                    rc_dig_out05 = 0;
                }
                else
                {
                    digital_io_05 = INPUT;
                }
                break;
                
            case 5:
                if (bIsOut)
                {
                    digital_io_06 = OUTPUT;
                    rc_dig_out06 = 0;
                }
                else
                {
                    digital_io_06 = INPUT;
                }
                break;
                
            case 6:
                if (bIsOut)
                {
                    digital_io_07 = OUTPUT;
                    rc_dig_out07 = 0;
                }
                else
                {
                    digital_io_07 = INPUT;
                }
                break;
                
            case 7:
                if (bIsOut)
                {
                    digital_io_08 = OUTPUT;
                    rc_dig_out08 = 0;
                }
                else
                {
                    digital_io_08 = INPUT;
                }
                break;
                
            case 8:
                if (bIsOut)
                {
                    digital_io_09 = OUTPUT;
                    rc_dig_out09 = 0;
                }
                else
                {
                    digital_io_09 = INPUT;
                }
                break;
                
            case 9:
                if (bIsOut)
                {
                    digital_io_10 = OUTPUT;
                    rc_dig_out10 = 0;
                }
                else
                {
                    digital_io_10 = INPUT;
                }
                break;
                
            case 10:
                if (bIsOut)
                {
                    digital_io_11 = OUTPUT;
                    rc_dig_out11 = 0;
                }
                else
                {
                    digital_io_11 = INPUT;
                }
                break;
                
            case 11:
                if (bIsOut)
                {
                    digital_io_12 = OUTPUT;
                    rc_dig_out12 = 0;
                }
                else
                {
                    digital_io_12 = INPUT;
                }
                break;
                
            case 12:
                if (bIsOut)
                {
                    digital_io_13 = OUTPUT;
                    rc_dig_out13 = 0;
                }
                else
                {
                    digital_io_13 = INPUT;
                }
                break;
                
            case 13:
                if (bIsOut)
                {
                    digital_io_14 = OUTPUT;
                    rc_dig_out14 = 0;
                }
                else
                {
                    digital_io_14 = INPUT;
                }
                break;
                
            case 14:
                if (bIsOut)
                {
                    digital_io_15 = OUTPUT;
                    rc_dig_out15 = 0;
                }
                else
                {
                    digital_io_15 = INPUT;
                }
                break;
                
            case 15:
                if (bIsOut)
                {
                    digital_io_16 = OUTPUT;
                    rc_dig_out16 = 0;
                }
                else
                {
                    digital_io_16 = INPUT;
                }
                break;
                
            case 16:
                if (bIsOut)
                {
                    digital_io_17 = OUTPUT;
                    rc_dig_out17 = 0;
                }
                else
                {
                    digital_io_17 = INPUT;
                }
                break;

            case 17:
                if (bIsOut)
                {
                    digital_io_18 = OUTPUT;
                    rc_dig_out18 = 0;
                }
                else
                {
                    digital_io_18 = INPUT;
                }
                break;
        }
    }
    
    relay1_fwd = relay1_rev = relay2_fwd = relay2_rev = 0;
    relay3_fwd = relay3_rev = relay4_fwd = relay4_rev = 0;
    relay5_fwd = relay5_rev = relay6_fwd = relay6_rev = 0;
    relay7_fwd = relay7_rev = relay8_fwd = relay8_rev = 0;
    
    return;
}       //RCIOInitialize

/// <summary>
///     This function prints the state of the RC I/O.
/// </summary>
///
/// <returns> None. </returns>

void
RCIOPrintData(
    void
    )
{
    RCIOPrintDigitalIo(1, digital_io_01, rc_dig_in01, rc_dig_out01);
    RCIOPrintDigitalIo(2, digital_io_02, rc_dig_in02, rc_dig_out02);
    RCIOPrintDigitalIo(3, digital_io_03, rc_dig_in03, rc_dig_out03);
    RCIOPrintDigitalIo(4, digital_io_04, rc_dig_in04, rc_dig_out04);
    RCIOPrintDigitalIo(5, digital_io_05, rc_dig_in05, rc_dig_out05);
    RCIOPrintDigitalIo(6, digital_io_06, rc_dig_in06, rc_dig_out06);
    RCIOPrintDigitalIo(7, digital_io_07, rc_dig_in07, rc_dig_out07);
    RCIOPrintDigitalIo(8, digital_io_08, rc_dig_in08, rc_dig_out08);
    RCIOPrintDigitalIo(9, digital_io_09, rc_dig_in09, rc_dig_out09);
    RCIOPrintDigitalIo(10, digital_io_10, rc_dig_in10, rc_dig_out10);
    RCIOPrintDigitalIo(11, digital_io_11, rc_dig_in11, rc_dig_out11);
    RCIOPrintDigitalIo(12, digital_io_12, rc_dig_in12, rc_dig_out12);
    RCIOPrintDigitalIo(13, digital_io_13, rc_dig_in13, rc_dig_out13);
    RCIOPrintDigitalIo(14, digital_io_14, rc_dig_in14, rc_dig_out14);
    RCIOPrintDigitalIo(15, digital_io_15, rc_dig_in15, rc_dig_out15);
    RCIOPrintDigitalIo(16, digital_io_16, rc_dig_in16, rc_dig_out16);
    RCIOPrintDigitalIo(17, digital_io_17, rc_dig_in17, rc_dig_out17);
    RCIOPrintDigitalIo(18, digital_io_18, rc_dig_in18, rc_dig_out18);

    RCIOPrintRelay(1, relay1_fwd, relay1_rev);
    RCIOPrintRelay(2, relay2_fwd, relay2_rev);
    RCIOPrintRelay(3, relay3_fwd, relay3_rev);
    RCIOPrintRelay(4, relay4_fwd, relay4_rev);
    RCIOPrintRelay(5, relay5_fwd, relay5_rev);
    RCIOPrintRelay(6, relay6_fwd, relay6_rev);
    RCIOPrintRelay(7, relay7_fwd, relay7_rev);
    RCIOPrintRelay(8, relay8_fwd, relay8_rev);

    return;
}       //RCIOPrintData

/// <summary>
///     This function prints the state of the RC digital IO.
/// </summary>
///
/// <param name="bIoChannel">
///     Specifies the digital I/O pin.
/// </param>
/// <param name="bIoConfig">
///     Specifies the I/O pin's configuration.
/// </param>
/// <param name="bInValue">
///     Specifies input state if the pin is configured as INPUT.
/// </param>
/// <param name="bOutValue">
///     Specifies output state if the pin is configured as OUTPUT.
/// </param>
///
/// <returns> None. </returns>

static void
RCIOPrintDigitalIo(
    __in BYTE bIoChannel,
    __in BYTE bIoConfig,
    __in BYTE bInValue,
    __in BYTE bOutValue
    )
{
    static char szInput[] = "INPUT";
    static char szOutput[] = "OUTPUT";
    
    printf("DigitalIo[%02d]: %s=%d\r\n",
           bIoChannel, bIoConfig == INPUT? szInput: szOutput,
           bIoConfig == INPUT? bInValue: bOutValue);
           
    return;
}       //RCIOPrintDigitalIo

/// <summary>
///     This function prints the state of the RC relay.
/// </summary>
///
/// <param name="bRelayChannel">
///     Specifies the relay channel.
/// </param>
/// <param name="bFwd">
///     Specifies the Fwd state of the relay.
/// </param>
/// <param name="bRev">
///     Specifies the Rev state of the relay.
/// </param>
///
/// <returns> None. </returns>

static void
RCIOPrintRelay(
    __in BYTE bRelayChannel,
    __in BYTE bFwd,
    __in BYTE bRev
    )
{
    printf("Relay[%02d]: Fwd=%d, Rev=%d\r\n", bRelayChannel, bFwd, bRev);
           
    return;
}       //RCIOPrintRelay
