/*
 * fs.c
 *
 * Created: 2019/03/10
 * Copyright ? 2019 Kiyotaka Akasaka. All rights reserved.
 */ 
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <samd51_error.h>
#include <samd51_clock.h>
#include <samd51_sdhc.h>

#include "include/internal/clock.h"
#include "include/internal/ff.h"

#include "include/ai_mini4wd_fs.h"
#include "include/ai_mini4wd_timer.h"

#define MAX_FILES		(4)

#pragma pack(1)
typedef struct AiMini4wdFilePointers_t
{
	volatile int used;
	FIL file;
} AiMini4wdFilePointers;

static FATFS sFatFs;

static FIL sFiles[MAX_FILES];
static int sUsed[MAX_FILES];


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static AiMini4wdFile *_getUnusedFileDescriptor(void)
{
	for (int i=0 ; i<MAX_FILES ; ++i) {
		if (sUsed[i]== 0) {
			sUsed[i] = 1;

			return (AiMini4wdFile *)&(sFiles[i]);
		}
	}
	
	return NULL;
}

/*--------------------------------------------------------------------------*/
static void _destroyFileDescriptor(AiMini4wdFile *file)
{
	for (int i=0 ; i<MAX_FILES ; ++i) {
		if (((AiMini4wdFile *)&(sFiles[i])) == file) {
			sUsed[i] = 0;
		}
	}

	return;
}

/*--------------------------------------------------------------------------*/
static int _aiMini4wdFsErrorCodeConvert(FRESULT res)
{
	static int errorCodeTable[] =
	{
		AI_OK,
		AI_ERROR_FS_DISK,
		AI_ERROR_FS_INTERNAL,
		AI_ERROR_FS_NOT_READY,
		AI_ERROR_FS_NO_FILE,
		AI_ERROR_FS_NO_PATH,
		AI_ERROR_FS_INVALID_NAME,
		AI_ERROR_FS_DENIED,
		AI_ERROR_FS_EXIST,
		AI_ERROR_FS_INVALID_OBJECT,
		AI_ERROR_FS_WRITE_PROTECRTED,
		AI_ERROR_FS_INVALID_DRIVE,
		AI_ERROR_FS_NOT_ENABLED,
		AI_ERROR_FS_NO_FILESYSTEM,
		AI_ERROR_FS_MKFS_ABORTED,
		AI_ERROR_FS_TIMEOUT,
		AI_ERROR_FS_LOCKED,
		AI_ERROR_FS_NOT_ENOUGH_CORE,
		AI_ERROR_FS_TOO_MANY_OPEN_FILES,
		AI_ERROR_FS_INVALID_PARAMETER
	};

	if ((int)res >= (sizeof(errorCodeTable) / sizeof(errorCodeTable[0]))) {
		return AI_ERROR_FS_UNKNOWN;
	}

	return errorCodeTable[(int)res];
}


