/** \file sd_manager.c
 *  \brief FAT File System manager APIs
 *
 * \addtogroup Storage
 * @{
*/

/**
\defgroup FAT File System - SD CARD
@{

FAT16/FAT32 File system libraries for SD-Card 

\section Storage SD-Card FAT File System library
*/

/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        sd_manager.c
 *  Dependencies:    Microchip configs files
 *  Module:          FlyPort generic module
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/
 
#include "sd_manager.h"
/// @cond debug

//	Static function prototypes
static BOOL diskUnmount();
static BOOL diskMount();
static void setSdErr(int _err);
static void fileInit();
static void fileClose(FIL *file_close);
static void errManager();
static void fObjClose(FIL* fobj);
static void fObjInit(FIL* fobj);

//	Static variables
static int sdErr = 0;
static FATFS Fatfs;
static FIL fObj;
#if _FS_MINIMIZE == 0
static FILINFO fInfoObj;
#endif
static BOOL sdInitOk = FALSE;
static FRESULT fRes;
static BOOL streamEof = FALSE;
static FILINFO workfno;
static DIR workdir;
static BOOL scan_started = FALSE;

//	Error handling
static void setSdErr(int _err)
{
	sdErr = _err;
}

static void errManager()
{
	if (fRes == FR_NOT_READY)
	{
		if (sdDetect())
			setSdErr(SD_NOT_READY);
		else
			setSdErr(SD_NOT_PRESENT);
	}
	else
		setSdErr(fRes);
}
/// @endcond

/**
 * Initializes the hardware/software SD card module using a Nest carrier 
 * board. If a custom carrier board is used, please use SDInit() instead
 * 	\param int _nest - Nest carrier board definition to use
 *	\param BYTE _nTimeout - timeout of operation (expressed in about 0.1 sec scale)
 *	\return
 *	BOOL result of operation
 */	
BOOL SDOnNest(int _nest, BYTE _nTimeout)
{
	if (_nest == SD_GROVE)
		return SDInit(p18, p7, p17, p9, p19, _nTimeout);
	else if(_nest == SD_MUSIC)
		return SDInit(p8, p12, p10, p7, p5, _nTimeout);
	else
		return FALSE;
}


/**
 *	Initializes the hardware/software SD card module. If the SD card is not 
 *	inserted, loops waiting to mount it until the SD ready is ready.
 *
 *	\param int pin_sck - pin used as SPI SCLK signal
 *	\param int pin_si  - pin used as SPI MISO signal
 *	\param int pin_so  - pin used as SPI MOSI  signal
 *	\param int pin_cs  - pin used as SD Chip Select signal
 *	\param int pin_cd  - pin used as SD Card Detect signal
 *	\param BYTE _nTimeout - timeout of operation (expressed in about 0.1 sec scale, 255 to wait forever)
 *	\return
 *	BOOL result of operation
 */	
BOOL SDInit(int pin_sck, int pin_si, int pin_so, int pin_cs, int pin_cd, BYTE timeout)
{
	BYTE timecnt = 0;
	//	SD hardware initialization
	pinConfig(pin_sck, pin_si, pin_so, pin_cs, pin_cd);
			
	SDdebug("Initializing SD card...\r\n");

	if (diskMount())
		SDdebug("SD software module initialized!\r\n");
	else
		SDdebug("SD software module NOT initialized!\r\n");
		
	// Check to detect SD card insertion
	while (disk_status(0))
	{
		diskUnmount();
		vTaskDelay(5);
		SDdebug(".");
		diskMount();
		timecnt++;
		if ( (timecnt == timeout) && (timeout < 255) )
			break;
		vTaskDelay(5);
	}
	
	if (!disk_status(0))
	{
		SDdebug("SD card initialized and ready!\r\n");
		setSdErr(0);
		fileInit();
		sdInitOk = TRUE;
		return TRUE;
	}
	else 
	{
		SDdebug("ERROR\n");
		if (sdDetect())
			setSdErr(SD_FS_NOT_INIT);
		else
			setSdErr(SD_NOT_PRESENT);
		sdInitOk = FALSE;
		return FALSE;
	}
}


