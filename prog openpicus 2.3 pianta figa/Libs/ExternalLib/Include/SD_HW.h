#include "HWlib.h"

#define SD_SUCCESS			0
#define	SD_FS_NOT_INIT		20
#define	SD_NOT_PRESENT		21
#define SD_NOT_READY		22
#define SD_FILE_NOT_FOUND	23
#define SD_GENERIC_ERROR	24


void CS_LOW();
void CS_HIGH();
void MMC_SEL();
void pinConfig(int _sdsc, int _sdsi, int _sdso, int _sdcs, int _sddet);
BOOL sdDetect();
