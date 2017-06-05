#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="rcio.h" />
///
/// <summary>
///     This module contains the definitions of the RC I/O module.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   11-Mar-2008
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _rcio_h_
#define _rcio_h_

//
// Exported function prototypes.
//
void
RCIOInitialize(
    __in DWORD dwOutChannelMask
    );

void
RCIOPrintData(
    void
    );
    
#endif