/**
 *	Unmounts SD Card disk in software libraries. Useful to provide removable 
 *	disk drives application.
 *
 *	\param None
 *	\return
 *	BOOL result of operation
 */
BOOL SDUnMount()
{
	return diskUnmount();
}

/**
 * Retrieve error number of last operation failed
 * \return error number from a list
<UL>
	<LI>(0) Succeeded</LI>
	<LI>(1) A hard error occured in the low level disk I/O layer</LI>
	<LI>(2) Assertion failed</LI>
	<LI>(3) The physical drive cannot work</LI>
	<LI>(4) Could not find the file</LI>
	<LI>(5) Could not find the path</LI>
	<LI>(6) The path name format is invalid</LI>
	<LI>(7) Acces denied due to prohibited access or directory full</LI>
	<LI>(8) Acces denied due to prohibited access</LI>
	<LI>(9) The file/directory object is invalid</LI>
	<LI>(10) The physical drive is write protected</LI>
	<LI>(11) The logical drive number is invalid</LI>
	<LI>(12) The volume has no work area</LI>
	<LI>(13) There is no valid FAT volume on the physical drive</LI>
	<LI>(14) The f_mkfs() aborted due to any parameter error</LI>
	<LI>(15) Could not get a grant to access the volume within defined period</LI>
	<LI>(16) The operation is rejected according to the file shareing policy</LI>
	<LI>(17) LFN working buffer could not be allocated</LI>
	<LI>(18) Number of open files > _FS_SHARE</LI>
	<LI>(20) SD library not Initialized</LI>
	<LI>(21) SD card not present</LI>
	<LI>(22) SD not ready</LI>
	<LI>(23) File not found</LI>
	<LI>(24) Generic Error</LI>
</UL>
 */
int SDGetErr()
{
	return sdErr;
}

/**
 * Retrieve available free space on disk (in KB)
 * \return DWORD >0: amount of KB available
 * \return DWORD 0: operation failed (for example read only configuration set) or no available space left
 */
DWORD SDFreeSpace()
{
#if _FS_READONLY != 0
	return 0;
#else
	FATFS *fs;
    DWORD fre_clust, fre_sect;
    
    // Get volume information and free clusters of drive 1 
    fRes = f_getfree("0:", &fre_clust, &fs);
    if (fRes != FR_OK)
    {
	    errManager();
    	return 0;
    } 
    setSdErr(0);
    fre_sect = fre_clust * fs->csize;

	return (DWORD)(fre_sect / 2);
#endif
}

/**
 *	Checks for a file existence
 *
 *	\param char* nfile_check - char[] with filename to check
 *	\return TRUE - the file exists
 *	\return FALSE - The file doesn't exist
 */
BOOL SDFileCheck(char* nfile_check)
{	
	//	Open file; it must be specified the mode (now it is set to read only existing files)
	fRes = f_open(&fObj, nfile_check, FA_OPEN_EXISTING);
	
	//	Check error
	if (fRes != FR_OK)
	{
		if (fRes == FR_NOT_READY)
		{
			if (sdDetect())
				setSdErr(SD_NOT_READY);
			else
				setSdErr(SD_NOT_PRESENT);
		}
		else
			setSdErr(SD_GENERIC_ERROR);
		return FALSE;
	}
	else 
	{
		fileClose(&fObj);
		setSdErr(0);
		return TRUE;
	}
}

/**
 *	Provides file size (in bytes)
 *
 *	\param char* nfile_size - char[] with filename to check
 *	\return	DWORD >=0 - file size
 *	\return -1 - Operation failed
 */
DWORD SDFileSize(char *nfile_size)
{
	DWORD size = -1;
	//	Open file; it must be specified the mode (now it is set to read only existing files)
	fRes = f_open(&fObj, nfile_size, FA_OPEN_EXISTING);

	//	Check error
	if (fRes != FR_OK)
	{
		if (fRes == FR_NOT_READY)
		{
			if (sdDetect())
				setSdErr(SD_NOT_READY);
			else
				setSdErr(SD_NOT_PRESENT);
		}
		else
			setSdErr(SD_GENERIC_ERROR);
		return size;
	}
	else 
	{
		size = fObj.fsize;
		fileClose(&fObj);
		setSdErr(0);
		return size;
	}
}

