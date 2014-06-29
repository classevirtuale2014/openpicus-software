/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/
#ifndef __integer
#define __integer

//#include "taskTCPIP.h"
//#include "TCPIP Stack/TCPIP.h"
#include "HWlib.h"

#ifndef _INTEGER
#define _INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
#include <tchar.h>

#else			/* Embedded platform */

typedef unsigned long CLUST;
typedef unsigned short	WCHAR;

// other types covered in openpicus sw

#endif

#endif
#endif
