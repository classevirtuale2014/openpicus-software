#include "HWlib.h"

#include "pic24f.h"
#include "uart.h"
#include "diskio.h"
#include "ff.h"
#include <string.h>

#ifndef _FATFSHELPER
#define _FATFSHELPER	

#ifdef USE_RTCC_LIB
	#include "RTCClib.h"
#endif

extern DWORD get_fattime (void);

#endif //_FATFSHELPER
