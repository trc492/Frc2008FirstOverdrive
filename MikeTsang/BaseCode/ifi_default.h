#if 0
/// <module name="ifi_default.h" />
///
/// <summary>
///     This module contains important data definitions.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef __ifi_default_h_
#define __ifi_default_h_

#include <p18cxxx.h>

#define UNCHANGEABLE_DEFINITION_AREA    1
#ifdef UNCHANGEABLE_DEFINITION_AREA
//
// Alias definitions. Do not change!
//
#define DATA_SIZE               30
#define SPI_TRAILER_SIZE        2
#define SPI_XFER_SIZE           (DATA_SIZE + SPI_TRAILER_SIZE)

#ifdef _SIMULATOR
#define RESET_VECTOR            0x000
#define HIGH_INT_VECTOR         0x008
#define LOW_INT_VECTOR          0x018
#else
#define RESET_VECTOR            0x800
#define HIGH_INT_VECTOR         0x808
#define LOW_INT_VECTOR          0x818
#endif

//
// Type definitions. Do not change!
//

// This structure contains important system status information.
typedef struct
{
    unsigned int :5;
    unsigned int user_display:1;// User display enabled=1, disabled=0.
    unsigned int autonomous:1;  // Autonomous enabled=1, disabled=0.
    unsigned int disabled:1;    // Robot disabled=1, enabled=0.
} modebits;

// This structure allows you to address specific bits of a byte.
typedef struct
{
    unsigned int  bit0:1;
    unsigned int  bit1:1;
    unsigned int  bit2:1;
    unsigned int  bit3:1;
    unsigned int  bit4:1;
    unsigned int  bit5:1;
    unsigned int  bit6:1;
    unsigned int  bit7:1;
} bitid;

// This structure defines the contents of the data received from the Master
// microprocessor.
typedef struct
{
    unsigned char packet_num;
    union
    {
        bitid bitselect;
        modebits mode;          //rxdata.rc_mode_byte.mode.(user_display|autonomous|disabled)
        unsigned char allbits;  //rxdata.rc_mode_byte.allbits
    } rc_mode_byte;
    union
    {
        bitid bitselect;        //rxdata.oi_swA_byte.bitselect.bit0
        unsigned char allbits;  //rxdata.oi_swA_byte.allbits
    } oi_swA_byte;
    union
    {
        bitid bitselect;        //rxdata.oi_swB_byte.bitselect.bit0
        unsigned char allbits;  //rxdata.oi_swB_byte.allbits
    } oi_swB_byte;
    union
    {
        bitid bitselect;        //rxdata.rc_swA_byte.bitselect.bit0
        unsigned char allbits;  //rxdata.rc_swA_byte.allbits
    } rc_swA_byte;
    union
    {
        bitid bitselect;        //rxdata.rc_swB_byte.bitselect.bit0
        unsigned char allbits;  //rxdata.rc_swB_byte.allbits
    } rc_swB_byte;
    unsigned char oi_analog[MAX_OI_ANALOG_CHANNELS];
    unsigned char rc_main_batt;
    unsigned char rc_backup_batt;
    unsigned char reserve[8];
} rx_data_record, *rx_data_ptr;

// This structure defines the contents of the data transmitted to the master
// microprocessor.
typedef struct
{
    union
    {
        bitid bitselect;        //txdata.LED_byte1.bitselect.bit0
        unsigned char data;     //txdata.LED_byte1.data
    } LED_byte1;
    union
    {
        bitid bitselect;        //txdata.LED_byte2.bitselect.bit0
        unsigned char data;     //txdata.LED_byte2.data
    } LED_byte2;
    union
    {
        bitid bitselect;        //txdata.user_byte1.bitselect.bit0
        unsigned char allbits;  //txdata.user_byte1.allbits
    } user_byte1;               //for OI feedback
    union
    {
        bitid bitselect;        //txdata.user_byte2.bitselect.bit0
        unsigned char allbits;  //txdata.user_byte2.allbits
    } user_byte2;               //for OI feedback
    unsigned char rc_pwm[MAX_RC_PWM_CHANNELS];

    unsigned char user_cmd;     //reserved - for future use
    unsigned char cmd_byte1;    //reserved - Don't use
    unsigned char pwm_mask;     //<EDU> make sure you know how this works before changing
    unsigned char warning_code; //reserved - Don't use
    unsigned char user_byte3;   //<FRC> break panel byte 3
    unsigned char user_byte4;   //<FRC> break panel byte 4
    unsigned char user_byte5;   //<FRC> break panel byte 5
    unsigned char user_byte6;   //<FRC> break panel byte 6
    unsigned char error_code;   //reserved - Don't use
    unsigned char packetnum;    //reserved - Don't use
    unsigned char current_mode; //reserved - Don't use
    unsigned char control;      //reserved - Don't use
} tx_data_record, *tx_data_ptr;

// This structure defines some flags which are used by the system.
typedef struct
{
    unsigned int NEW_SPI_DATA:1;
    unsigned int TX_UPDATED:1;
    unsigned int FIRST_TIME:1;
    unsigned int TX_BUFFSELECT:1;
    unsigned int RX_BUFFSELECT:1;
    unsigned int SPI_SEMAPHORE:1;
    unsigned int :2;
} packed_struct;

#else
  #error  *** Error - Invalid Default File!
#endif

//
// Global data.
//
extern rx_data_record   rxdata;
extern tx_data_record   txdata;
extern packed_struct    statusflag;

//
// Public function prototypes.
//

// These routines reside in ifi_library.lib.

// This vector jumps to the appropriate high priority interrupt handler.
void
InterruptHandlerHigh(
    void
    );

void
Initialize_Registers(
    void
    );

// Configure registers and initializes the SPI RX/TX buffers.
void
IFI_Initialization(
    void
    );

// This routine informs the master processor that all user initalization
// has been complete. The call to this routine must always be the last line
// of User_Initialization.
void
User_Proc_Is_Ready(
    void
    );

// Fill the transmit buffer with data to send out to the master microprocessor.
// This routine takes 23 us to complete.
void
Putdata(
   __in tx_data_ptr TxData
    );

// Retrieve data from the SPI receive buffer sent by the master microprocessor.
//  This routine takes 14.8 us to complete.
void
Getdata(
    __out rx_data_ptr RxData
    );

void
Setup_PWM_Output_Type(
    __in int pwmSpec1,
    __in int pwmSpec2,
    __in int pwmSpec3,
    __in int pwmSpec4
    );

#endif