/*--------------------------------------------------------------------------*/
int aiMini4wdFsInitialize(void)
{
	//J 一旦すべて初期化する
	memset ((void *)sFiles, 0, sizeof(AiMini4wdFilePointers)*4);	
	
	//J SDC 初期化
	samd51_mclk_enable(SAMD51_AHB_SDHCn0, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_SDHC0, LIB_MINI_4WD_CLK_GEN_NUMBER_48MHZ);
	
	samd51_sdhc_initialize(SAMD51_SDHC0);

	volatile uint32_t tick = aiMini4WdTimerGetSystemtick();
	while ((tick + 2000) > aiMini4WdTimerGetSystemtick());

	FRESULT res = f_mount(&sFatFs, "", 1);

	return _aiMini4wdFsErrorCodeConvert(res);	
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
AiMini4wdFile *aiMini4wdFsOpen(const char *path, const char *mode)
{
	if (mode == NULL) {
		return NULL;
	}
	
	//J オプションをFatFSに合わせて付け替える
	uint8_t mode_b = 0;
	for (int i=0 ; i<strlen(mode) ; ++i) {
		switch (mode[i]) {
		case 'a':
			mode_b |= FA_OPEN_APPEND | FA_WRITE;
			break;
		case 'w':
			mode_b |= FA_WRITE | FA_OPEN_ALWAYS;
			break;
		case 'r':
			mode_b |= FA_READ;
			break;
		case '+':
			mode_b |= FA_WRITE | FA_READ;
			break;
		case 'x':
			mode_b &= ~FA_CREATE_ALWAYS;
			mode_b |= FA_CREATE_NEW;
		default:
			return NULL;
		}
	}

	FIL *fil = (FIL *)_getUnusedFileDescriptor();
	if (fil == NULL) {
		return NULL;
	}

	FRESULT res =  f_open(fil, path, mode_b);
	if (res != FR_OK) {
		_destroyFileDescriptor((AiMini4wdFile *)fil);
		return NULL;		
	}

	return (AiMini4wdFile *)fil;
}


/*--------------------------------------------------------------------------*/
void aiMini4wdFsClose(AiMini4wdFile *file)
{
	f_close((FIL*)file);
	_destroyFileDescriptor(file);	
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsRead(AiMini4wdFile *file, void *buf, size_t size)
{
	UINT read_size = 0;
	FRESULT res = f_read((FIL*)file, buf, size, &read_size);
	if (res != FR_OK) {
		return _aiMini4wdFsErrorCodeConvert(res);
	}

	return (int)read_size;
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsWrite(AiMini4wdFile *file, const void *buf, size_t size)
{
	UINT write_size = 0;
	FRESULT res = f_write((FIL*)file, buf, size, &write_size);
	if (res != FR_OK) {
		return _aiMini4wdFsErrorCodeConvert(res);
	}

	return (int)write_size;
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsSeek(AiMini4wdFile *file, int32_t offset)
{
	FRESULT res = f_lseek((FIL*)file, offset);
	
	return _aiMini4wdFsErrorCodeConvert(res);
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsSize(AiMini4wdFile *file)
{
	return f_size((FIL*)file);
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsEof(AiMini4wdFile *file)
{
	return f_eof((FIL*)file);
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsSync(AiMini4wdFile *file)
{
	FRESULT ret = f_sync((FIL *)file);
	
	return _aiMini4wdFsErrorCodeConvert(ret);
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsTruncate(AiMini4wdFile *file)
{
	FRESULT ret = f_truncate((FIL *)file);
	return _aiMini4wdFsErrorCodeConvert(ret);
}

/*--------------------------------------------------------------------------*/
int aiMini4wdFsStat(const char *path, AiMini4wdFileInfo *info)
{
	if (info == NULL) {
		return AI_ERROR_NULL;
	}
	
	FILINFO fno;
	FRESULT res = f_stat(path, &fno);
	if (res != FR_OK) {
		return _aiMini4wdFsErrorCodeConvert(res);
	}

	info->attr = fno.fattrib;
	strncpy(info->name, fno.fname, sizeof(info->name));
	info->size = fno.fsize;
	
	return AI_OK;
}

/*--------------------------------------------------------------------------*/
extern char gCommonLineBuf[512];
int aiMini4wdFsPrintf(AiMini4wdFile *file, const char *str, ...)
{
    va_list ap;

    va_start( ap, str );
    int len = vsnprintf(gCommonLineBuf, sizeof(gCommonLineBuf), str, ap );
    va_end( ap );

	f_puts(gCommonLineBuf, (FIL*)file);

	return len;
}

/*--------------------------------------------------------------------------*/
static char sPutsBuf[2][512];
static int sActiveBufIndex = 0;
static size_t sBufferdSize = 0;
int aiMini4wdFsPuts(AiMini4wdFile *file, const char *str, size_t len)
{
	while (len > 0) {
		size_t cpySize = ((len + sBufferdSize) >  sizeof(sPutsBuf[sActiveBufIndex])) ? sizeof(sPutsBuf[sActiveBufIndex])-sBufferdSize : len;
		memcpy(&(sPutsBuf[sActiveBufIndex][sBufferdSize]), str, cpySize);		

		sBufferdSize += cpySize;
		if (sBufferdSize >= sizeof(sPutsBuf[sActiveBufIndex])) {
			sBufferdSize = 0;
			UINT written = 0;
			f_write((FIL*)file, sPutsBuf[sActiveBufIndex], sizeof(sPutsBuf[sActiveBufIndex]), &written);
//			f_sync((FIL*)file);

			sActiveBufIndex = 1-sActiveBufIndex;
		}
		str += cpySize;
		len -= cpySize;
	}
	
	return 0;
}

int aiMini4wdFsPutsFlush(AiMini4wdFile *file)
{
	UINT written = 0;
	f_write((FIL *)file, sPutsBuf[sActiveBufIndex], sBufferdSize, &written);
	
	sBufferdSize = 0;
	sActiveBufIndex = 1 - sActiveBufIndex;
	
	return 0;
}


/*--------------------------------------------------------------------------*/
char *aiMini4wdFsGets(AiMini4wdFile *file, char *buf, size_t len)
{
	return f_gets(buf, len, (FIL*)file);
}


