/*                     - ICCEXT.H -

   Some extra declarations for non-ANSI functions used by the C library.

   $Name: not supported by cvs2svn $

   Copyright 1986 - 1999 IAR Systems. All rights reserved.
*/


#ifndef _ICCEXT_INCLUDED
#define _ICCEXT_INCLUDED

#include "sysmac.h"

#if __IAR_SYSTEMS_ICC__ < 2
#if __TID__ & 0x8000
#pragma function=intrinsic(0)
#endif
#endif

#ifndef MEMORY_ATTRIBUTE
#define MEMORY_ATTRIBUTE
#endif

__INTRINSIC MEMORY_ATTRIBUTE double exp10(double);

#if __IAR_SYSTEMS_ICC__ < 2
#if __TID__ & 0x8000
#pragma function=default
#endif
#endif

#endif