/**
 *	Provides file date/time timestamp 
 *
 *	\param char* nfile_tm - char[] with filename to check
 *	\param struct tm* SDtime - struct tm to fill with date and time of file timestamp 
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDFileDateTime(char* nfile_tm, struct tm* SDtime)
{
#if _FS_MINIMIZE != 0
	return FALSE;
#else
	char pathTemp[_MAX_LFN];
	
	SDDirGet(pathTemp, _MAX_LFN);
	
	int tempPathLen = strlen(pathTemp);
	if(pathTemp[tempPathLen-1] != '/')
		strcat(pathTemp, "/");
	strcat(pathTemp, nfile_tm);
	
	SDtime->tm_hour = 0;
   	SDtime->tm_min = 0;
   	SDtime->tm_sec = 0;
   	SDtime->tm_mday = 0;
   	SDtime->tm_mon = 0;
   	SDtime->tm_year = 0;
   	SDtime->tm_wday = 0;
   	SDtime->tm_yday = 0;
   	SDtime->tm_isdst = 0;
   	
	#ifndef USE_RTCC_LIB
		return FALSE;
	#endif
	
	//	Get file status
	fRes = f_stat(pathTemp, &fInfoObj);
	
	//	Check error
	if (fRes != FR_OK)
	{
		if (fRes == FR_NOT_READY)
		{
			if (sdDetect())
				setSdErr(SD_NOT_READY);
			else
				setSdErr(SD_NOT_PRESENT);
		}
		else
			setSdErr(SD_GENERIC_ERROR);
		return FALSE;
	}
	else 
	{
		WORD thisDate = fInfoObj.fdate;
		WORD thisTime = fInfoObj.ftime;
		
		SDtime->tm_sec = (int)((thisTime & 0x1F)*2);
	   	SDtime->tm_min = (int)((thisTime >> 5)&0x3F);
	   	SDtime->tm_hour = (int)((thisTime >> 11)&0x1F);
		
	   	SDtime->tm_mday = (int)(thisDate & 0x1F);
	   	SDtime->tm_mon = (int)((thisDate >> 5)&0x0F);
	   	SDtime->tm_year = (int)(((thisDate >> 9)&0x7F)+80);
	   	
		setSdErr(0);
		return TRUE;
	}
#endif
}

/**
 *	Creates/overwrites a file on current directory
 *
 *	\param char* nfile_create - char[] with filename to create
 *	\return	TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDFileCreate(char *nfile_create)
{
#if _FS_READONLY
	return FALSE;
#else
	if (fObj.flag == 0)
	{
		fRes = f_open(&fObj, nfile_create, FA_CREATE_ALWAYS|FA_WRITE);
		
		if (fRes != FR_OK)
		{
			errManager();
			return FALSE;
		}
		fileClose(&fObj);
		return TRUE;
	}
	else
		return FALSE;
#endif
}

/**
 *	Deletes a file from current directory
 *
 *	\param char* nfile_delete - char[] with filename to delete
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDFileDelete(char* nfile_delete)
{
#if ((_FS_MINIMIZE != 0)||(_FS_TINY == 1)||(_FS_READONLY == 1))
	return FALSE;
#else
	if (fObj.flag == 0)
	{
		fRes = f_unlink(nfile_delete);
		
		if(fRes != FR_OK)
		{
			errManager();
			return FALSE;
		}
		else
			return TRUE;
	}
	else
		return FALSE;	
#endif
}

/**
 *	Appends data on a file on current directory
 *
 *	\param char* nfile_append - char[] with filename to create
 *	\param char* buff_to_app - char[] with data to write
 *	\param unsigned int to_write - number of chars to write
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDFileAppend(char *nfile_append, char *buff_to_app, unsigned int to_write)
{
#if _FS_READONLY
	return FALSE;
#else
	unsigned int len;
	if (fObj.flag == 0)
	{
		//	File opening
		fRes = f_open(&fObj, nfile_append, FA_OPEN_EXISTING|FA_WRITE);
		if (fRes != FR_OK)
		{
			errManager();
			return FALSE;
		}	
		
		//	Seek to the end of file
		fRes = f_lseek(&fObj, f_size(&fObj));
		if (fRes != FR_OK)
		{
			errManager();
			fileClose(&fObj);
			return FALSE;		
		}
		
		//	Append of the text at the end of file
		fRes = f_write(&fObj, buff_to_app, to_write, &len);
		if ( (fRes != FR_OK) || (len != to_write) )
		{
			SDdebug("ERROR on write\n");
			errManager();
			fileClose(&fObj);
			return FALSE;
		}
		fileClose(&fObj);
		return TRUE;
	}
	else
		return FALSE;
#endif
}

/**
 * Reads data form a file
 *
 *	\param char* nfile_read - name of file to be read
 *	\param char* destBuff - buffer to fill with data read
 *	\param unsigned long to_read - amount of data to read from file
 *	\return WORD - amount of data read
 */
