#if 0
/// Copyright (c) 2005-2006 R. Kevin Watson. All rights are reserved.
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="serial_ports.c" />
///
/// <summary>
///     This module implements a fully buffered, interrupt-driven serial
///     port driver that can be used with either or both on-board serial
///     ports.
///     By default, serial port one will operate at 115200 baud which
///     is compatible with InnovationFIRST's terminal program. Serial
///     port two will operate at 9600 baud. These values can be easily
///     changed by modifying the serial port initialization functions
///     mentioned below.
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
///     22-Dec-2004 0.1 RKW - Original code.
///     28-Dec-2004 0.2 RKW - Using preprocessor directives, added the ability
///                     to enable/disable individual serial port receive and
///                     transmit code. Updated documentation.
///     05-Feb-2005 0.3 RKW - Added _user_putc() interface to C18's new output
///                     stream capabilities. Updated documentation.
///     05-Jan-2006 0.4 RKW - Partial port to 18F8722. Updated documentation.
///     10-Jan-2006 0.4 RKW - Modified the #pragma interruptlow line to also
///                     save the .tmpdata section.
/// </remarks>
#endif

#include "basecode.h"
#include "serial_ports.h"

//
// Local constants.
//

/// <summary>
///     FIFO queue state: the queue has characters in it.
/// </summary>
#define QSTATE_NORMAL           0
/// <summary>
///     FIFO queue state: the queue is empty.
/// </summary>
#define QSTATE_EMPTY            1
/// <summary>
///     FIFO queue state: the queue is full.
/// </summary>
#define QSTATE_FULL             2

// These values define the size, in bytes, of the circular queues used to
// buffer incoming and outgoing serial port data. The size of the receive
// queue is critical to prevent data loss. As an example, if you're
// continuously receiving data at 38,400 baud and only call SerialReadData
// in the Process_Data_From_Master_uP() loop, which is called every 26.2ms,
// you'll need to size the queue to accept all of the data you expect to
// receive in 26.2 ms. To do this, multiply the rate at which bytes of data
// are being received times the amount of time between calls to SerialReadData
// In the above example, we're receiving data at a rate of 3840 bytes per
// second (8 bits of data plus 2 bits of overhead per byte transferred) and
// we need to store at least 26.2 ms worth of that data for a calculated
// queue size of 101 bytes. Another solution is to check for received serial
// data at a higher rate by putting the call to SerialReadData in the much
// faster Process_Data_From_Local_IO() loop.
#define RX_QUEUE_SIZE           32
#define TX_QUEUE_SIZE           32

//
// Local type definitions.
//
typedef struct _SerialPort
{
    volatile BYTE   RxQueue[RX_QUEUE_SIZE]; //Receive circular Q
    volatile BYTE   RxQueueState;           //Receive Q state
    BYTE            RxQueueReadIndex;       //Receive Q read pointer
    volatile BYTE   RxQueueWriteIndex;      //Receive Q write pointer
    volatile BYTE   RxQueueByteCount;       //Data in receive Q
    volatile BYTE   RxOverrunErrors;        //# of overrun errors
    volatile BYTE   RxFramingErrors;        //# of framing errors
    volatile BYTE   TxQueue[TX_QUEUE_SIZE]; //Transmit circular Q
    volatile BYTE   TxQueueState;           //Transmit Q state
    volatile BYTE   TxQueueReadIndex;       //Transmit Q read pointer
    BYTE            TxQueueWriteIndex;      //Transmit Q write pointer
    volatile BYTE   TxQueueByteCount;       //Data in transmit Q
    BYTE            bPortFlags;
} SERIAL_PORT, *PSERIAL_PORT;

//
// Local data.
//
SERIAL_PORT SerialPorts[MAX_SERIAL_PORTS] = {0};

//
// Local function prototypes.
//
static void
SerialInitPort(
    __out PSERIAL_PORT SerialPort,
    __in BYTE          bPortFlags
    );

static void
SerialRxInterrupt(
    __in BYTE bSerialPort,
    __in BYTE fEnable
    );

static void
SerialTxInterrupt(
    __in BYTE bSerialPort,
    __in BYTE fEnable
    );

