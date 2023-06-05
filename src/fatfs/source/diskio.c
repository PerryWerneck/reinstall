/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32

	#include <windows.h>

#else

	#include <unistd.h>

#endif // _WIN32


#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#define SECTOR_LENGTH 512


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static int disks[FF_VOLUMES] = { -1 };

#define MAX_DISKS (sizeof(disks)/sizeof(disks[0]))

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if(pdrv > MAX_DISKS || disks[pdrv] < 0) {
		return STA_NODISK;
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive n to identify the drive */
) {

	if(pdrv > MAX_DISKS) {
		return STA_NODISK;
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if(pdrv > MAX_DISKS || disks[pdrv] < 0) {
		return STA_NODISK;
	}

	size_t offset = ((size_t) sector) * SECTOR_LENGTH;
	size_t length = ((size_t) count) * SECTOR_LENGTH;
	while(length > 0) {

		ssize_t bytes = pread(disks[pdrv], buff, length, offset);
		if(bytes < 1) {
			return RES_ERROR;
		}

		length -= bytes;
		offset += bytes;
		buff += bytes;

	}

	return RES_OK;

}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if(pdrv > MAX_DISKS || disks[pdrv] < 0) {
		return STA_NOINIT;
	}

	size_t offset = ((size_t) sector) * SECTOR_LENGTH;
	size_t length = ((size_t) count) * SECTOR_LENGTH;
	while(length > 0) {

		ssize_t bytes = pwrite(disks[pdrv], buff, length, offset);
		if(bytes < 1) {
			return RES_ERROR;
		}

		length -= bytes;
		offset += bytes;
		buff += bytes;

	}

	return RES_OK;

}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if(pdrv > MAX_DISKS) {
		return STA_NODISK;
	}

	if(cmd == CTRL_FORMAT) {
		disks[pdrv] = *((int *) buff);
		return RES_OK;
	}

	if(disks[pdrv] < 0) {
		return STA_NODISK;
	}

	switch(cmd) {
	case CTRL_SYNC:
		fsync(disks[pdrv]);
		break;

	case GET_SECTOR_COUNT:
		{
			struct stat st;
			if(fstat(disks[pdrv],&st) < 0) {
				return RES_ERROR;
			}
			*((UINT *) buff) = st.st_blocks;
		}
		break;

	case GET_SECTOR_SIZE:
	case GET_BLOCK_SIZE:
		*((UINT *) buff) = (UINT) 512;
		break;

	case CTRL_TRIM:
		break;

	default:
		return RES_PARERR;

	}

	return RES_OK;

}

DWORD get_fattime (void) {
    time_t t;
    struct tm *stm;


    t = time(0);
    stm = localtime(&t);

    return (DWORD)(stm->tm_year - 80) << 25 |
           (DWORD)(stm->tm_mon + 1) << 21 |
           (DWORD)stm->tm_mday << 16 |
           (DWORD)stm->tm_hour << 11 |
           (DWORD)stm->tm_min << 5 |
           (DWORD)stm->tm_sec >> 1;
}

