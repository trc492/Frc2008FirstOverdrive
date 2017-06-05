#if 0
/// Copyright (c) 2008 Michael Tsang. All rights are reserved.
///
/// <module name="basecode.h" />
///
/// <summary>
///     This module contains global definitions for all the base code.
///     This header should be included by every source file.
/// </summary>
///
/// <remarks>
///     Author: Michael Tsang (mikets@hotmail.com)
///     Date:   09-Mar-2008
///     Environment: MPLAB
/// </remarks>
#endif

#ifndef _basecode_h_
#define _basecode_h_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// Global macros.
//
#ifndef __in
  #define __in
#endif
#ifndef __out
  #define __out
#endif
#ifndef __inout
  #define __inout
#endif
#define ARRAYSIZE(x)            (sizeof(x)/sizeof(x[0]))
#define HIBYTE(x)               ((BYTE)(((WORD)(x) >> 8) & 0xff))
#define LOBYTE(x)               ((BYTE)(x))
#define HIWORD(x)               ((WORD)(((DWORD)(x) >> 16) & 0xffff))
#define LOWORD(x)               ((WORD)(x))
#define abs(x)                  (((x) >= 0)? (x): -(x))
#define IS_TRACE_ON             (g_dwSystemFlags & SYSF_TRACE_ON)
#define TPRINTF(x)              {if (IS_TRACE_ON) printf x;}

//
// Global constants.
//
#define FALSE                   0
#define TRUE                    !FALSE
#define PI_NUMER                355
#define PI_DENOM                113
#define PI                      PI_NUMER/PI_DENOM

#define MAX_OI_ANALOG_CHANNELS  16
#define MAX_RC_DIGITAL_IOS      18
#define MAX_RC_PWM_CHANNELS     16

#define DEF_TRACE_INTERVAL      15      //Trace interval in system ticks.
#define MAX_PREPROCESS_HOOKS    4
#define MAX_POSTPROCESS_HOOKS   4
#define MAX_CALIBRATION_HOOKS   4

// g_dwSysFlags values.
#define SYSF_TRACE_RCIO         0x00000001
#define SYSF_TRACE_PWM          0x00000002
#define SYSF_TRACE_ADC          0x00000004
#define SYSF_TRACE_ENCODER      0x00000008
#define SYSF_TRACE_GYRO         0x00000010
#define SYSF_TRACE_INPUT        0x00000020
#define SYSF_TRACE_SERVO        0x00000040
#define SYSF_CALIBRATION_MODE   0x20000000
#define SYSF_CONSOLE_MODE       0x40000000
#define SYSF_TRACE_ON           0x80000000

// RC digital channels.
#define RC_DIGITAL_01           (1 << 0)
#define RC_DIGITAL_02           (1 << 1)
#define RC_DIGITAL_03           (1 << 2)
#define RC_DIGITAL_04           (1 << 3)
#define RC_DIGITAL_05           (1 << 4)
#define RC_DIGITAL_06           (1 << 5)
#define RC_DIGITAL_07           (1 << 6)
#define RC_DIGITAL_08           (1 << 7)
#define RC_DIGITAL_09           (1 << 8)
#define RC_DIGITAL_10           (1 << 9)
#define RC_DIGITAL_11           (1 << 10)
#define RC_DIGITAL_12           (1 << 11)
#define RC_DIGITAL_13           (1 << 12)
#define RC_DIGITAL_14           (1 << 13)
#define RC_DIGITAL_15           (1 << 14)
#define RC_DIGITAL_16           (1 << 15)
#define RC_DIGITAL_17           (1 << 16)
#define RC_DIGITAL_18           (1 << 17)

//
// Global type definitions.
//
typedef unsigned char           BYTE;
typedef unsigned short          WORD;
typedef unsigned long           DWORD;
typedef unsigned char           BOOL;
typedef char *                  PSTR;
typedef BOOL (*PFNHOOK)(void);
typedef struct _HookTable
{
    PFNHOOK *HookFuncs;
    BYTE     bNumSlots;
    BYTE     bNumFuncs;
} HOOKTABLE, *PHOOKTABLE;

//
// Exported functions.
//
BOOL
RegisterHook(
    __inout PHOOKTABLE HookTable,
    __in PFNHOOK       pfnHook
    );

//
// Global data.
//
extern DWORD    g_dwCurrentTicks;
extern DWORD    g_dwLostPackets;
extern BYTE     g_bTraceInterval;
extern DWORD    g_dwSysFlags;
extern DWORD    g_dwFastLoops;

#include "ifi_default.h"
#include "ifi_aliases.h"
#include "project.h"

#endif

