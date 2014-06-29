#include "pic24f.h"
#include "diskio.h"
#include "ff.h"
#include "ffhelper.h"
#include <string.h>
#include "SD_HW.h"
#include <time.h>

#ifndef __F_MAN_H
#define __F_MAN_H

//#define _DEBUG

#define SD_GROVE		1
#define	SD_MUSIC		2
#define _SD_END		-2
#define _SD_ERR		-1
#define	_SD_FILE	1
#define _SD_DIR		2


// INIT & TOOLS
BOOL SDInit(int pin_sck, int pin_si, int pin_so, int pin_cs, int pin_cd, BYTE timeout);
BOOL SDOnNest(int _nest, BYTE _nTimeout);
BOOL SDUnMount();
void SDdebug(char* str);
int SDGetErr();
DWORD SDFreeSpace();

// FILE
BOOL SDFileCheck(char* nfile_check);
DWORD SDFileSize(char* nfile_size);
BOOL SDFileDateTime(char* nfile_tm, struct tm* SDtime);
BOOL SDFileCreate(char* nfile_create);
BOOL SDFileDelete(char* nfile_delete);
BOOL SDFileAppend(char* nfile_append, char *buff_to_app, unsigned int to_write);
WORD SDFileRead(char* nfile_read, char* destBuff, unsigned long to_read);
WORD SDFileReadAt(char* nfile_read, char* destBuff, unsigned long to_read, unsigned long start);
BOOL SDFileRename(char* nfile_to_change, char* nfile_new_name);
BOOL SDFileMove(char* nfile_to_move, char* npath_to_use);

// DIRECTORY
BOOL SDDirGet(char* ndir_get, int size_max);
BOOL SDDirChange(char* ndir_chg);
BOOL SDDirBack();
BOOL SDDirCreate(char* ndir_create);
BOOL SDDirDelete(char* ndir_delete);
BOOL SDDirRename(char* ndir_to_change, char* ndir_new_name);


// STREAM
BOOL SDStreamOpen(FIL* fobj, char* nfileRead);
WORD SDStreamRead(FIL* fobj, char* destBuff, unsigned long len);
WORD SDStreamReadAt(FIL* fobj, char* destBuff, unsigned long len, unsigned long start);
WORD SDStreamWrite(FIL* fobj, char *srcBuff, unsigned long to_write);
BOOL SDStreamClose(FIL* fobj);
BOOL SDStreamEOF(FIL* fobj);


// SCAN
int SDStartScan (char * path);
int SDScanResults ( char *item);

#endif