/// <summary>
///     This function initializes serial port one.
/// </summary>
///
/// <param name="bBaudRate">
///     Specifies the baud rate to initialize the serial port.
/// </param>
/// <param name="bFlags">
///     Specifies the option flags for initializing the serial port.
/// </param>
///
/// <returns> None. </returns>
///
/// <remarks>
///     This function must be called before you try to use serial port one.
///     By default, this serial port is set to 115200 baud with the
///     transmitter and receiver enabled. This is the rate IFI's terminal
///     program expects. The serial port's baud rate is programmed by
///     entering a value into the SPBRG1 register and possibly changing
///     the value of the BRGH bit. Several example values are included
///     in the serial_ports.h file. Numbers within brackets refer to the
///     PIC18F8722 data sheet page number where more information can be
///     found. This document can be found at microchip's website at
///     http://www.microchip.com or at the author's website at
///     http://www.kevin.org/frc.
/// </remarks>

void
Serial1Initialize(
    __in BYTE bBaudRate,
    __in BYTE bFlags
    )
{
    //
    // Start by initializing the serial port with code common to receive
    // and transmit functions.
    //
    // Baud rate generator register [251]
    SPBRG1 = bBaudRate;

    // High baud rate select bit (asynchronous mode only) [248]
    // 0: low speed
    // 1: high speed
    TXSTA1bits.BRGH = 1;

    // Receive interrupt enable bit [127]
    // 0: disable received data interrupt
    // 1: enable received data interrupt
    PIE1bits.RC1IE = 0;

    // Transmit interrupt enable bit [127]
    // 0: disable transmit register empty interrupt
    // 1: enable transmit register empty interrupt
    PIE1bits.TX1IE = 0;

    // USART mode select bit [248]
    // 0: asynchronous mode
    // 1: synchronous mode
    TXSTA1bits.SYNC = 0;

    // Clock source select bit (asynchronous mode only) [248]
    // 0: Slave mode (clock generated by external source)
    // 1: Master mode (clock generated internally from BRG)
    TXSTA1bits.CSRC = 0;

    if (bFlags & ENABLE_SERIAL_RX)
    {
        // Make sure the RC7/RX1/DT1 pin is configured as an input [140]
        TRISCbits.TRISC7 = 1;

        // 9-bit receive enable bit [249]
        // 0: 8-bit reception mode
        // 1: 9-bit reception mode
        RCSTA1bits.RX9 = 0;

        // Address detect enable bit (9-bit asynchronous mode only) [249]
        // 0: disables address detection
        // 1: enables address detection
        RCSTA1bits.ADDEN = 0;

        // Single receive enable bit (master synchronous mode only) [249]
        // 0: disables single receive mode
        // 1: enables single receive mode
        RCSTA1bits.SREN = 1;

        // Continuous receive mode enable bit [249]
        // Asynchronous mode:
        //  0: disables receiver
        //  1: enable receiver
        // Synchronous mode:
        //  0: disables continuous receive
        //  1: enables continuous receive until CREN is cleared
        RCSTA1bits.CREN = 1;

        // Receive interrupt priority bit (must be 0 for IFI controllers) [130]
        // 0: low-priority
        // 1: high-priority
        IPR1bits.RC1IP = 0;

        // Receive interrupt enable bit [127]
        // 0: disables received data interrupt
        // 1: enables received data interrupt
        PIE1bits.RC1IE = 1;
    }

    if (bFlags & ENABLE_SERIAL_TX)
    {
        // Use this driver for output stream functions
        stdout = _H_USER;

        // Make sure the RC6/TX1/CK1 pin is configured as an output [140]
        TRISCbits.TRISC6 = 0;

        // 9-bit transmit enable bit [248]
        // 0: 8-bit transmission mode
        // 1: 9-bit transmission mode
        TXSTA1bits.TX9 = 0;

        // Send break character bit [248]
        // 0: Sync Break transmission completed
        // 1: Send Sync Break on next transmission
        TXSTA1bits.SENDB = 0;

        // Transmit interrupt priority bit (must be 0 for IFI controllers) [130]
        // 0: low-priority
        // 1: high-priority
        IPR1bits.TX1IP = 0;

        // Transmit interrupt enable bit [127]
        // 0: disables transmit register empty interrupt
        // 1: enables transmit register empty interrupt
        PIE1bits.TX1IE = 1;

        // Enable transmitter [248]
        // 0: serial transmitter is disabled
        // 1: serial transmitter
        TXSTA1bits.TXEN = 1;
    }
    //
    // Finally, turn on the serial port.
    //
    // Serial Port Enable [249]
    // 0: serial port is disabled
    // 1: serial port is enabled
    RCSTA1bits.SPEN = 1;

    SerialInitPort(&SerialPorts[SERIAL_PORT_ONE], bFlags);
}       //Serial1Initialize

