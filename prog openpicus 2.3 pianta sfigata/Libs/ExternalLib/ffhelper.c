#include "ffhelper.h"

volatile BYTE rtcYear = 110, rtcMon = 05, rtcMday = 13, rtcHour= 15, rtcMin =30, rtcSec=0;

/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

DWORD get_fattime (void)
{
	DWORD tmr;
		
#if defined USE_RTCC_LIB
	struct tm rtccTime;
	RTCCGet(&rtccTime);
	
	// Pack date and time into a DWORD variable 
	tmr = 	(((DWORD)rtccTime.tm_year - 80) << 25) |
			((DWORD)rtccTime.tm_mon << 21) | 
			((DWORD)rtccTime.tm_mday << 16)| 
			(WORD)(rtccTime.tm_hour << 11) | 
			(WORD)(rtccTime.tm_min<< 5) |
			(WORD)(rtccTime.tm_sec >> 1); // to have sec/2 value...
#else
		/* Pack date and time into a DWORD variable 
		tmr =	  (((DWORD)rtcYear - 80) << 25)
				| ((DWORD)rtcMon << 21)
				| ((DWORD)rtcMday << 16)
				| (WORD)(rtcHour << 11)
				| (WORD)(rtcMin << 5)
				| (WORD)(rtcSec >> 1);
		*/
		tmr = 0;
#endif
	return tmr;
}