WORD SDFileRead(char* nfile_read, char* destBuff, unsigned long to_read)
{
	unsigned int len;
	if (fObj.flag == 0)
	{
		//	File opening
		fRes = f_open(&fObj, nfile_read, FA_OPEN_EXISTING|FA_READ);
		if (fRes != FR_OK)
		{
			errManager();
			return 0;
		}
		
		//	Read text from file
		fRes = f_read(&fObj, destBuff, to_read, &len);
		if ( (fRes != FR_OK) || (len != to_read) )
		{
			SDdebug("ERROR on SDFileRead\n");
			errManager();
			fileClose(&fObj);
			return len;
		}
		fileClose(&fObj);

		return len;
	}
	else
		return 0;
}

/**
 * Reads data form a file starting form a position
 *
 *	\param char* nfile_read - name of file to be read
 *	\param char* destBuff - buffer to fill with data read
 *	\param unsigned long to_read - amount of data to read from file
 *	\param unsigned long start - starting position (absolute position from, 0 is first data of file)
 *	\return WORD - amount of data read
 */
WORD SDFileReadAt(char* nfile_read, char* destBuff, unsigned long to_read, unsigned long start)
{
	unsigned int len;
	if (fObj.flag == 0)
	{
		//	File opening
		fRes = f_open(&fObj, nfile_read, FA_OPEN_EXISTING|FA_READ);
		if (fRes != FR_OK)
		{
			errManager();
			return 0;
		}
		
		//	Seek to the end of file
		fRes = f_lseek(&fObj, start);
		if (fRes != FR_OK)
		{
			errManager();
			fileClose(&fObj);
			return 0;		
		}
		
		//	Read text from file
		fRes = f_read(&fObj, destBuff, to_read, &len);
		if ( (fRes != FR_OK) || (len != to_read) )
		{
			SDdebug("ERROR on SDFileReadAt\n");
			errManager();
			fileClose(&fObj);
			return len;
		}
		fileClose(&fObj);
		return len;
	}
	else
		return 0;
}