/// <summary>
///     This function initializes serial port two.
/// </summary>
///
/// <param name="bBaudRate">
///     Specifies the baud rate to initialize the serial port.
/// </param>
/// <param name="bFlags">
///     Specifies the option flags for initializing the serial port.
/// </param>
///
/// <returns> None. </returns>
///
/// <remarks>
///     This function must be called before you try to use serial port two.
///     By default, this serial port is set to 9600 baud with the
///     transmitter and receiver enabled. The serial port's baud rate is
///     programmed by entering a value into the SPBRG2 register and possibly
///     changing the value of the BRGH bit. Several example values are included
///     in the serial_ports.h file. Numbers within brackets refer to the
///     PIC18F8722 data sheet page number where more information can be
///     found. This document can be found at microchip's website at
///     http://www.microchip.com or at the author's website at
///     http://www.kevin.org/frc.
/// </remarks>

void
Serial2Initialize(
    __in BYTE bBaudRate,
    __in BYTE bFlags
    )
{
    //
    // Start by initializing the serial port with code common to receive
    // and transmit functions.
    //
    // Baud rate generator register [251]
    SPBRG2 = bBaudRate;

    // High baud rate select bit (asynchronous mode only) [248]
    // 0: low speed
    // 1: high speed
    TXSTA2bits.BRGH = 1;

    // Receive interrupt enable bit [129]
    // 0: disable received data interrupt
    // 1: enable received data interrupt
    PIE3bits.RC2IE = 0;

    // Transmit interrupt enable bit [129]
    // 0: disable transmit register empty interrupt
    // 1: enable transmit register empty interrupt
    PIE3bits.TX2IE = 0;

    // USART mode select bit [248]
    // 0: asynchronous mode
    // 1: synchronous mode
    TXSTA2bits.SYNC = 0;

    // Clock source select bit (asynchronous mode only) [248]
    // 0: Slave mode (clock generated by external source)
    // 1: Master mode (clock generated internally from BRG)
    TXSTA2bits.CSRC = 0;

    if (bFlags & ENABLE_SERIAL_RX)
    {
        // Make sure the RG2/RX2/DT2 pin is configured as an input [151]
        TRISGbits.TRISG2 = 1;

        // 9-bit receive enable bit [249]
        // 0: 8-bit reception mode
        // 1: 9-bit reception mode
        RCSTA2bits.RX9 = 0;

        // Address detect enable bit (9-bit asynchronous mode only) [249]
        // 0: disables address detection
        // 1: enables address detection
        RCSTA2bits.ADDEN = 0;

        // Single receive enable bit (master synchronous mode only) [249]
        // 0: disables single receive mode
        // 1: enables single receive mode
        RCSTA2bits.SREN = 1;

        // Continuous receive mode enable bit [249]
        // Asynchronous mode:
        //  0: disables receiver
        //  1: enable receiver
        // Synchronous mode:
        //  0: disables continuous receive
        //  1: enables continuous receive until CREN is cleared
        RCSTA2bits.CREN = 1;

        // Receive interrupt priority bit (must be 0 for IFI controllers) [132]
        // 0: low-priority
        // 1: high-priority
        IPR3bits.RC2IP = 0;

        // Receive interrupt enable bit [129]
        // 0: disables received data interrupt
        // 1: enables received data interrupt
        PIE3bits.RC2IE = 1;
    }

    if (bFlags & ENABLE_SERIAL_TX)
    {
        // Use this driver for output stream functions
        stdout = _H_USER;

        // Make sure the RG1/TX2/CK2 pin is configured as an output [151]
        TRISGbits.TRISG1 = 0;

        // 9-bit transmit enable bit [248]
        // 0: 8-bit transmission mode
        // 1: 9-bit transmission mode
        TXSTA2bits.TX9 = 0;

        // Send break character bit [248]
        // 0: Sync Break transmission completed
        // 1: Send Sync Break on next transmission
        TXSTA2bits.SENDB = 0;

        // Transmit interrupt priority bit (must be 0 for IFI controllers) [132]
        // 0: low-priority
        // 1: high-priority
        IPR3bits.TX2IP = 0;

        // Transmit interrupt enable bit [129]
        // 0: disables transmit register empty interrupt
        // 1: enables transmit register empty interrupt
        PIE3bits.TX2IE = 1;

        // Enable transmitter [248]
        // 0: serial transmitter is disabled
        // 1: serial transmitter
        TXSTA2bits.TXEN = 1;
    }
    //
    // Finally, turn on the serial port.
    //
    // Serial Port Enable [249]
    // 0: serial port is disabled
    // 1: serial port is enabled
    RCSTA2bits.SPEN = 1;
    
    SerialInitPort(&SerialPorts[SERIAL_PORT_TWO], bFlags);
}       //Serial2Initialize

