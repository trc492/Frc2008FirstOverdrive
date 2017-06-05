#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="serial_ports.h" />
///
/// <summary>
///     This module contains the definitions of the serial port.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on code by: R. Kevin Watson (kevinw@jpl.nasa.gov)
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _serial_ports_h
#define _serial_ports_h

//
// Exported constants.
//
#define MAX_SERIAL_PORTS        2

// bFlags values used in the SerialInitialize functions.
#define ENABLE_SERIAL_RX        0x01
#define ENABLE_SERIAL_TX        0x02
#define ENABLE_SERIAL_STDOUT    0x04

// bBaudRate values used in the SerialInitialize functions.
#define BAUD_4800               129     // set BRGH = 0
#define BAUD_9600               64      // set BRGH = 0
#define BAUD_14400              42      // set BRGH = 0
#define BAUD_19200              129     // set BRGH = 1
#define BAUD_38400              64      // set BRGH = 1
#define BAUD_57600              42      // set BRGH = 1
#define BAUD_115200             21      // set BRGH = 1
#define BAUD_230400             10      // set BRGH = 1

// #defines used with the stdout_serial_port global variable
#define SERIAL_PORT_ONE         0
#define SERIAL_PORT_TWO         1

//
// Exported function prototypes.
//

void
Serial1Initialize(
    __in BYTE bBaudRate,
    __in BYTE bFlags
    );

void
Serial2Initialize(
    __in BYTE bBaudRate,
    __in BYTE bFlags
    );

BYTE
SerialGetByteCount(
    __in BYTE bSerialPort
    );

BYTE
SerialReadData(
    __in BYTE bSerialPort
    );

void
SerialWriteData(
    __in BYTE bSerialPort,
    __in BYTE bData
    );
    
void
_user_putc(
    __in BYTE bData
    );

void
SerialRxIntHandler(
    __in BYTE bSerialPort
    );

void
SerialTxIntHandler(
    __in BYTE bSerialPort
    );

#endif
