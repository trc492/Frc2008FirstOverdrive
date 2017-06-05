#if 0
/// <module name="ifi_utilities.h" />
///
/// <summary>
///     This module contains the definitions of the file ifi_utilities.c.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef __ifi_utilities_h_
#define __ifi_utilities_h_

#ifdef _SNOOP_ON_COM1   //For future use
#define TXINTF                  PIR3bits.TX2IF 
#else
#define TXINTF                  PIR1bits.TXIF 
#endif

//
// Exported function prototypes.
//
void
Wait4TXEmpty(
    void
    );

#endif