/// <summary>
///     This function returns the numbers of bytes in the receive queue.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to get the byte count.
/// </param>
///
/// <returns> Returns the number of bytes in the receive queue. </returns>
///
/// <remarks>
///     This function must be called to determine how much data, if any,
///     is present in serial port's received data queue. If the returned
///     number is greater than zero, then a call to SerialGetData can be
///     made to retrieve the next byte.
/// </remarks>

BYTE
SerialGetByteCount(
    __in BYTE bSerialPort
    )
{
    BYTE bByteCount = 0;

    if (bSerialPort < MAX_SERIAL_PORTS)
    {
        // Since we're about to access the RxQueueByteCount variable
        // which can also be modified in the interrupt service routine,
        // let's briefly disable the serial port interrupt to make sure
        // that RxQueueByteCount doesn't get altered while we're using it.
        SerialRxInterrupt(bSerialPort, 0);
        bByteCount = SerialPorts[bSerialPort].RxQueueByteCount;
        SerialRxInterrupt(bSerialPort, 1);
    }

    return bByteCount;
}       //SerialGetByteCount

/// <summary>
///     This function returns the next byte in the serial port's receive
///     queue.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to get the byte count.
/// </param>
///
/// <returns> Returns the next bytes in the receive queue. </returns>

BYTE
SerialReadData(
    __in BYTE bSerialPort
    )
{
    BYTE bData = 0;

    if ((bSerialPort < MAX_SERIAL_PORTS) &&
        (SerialPorts[bSerialPort].RxQueueState != QSTATE_EMPTY))
    {
        // Get a byte from the circular queue and store it temporarily.
        bData = SerialPorts[bSerialPort].RxQueue[
                        SerialPorts[bSerialPort].RxQueueReadIndex];
        // Decrement the queue byte count.
        SerialPorts[bSerialPort].RxQueueByteCount--;
        // Increment the read pointer.
        SerialPorts[bSerialPort].RxQueueReadIndex++;
        if (SerialPorts[bSerialPort].RxQueueReadIndex >= RX_QUEUE_SIZE)
        {
            //
            // Wrap around the circular queue.
            //
            SerialPorts[bSerialPort].RxQueueReadIndex = 0;
        }

        // Since we're about to use the RxQueueWriteIndex variable which can
        // also be modified in the interrupt service routine, let's briefly
        // disable the serial port interrupt to make sure that
        // RxQueueWriteIndex doesn't get altered while we're using it.
        SerialRxInterrupt(bSerialPort, 0);

        // Check to see if the queue is empty
        if (SerialPorts[bSerialPort].RxQueueReadIndex ==
            SerialPorts[bSerialPort].RxQueueWriteIndex)
        {
            SerialPorts[bSerialPort].RxQueueState = QSTATE_EMPTY;
        }
        else
        {
            // Since we've just remove a byte from the queue, it can't
            // possibly be full.
            SerialPorts[bSerialPort].RxQueueState = QSTATE_NORMAL;
        }

        // Okay, we're done using RxQueueWriteIndex, so turn the serial port
        // interrupt back on.
        SerialRxInterrupt(bSerialPort, 1);
    }

    return bData;
}       //SerialReadData