/**
 * Renames a file to new name.
 *
 *	\param char* nfile_to_change - original file name
 *	\param char* nfile_new_name - new file name or file path/name
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDFileRename(char* nfile_to_change, char* nfile_new_name)
{
#if ((_FS_MINIMIZE != 0)||(_FS_TINY == 1)||(_FS_READONLY == 1))
	return FALSE;
#else
	if (fObj.flag == 0)
	{
		fRes = f_rename(nfile_to_change, nfile_new_name);
		if(fRes != FR_OK)
		{
			SDdebug("ERROR on SDFileRename\n");
			errManager();
			return FALSE;
		}
		return TRUE;
	}
	else
		return FALSE;
#endif	
}

/**
 *  Moves a file to another directory
 *
 *	\param char* nfile_to_change - original file name
 *	\param char* npath_to_use - new directory path for (use SDDirGet to retrieve current directory)
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDFileMove(char* nfile_to_move, char* npath_to_use)
{
	int pathLen = strlen(nfile_to_move) + strlen(npath_to_use) + 3;
	if(pathLen > _MAX_LFN)
		return FALSE;
	
	char pathTemp[_MAX_LFN];
	strcat(pathTemp, "/");
	strcat(pathTemp, npath_to_use);
	strcat(pathTemp, "/");
	strcat(pathTemp, nfile_to_move);
	
	return SDFileRename(nfile_to_move, pathTemp);
}

/**
 *	Gets current directory path 
 *
 *	\param char* ndir_get - char[] to fill with path
 *	\param int size_max - ndir_get max array size to prevent overflows
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDDirGet(char* ndir_get, int size_max)
{
#if ((_FS_MINIMIZE > 1)||(_FS_RPATH < 2))
	return FALSE;
#else
	fRes = f_getcwd(ndir_get, size_max);
	
	int t;
	int ml = strlen(ndir_get);
	if(ml > 1)
	{
		for (t = 0; t < ml; t++)
			ndir_get[t] = ndir_get[t+2];
		ndir_get[ml-1] = '\0';
		ndir_get[ml] = '\0';	
	}
	
	if(fRes != FR_OK)
	{
		errManager();
		return FALSE;
	}
	else
		return TRUE;
#endif
}

/**
 *	Changes current directory
 *
 *	\param char* ndir_chg - char[] with directory to change
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDDirChange(char* ndir_chg)
{
#if (_FS_RPATH < 1)
	return FALSE;
#else
	fRes = f_chdir(ndir_chg);
	
	if(fRes != FR_OK)
	{
		errManager();
		return FALSE;
	}
	return TRUE;
#endif
}

/**
 *	Moves to parent directory (if possible)
 *
 *	\param None
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDDirBack()
{
	char backDir[3] = "..";
	return SDDirChange(backDir);
}

/**
 *	Creates a subdirectory inside current directory
 *
 *	\param char* ndir_create - char[] with directory name to create
 *	\return	TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDDirCreate(char* ndir_create)
{
#if ((_FS_MINIMIZE != 0)||(_FS_TINY == 1)||(_FS_READONLY == 1))
	return FALSE;
#else
	fRes = f_mkdir(ndir_create);
	
	if(fRes != FR_OK)
	{
		errManager();
		return FALSE;
	}
	return TRUE;
#endif
}

/**
 *	Deletes a subfolder from current directory
 *
 *	\param char* ndir_delete - char[] with directory name to delete
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDDirDelete(char* ndir_delete)
{
#if ((_FS_MINIMIZE != 0)||(_FS_TINY == 1)||(_FS_READONLY == 1))
	return FALSE;
#else
	fRes = f_unlink(ndir_delete);
	
	if(fRes != FR_OK)
	{
		errManager();
		return FALSE;
	}
	return TRUE;
#endif
}

/**
 * Renames a directory. Can be used also to move directory to other path
 *
 *	\param char* ndir_to_change - original directory name
 *	\param char* ndir_new_name - new directory name or path/name
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDDirRename(char* ndir_to_change, char* ndir_new_name)
{
	return SDFileRename(ndir_to_change, ndir_new_name);
}

/**
 *	Provides End Of File for opened stream file
 *
 *	\param FIL* fobj - file object to be used (NOTE: remember to use '&' operator)
 *	\return TRUE - EOF reached
 *  \return FALSE - EOF not reached
 */
BOOL SDStreamEOF(FIL* fobj)
{
	return f_eof(fobj);
}

