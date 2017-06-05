#if 0
/// <module name="ifi_utilities.c" />
///
/// <summary>
///     This module contains some useful functions that you can call in
///     your program.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

#include "basecode.h"
#include "ifi_utilities.h"

/// <summary>
///     This function waits for serial transmit buffer to be empty.
/// </summary>
///
/// <returns> None. </returns>
///
/// <remarks>
///     Used when transmitting data serially. It waits for each byte to finish.
/// </remarks>

void
Wait4TXEmpty(
    void
    )
{
#ifndef _SIMULATOR
    while (!PIR3bits.TX2IF)
    {
        continue;
    }
#endif
}       //Wait4TXEmpty