/// <summary>
///     This function writes a byte to the serial port's transmit queue.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to get the byte count.
/// </param>
/// <param name="bData">
///     Specifies the data to write to the serial port.
/// </param>
///
/// <returns> Returns the next bytes in the receive queue. </returns>
///
/// <remarks>
///     If you don't initialize the serial port before calling this function,
///     the robot controller will stop functioning and you'll get the much
///     dreaded red-light-of-death. This is because the while() statement
///     below is waiting for the transmit circuitry to send another byte,
///     but if the serial port hasn't been configured, nothing will be
///     transmitted and we'll be stuck in the while() loop.
/// </remarks>

void
SerialWriteData(
    __in BYTE bSerialPort,
    __in BYTE bData
    )
{
    if (bSerialPort < MAX_SERIAL_PORTS)
    {
        // If the queue is full, wait here until space is available.
        while (SerialPorts[bSerialPort].TxQueueState == QSTATE_FULL)
            ;

        // Put the byte into the circular queue.
        SerialPorts[bSerialPort].TxQueue[
            SerialPorts[bSerialPort].TxQueueWriteIndex] = bData;
        // Increment the queue byte count.
        SerialPorts[bSerialPort].TxQueueByteCount++;
        // increment the write pointer
        SerialPorts[bSerialPort].TxQueueWriteIndex++;
        if (SerialPorts[bSerialPort].TxQueueWriteIndex >= TX_QUEUE_SIZE)
        {
            //
            // Wrap around the circular queue.
            //
            SerialPorts[bSerialPort].TxQueueWriteIndex = 0;
        }

        // Since we're about to use the TxQueueWriteIndex variable which can
        // also be modified in the interrupt service routine, let's briefly
        // disable the serial port interrupt to make sure that
        // TxQueueWriteIndex doesn't get altered while we're using it.
        SerialTxInterrupt(bSerialPort, 0);

        // Check to see if the queue is full.
        if (SerialPorts[bSerialPort].TxQueueReadIndex ==
            SerialPorts[bSerialPort].TxQueueWriteIndex)
        {
            SerialPorts[bSerialPort].TxQueueState = QSTATE_FULL;
        }
        else
        {
            // Since we've just added a byte to the queue, it can't possibly be
            // empty.
            SerialPorts[bSerialPort].TxQueueState = QSTATE_NORMAL;
        }

        // Okay, we're done using Tx_Queue_Write_Index, so turn the serial
        // port interrupt back on.
        SerialTxInterrupt(bSerialPort, 1);
    }

    return;
}       //SerialWriteData

/// <summary>
///     This function overrides the putc() interface to C18 2.4 output
///     stream functions and writes a byte to the serial port's transmit
///     queue.
/// </summary>
///
/// <param name="bData">
///     Specifies the data to write to the serial port.
/// </param>
///
/// <returns> None. </returns>

void
_user_putc(
    __in BYTE bData
    )
{
    if (SerialPorts[SERIAL_PORT_ONE].bPortFlags & ENABLE_SERIAL_STDOUT)
    {
        SerialWriteData(SERIAL_PORT_ONE, bData);
    }
    else if (SerialPorts[SERIAL_PORT_TWO].bPortFlags & ENABLE_SERIAL_STDOUT)
    {
        SerialWriteData(SERIAL_PORT_TWO, bData);
    }
    
    return;
}       //_user_putc

/// <summary>
///     This function is called when there is a receive interrupt on
///     the serial port.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to get the byte count.
/// </param>
///
/// <returns> None. </returns>
///
/// <remarks>
///     If the interrupt handler was installed correctly, this function will
///     be called every time a new byte of data is received by serial port.
/// </remarks>