/**
 *	Opens stream file
 *
 *	\param FIL* fobj - file object to be used (NOTE: remember to use '&' operator)
 *	\param char* nfileRead - name of the file to be opened
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDStreamOpen(FIL* fobj, char *nfileRead)
{
	BYTE openMode;
#if _FS_READONLY
	openMode = FA_OPEN_EXISTING|FA_READ;
#else
	openMode = FA_OPEN_ALWAYS|FA_READ|FA_WRITE;
#endif
	if (fobj->flag == 0)
	{
		fRes = f_open(fobj, nfileRead, openMode);
		if (fRes != FR_OK)
		{
			errManager();
			return FALSE;
		}
		return TRUE;
	}
	else
		return FALSE;
}

/**
 *	Closes stream file
 *
 *	\param FIL* fobj - file object to be used (NOTE: remember to use '&' operator)
 *	\return TRUE - operation success
 *	\return FALSE - operation failed
 */
BOOL SDStreamClose(FIL* fobj)
{
#if _FS_READONLY
	if ((fobj->flag && FA_OPEN_EXISTING)||(fobj->flag && FA_READ))
#else
	if ((fobj->flag && FA_OPEN_ALWAYS)||(fobj->flag && FA_READ)||(fobj->flag && FA_WRITE))
#endif
	{
		fObjClose(fobj);
		return TRUE;
	}
	return FALSE;		
}

/**
 *	Reads data stream from file
 *
 *	\param FIL* fobj - file object to be used (NOTE: remember to use '&' operator)
 *	\param char* destBuff - buffer to fill with data read
 *	\param unsigned long len - amount of data to read from file
 *	\return WORD - amount of data read
 */
WORD SDStreamRead(FIL* fobj, char *destBuff, unsigned long len)
{
	if ((fobj->flag && FA_READ) != 0)
	{
		unsigned int bRead = 0;
		fRes = f_read(fobj, destBuff, len, &bRead);
		if (fRes != FR_OK)
		{
			errManager();
			return 0;
		}	
		if (bRead < len)
			streamEof = TRUE;
		return bRead;
	}
	else 
		return 0;	
}

/**
 * Read data stream starting form a provided position
 *
 *	\param FIL* fobj - file object to be used (NOTE: remember to use '&' operator)
 *	\param char* destBuff - buffer to fill with data read
 *	\param unsigned long len - amount of data to read from file
 *	\param unsigned long start - starting position (absolute position from, 0 is first data of file)
 *	\return WORD - amount of data read
 */
WORD SDStreamReadAt(FIL* fobj, char* destBuff, unsigned long len, unsigned long start)
{
	if ((fobj->flag && FA_READ) != 0)
	{
		unsigned int bRead = 0;
		fRes = f_lseek(fobj, (DWORD)start);
		if(fRes != FR_OK)
		{
			errManager();
			return 0;
		}
		
		fRes = f_read(fobj, destBuff, len, &bRead);
		if (fRes != FR_OK)
		{
			errManager();
			return 0;
		}	
		if (bRead < len)
			streamEof = TRUE;
		return bRead;
	}
	else 
		return 0;
}

/**
 *	Writes (appends) data stream to a file
 *
 *	\param FIL* fobj - file object to be used (NOTE: remember to use '&' operator)
 *	\param char* srcBuff - buffer to fill with data read
 *	\param unsigned long to_write - amount of data to write on file
 *	\return WORD - amount of data written
 */
WORD SDStreamWrite(FIL* fobj, char *srcBuff, unsigned long to_write)
{
#if _FS_READONLY
	return FALSE;
#else
	unsigned int len;
	BOOL streamResult;
	if ((fobj->flag && FA_WRITE) != 0)
	{
		//	Seek to the end of file
		streamResult = f_lseek(fobj, f_size(fobj));
		if (streamResult != FR_OK)
		{
			errManager();
			fileClose(fobj);
			return 0;		
		}
		
		//	Append of the text at the end of file
		streamResult = f_write(fobj, srcBuff, to_write, &len);
		if ( (streamResult != FR_OK) || (len != to_write) )
		{
			SDdebug("ERROR on write\n");
			errManager();
			fileClose(fobj);
			return 0;
		}
		else
			return len;
	}
	else
		return 0;
#endif
}

/// @cond debug
/**
 *	Mount the SD module. Internal static funtcion not to call from main 
 *	application.
 *
 *	\return TRUE - the initialization succeded
 *	\return FALSE - the initialization doesn't succeded (SD card not inserted?)
 */
static BOOL diskMount()
{	
	if ( f_mount(0, &Fatfs) == FR_OK)
	{
		disk_initialize(0);
		return TRUE;
	}
	else
		return FALSE;
}


/**
 *	Unmount a previously mounted SD module. Internal static funtcion not to 
 *	call from main application.
 *
 *	\return TRUE - the unmount succeded
 *	\return FALSE - the unmount doesn't succeded (SD card not mounted?)
 *****************************************************************************/	
static BOOL diskUnmount()
{
	if ( f_mount(0, NULL) == FR_OK)
	{
		return TRUE;
	}
	else
		return FALSE;
}


/**
 *	Closes a previously open file. Internal static function not to use in main 
 *	application.
 *	\param FIL* file_close - file object to close
 */
static void fileClose(FIL *file_close)
{
	f_close(file_close);
	fileInit();
}

/**
 * Initializes static file object
 */
static void fileInit()
{
	fObj.fs = NULL;
	fObj.id = 0;
	fObj.flag = 0;
	fObj.pad1 = 0;
	fObj.fptr = 0;
	fObj.fsize = 0;
	fObj.sclust = 0;
	fObj.clust = 0;
	fObj.dsect = 0;
}

/**
 * Closes a Stream File Object
 * \param FIL* fobj - file object to close and initialize
 */
static void fObjClose(FIL* fobj)
{
	f_close(fobj);
	fObjInit(fobj);
}

/**
 * Initializes a Stream File Object
 * \param FIL* fobj - file object to initialize/reset
 */
static void fObjInit(FIL* fobj)
{	
	fobj->fs = NULL;
	fobj->id = 0;
	fobj->flag = 0;
	fobj->pad1 = 0;
	fobj->fptr = 0;
	fobj->fsize = 0;
	fobj->sclust = 0;
	fobj->clust = 0;
	fobj->dsect = 0;
}

void SDdebug(char* str)
{
#ifdef _DEBUG
	UARTWrite(1, str);
#endif
}

/// @endcond

/**
 * Starts a scan of the folder tree starting from the location passed as parameter
 * \param char* path - path from which to start the scan process
 * \return int - 0 if no error, -1 otherwise
 */
int SDStartScan (char * path)
{
	int retval;
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    workfno.lfname = lfn;
    workfno.lfsize = sizeof lfn;
#endif

	fRes = f_opendir(&workdir, path);                       /* Open the directory */
	if (fRes != FR_OK)
		retval = _SD_ERR;
	else
	{
		scan_started = TRUE;
		retval = 0;
	}
		
	return retval;		
}


/**
 * Pulls results from a folder scan started with SDStartScan. Results are saved into the char array passed by parameter, while the return value depicts the kind of result or error/end_of_list conditions.
 * \param char* item - pointer to a string to hold the name of the file or folder
 * \return int - 1 File, 2 Directory, -1 error, -2 end of list
 */
int SDScanResults ( char *item)
{				
	int retval;
	BOOL repeat;
	
	do
	{
		repeat = FALSE;
		if (scan_started)
		{
			fRes = f_readdir(&workdir, &workfno);	// Read a directory item */
			// Break on error or end of dir
			if (fRes != FR_OK)
			{
				scan_started = FALSE;
				return _SD_ERR;
			}
			if (workfno.fname[0] == 0)					
			{
				scan_started = FALSE;
				return _SD_END;
			}

			if (workfno.fname[0] == '.')
			{
				repeat = TRUE;
			}
			else
			{
				#if _USE_LFN
				strcpy(item, *workfno.lfname ? workfno.lfname : workfno.fname);
				//fn = *fno.lfname ? fno.lfname : fno.fname;
				#else
				strcpy(item, workfno.name);
				//fn = fno.fname;
				#endif
				if (workfno.fattrib & AM_DIR)// It is a directory
					retval = _SD_DIR;
				else                         // It is a file.
					retval = _SD_FILE;
				return retval;
			}
		}
		else
			return _SD_ERR;
	}while(repeat);
	return _SD_ERR;
}


/** @} */
/** @} */