void
SerialRxIntHandler(
    __in BYTE bSerialPort
    )
{
    if (SerialPorts[bSerialPort].RxQueueState == QSTATE_FULL)
    {
        // Just turn off the serial port interrupt if we can't store any more
        // data. The interrupt will be re-enabled within the SerialReadData
        // function when more data is read.
        SerialRxInterrupt(bSerialPort, 0);
    }
    else
    {
        if (bSerialPort == SERIAL_PORT_ONE)
        {
            // Put the byte into the circular queue.
            SerialPorts[SERIAL_PORT_ONE].RxQueue[SerialPorts[
                SERIAL_PORT_ONE].RxQueueWriteIndex] = RCREG1;

            // If the interrupt handler was disabled while data was being
            // received, data may have backed-up in the receiver circuitry,
            // causing an overrun condition. So let's check the OERR bit to
            // see if this has happened and if it has, we'll need to reset
            // the serial port receiver circuitry to get data flowing again.
            if (RCSTA1bits.OERR)
            {
                // Reset by turning off the receiver circuitry, then...
                RCSTA1bits.CREN = 0;
                // ...turn it back on.
                RCSTA1bits.CREN = 1;
                // Indicate that we've had an error.
                SerialPorts[SERIAL_PORT_ONE].RxOverrunErrors++;
            }

            // If incoming data gets misaligned and the receiver doesn't
            // receive a stop bit where it expects to detect it, the receiver
            // circuitry will set the FERR bit to indicate that it's received
            // corrupted data. The likely reason for this is an incorrectly
            // set baud rate on either the receiver or transmitter end.
            if (RCSTA1bits.FERR)
            {
                SerialPorts[SERIAL_PORT_ONE].RxFramingErrors++;
            }
        }
        else
        {
            // Put the byte into the circular queue.
            SerialPorts[SERIAL_PORT_TWO].RxQueue[SerialPorts[
                SERIAL_PORT_TWO].RxQueueWriteIndex] = RCREG2;

            // If the interrupt handler was disabled while data was being
            // received, data may have backed-up in the receiver circuitry,
            // causing an overrun condition. So let's check the OERR bit to
            // see if this has happened and if it has, we'll need to reset
            // the serial port receiver circuitry to get data flowing again.
            if (RCSTA2bits.OERR)
            {
                // Reset by turning off the receiver circuitry, then...
                RCSTA2bits.CREN = 0;
                // ...turn it back on.
                RCSTA2bits.CREN = 1;
                // Indicate that we've had an error.
                SerialPorts[SERIAL_PORT_TWO].RxOverrunErrors++;
            }

            // If incoming data gets misaligned and the receiver doesn't
            // receive a stop bit where it expects to detect it, the receiver
            // circuitry will set the FERR bit to indicate that it's received
            // corrupted data. The likely reason for this is an incorrectly
            // set baud rate on either the receiver or transmitter end.
            if (RCSTA2bits.FERR)
            {
                SerialPorts[SERIAL_PORT_TWO].RxFramingErrors++;
            }
        }

        // Increment the queue byte count.
        SerialPorts[bSerialPort].RxQueueByteCount++;

        // Increment the write pointer.
        SerialPorts[bSerialPort].RxQueueWriteIndex++;
        if (SerialPorts[bSerialPort].RxQueueWriteIndex >= RX_QUEUE_SIZE)
        {
            //
            // Wrap around the circular queue.
            //
            SerialPorts[bSerialPort].RxQueueWriteIndex = 0;
        }

        // Is the circular queue now full?
        if (SerialPorts[bSerialPort].RxQueueReadIndex ==
            SerialPorts[bSerialPort].RxQueueWriteIndex)
        {
            SerialPorts[bSerialPort].RxQueueState = QSTATE_FULL;
        }
        else
        {
            // Since we've just added a byte to the queue, it can't possibly
            // be empty.
            SerialPorts[bSerialPort].RxQueueState = QSTATE_NORMAL;
        }
    }

    return;
}       //SerialRxIntHandler

/// <summary>
///     This function is called when there is a transmit empty interrupt on
///     the serial port.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port to get the byte count.
/// </param>
///
/// <returns> None. </returns>
///
/// <remarks>
///     If the interrupt handler was installed correctly, this function will
///     be called every time the serial port is ready to start sending a byte
///     of data.
/// </remarks>

void
SerialTxIntHandler(
    __in BYTE bSerialPort
    )
{
    if (SerialPorts[bSerialPort].TxQueueState == QSTATE_EMPTY)
    {
        // Just turn off the serial port interrupt if we don't have data to
        // send. The interrupt will be re-enabled within the SerialWriteData
        // function when more data is sent.
        SerialTxInterrupt(bSerialPort, 0);
    }
    else
    {
        if (bSerialPort == SERIAL_PORT_ONE)
        {
            // Get a byte from the circular queue and send it to the USART.
            TXREG1 = SerialPorts[SERIAL_PORT_ONE].TxQueue[
                SerialPorts[SERIAL_PORT_ONE].TxQueueReadIndex];
        }
        else
        {
            // Get a byte from the circular queue and send it to the USART.
            TXREG2 = SerialPorts[SERIAL_PORT_TWO].TxQueue[
                SerialPorts[SERIAL_PORT_TWO].TxQueueReadIndex];
        }

        // Decrement the queue byte count.
        SerialPorts[bSerialPort].TxQueueByteCount--;

        // Increment the read pointer.
        SerialPorts[bSerialPort].TxQueueReadIndex++;
        if (SerialPorts[bSerialPort].TxQueueReadIndex >= TX_QUEUE_SIZE)
        {
            //
            // Wrap around the circular queue.
            //
            SerialPorts[bSerialPort].TxQueueReadIndex = 0;
        }

        // Is the circular queue now empty?
        if (SerialPorts[bSerialPort].TxQueueReadIndex ==
            SerialPorts[bSerialPort].TxQueueWriteIndex)
        {
            SerialPorts[bSerialPort].TxQueueState = QSTATE_EMPTY;
        }
        else
        {
            // Since we've just removed a byte from the queue, it can't
            // possibly be full.
            SerialPorts[bSerialPort].TxQueueState = QSTATE_NORMAL;
        }
    }

    return;
}       //SerialTxIntHandler

/// <summary>
///     This function initializes serial port structure.
/// </summary>
///
/// <param name="SerialPort">
///     Points to the SerialPort structure to be initialized.
/// </param>
/// <param name="bFlags">
///     Specifies the initialization flags.
/// </param>
///
/// <returns> None. </returns>

static void
SerialInitPort(
    __out PSERIAL_PORT SerialPort,
    __in BYTE          bFlags
    )
{
    SerialPort->RxQueueState = SerialPort->TxQueueState = QSTATE_EMPTY;
    SerialPort->RxQueueReadIndex = SerialPort->TxQueueReadIndex = 0;
    SerialPort->RxQueueWriteIndex = SerialPort->TxQueueWriteIndex = 0;
    SerialPort->RxQueueByteCount = SerialPort->TxQueueByteCount = 0;
    SerialPort->RxOverrunErrors = SerialPort->RxFramingErrors = 0;
    SerialPort->bPortFlags = bFlags;

    return;
}       //SerialInitPort

/// <summary>
///     This function enables or disable receive interrupt for the serial port.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port.
/// </param>
/// <param name="fEnable">
///     If 1, enable interrupt. If 0, disable interrupt.
/// </param>
///
/// <returns> None. </returns>

static void
SerialRxInterrupt(
    __in BYTE bSerialPort,
    __in BYTE fEnable
    )
{
    if (bSerialPort < MAX_SERIAL_PORTS)
    {
        if (bSerialPort == 0)
        {
            PIE1bits.RC1IE = fEnable;
        }
        else
        {
            PIE3bits.RC2IE = fEnable;
        }
    }

    return;
}       //SerialRxInterrupt

/// <summary>
///     This function enables or disable transmit interrupt for the serial port.
/// </summary>
///
/// <param name="bSerialPort">
///     Specifies the serial port.
/// </param>
/// <param name="fEnable">
///     If 1, enable interrupt. If 0, disable interrupt.
/// </param>
///
/// <returns> None. </returns>

static void
SerialTxInterrupt(
    __in BYTE bSerialPort,
    __in BYTE fEnable
    )
{
    if (bSerialPort < MAX_SERIAL_PORTS)
    {
        if (bSerialPort == 0)
        {
            PIE1bits.TX1IE = fEnable;
        }
        else
        {
            PIE3bits.TX2IE = fEnable;
        }
    }

    return;
}       //SerialTxInterrupt
