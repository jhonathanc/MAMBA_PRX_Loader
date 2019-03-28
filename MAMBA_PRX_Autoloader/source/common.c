#include <stdio.h>
#include <string.h>
#include <ppu-lv2.h>
#include <sys/systime.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "common.h"

#define FS_S_IFMT 0170000

#define VERSION_NAME "Unofficial MAMBA/PRX Autoloader v4.84.2 by Ps3ita Team (forked from NzV)"

//----------------------------------------
//LOG
//----------------------------------------

int verbose = 0;

#ifdef ENABLE_LOG

int fd_log = -1;
extern char s[64];

int WriteToLog(char *str)
{
	if(fd_log < 0 ) return FAILED;
	if(!str) return SUCCESS;
	u64 size = strlen(str);
	if(size == 0) return SUCCESS;
	u64 ret_size = 0;
	sprintf(str,"%s", str);
	if(sysLv2FsWrite(fd_log, str, size, &ret_size) || ret_size!=size)
	{
		return FAILED;
	}
	return SUCCESS;
}

void CloseLog()
{
	if (verbose) WriteToLog("-----[END]-----\n");
	if (verbose) WriteToLog("---[By NzV]---\n");
	verbose = 0;
	if(fd_log >= 0) sysLv2FsClose(fd_log);
	fd_log = -1;
}

int Open_Log(char *file)
{
	if(fd_log >= 0) return -666;
	if(!sysLv2FsOpen(file, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd_log, 0777, NULL, 0))
	{
		sysLv2FsChmod(file, FS_S_IFMT | 0777);
		if(WriteToLog(VERSION_NAME)!=SUCCESS) {CloseLog(); return FAILED;}
		WriteToLog("-----[LOG]-----\n");
		return SUCCESS;
	}
	fd_log = -1;
	verbose = 0;
	return FAILED;

}

#endif

//----------------------------------------
//COBRA/MAMBA
//----------------------------------------

#define SYSCALL8_OPCODE_GET_VERSION			0x7000ULL
#define SYSCALL8_OPCODE_GET_MAMBA			0x7FFFULL

int sys8_get_version(u32 *version)
{
	lv2syscall2(8, SYSCALL8_OPCODE_GET_VERSION, (u64)version);
	return_to_user_prog(int);
}

int sys8_get_mamba(void)
{
	lv2syscall1(8, SYSCALL8_OPCODE_GET_MAMBA);
	return_to_user_prog(int);
}

int is_cobra(void)
{
	u32 version = 0x99999999;
	if (sys8_get_version(&version) < 0)	return FAILED;
	if (version != 0x99999999 && sys8_get_mamba() != 0x666)	return SUCCESS;
	return FAILED;
}

int is_mamba(void)
{
	u32 version = 0x99999999;
	if (sys8_get_version(&version) < 0)	return FAILED;
	if (version != 0x99999999 && sys8_get_mamba() == 0x666)	return SUCCESS;
	return FAILED;
}

//----------------------------------------
//FILE UTILS
//----------------------------------------

sysFSStat stat1;

int file_exists(const char *path)
{
	int ret = sysLv2FsStat(path, &stat1);
	if(ret == SUCCESS && S_ISDIR(stat1.st_mode)) return FAILED;
	return ret;
}

int dir_exists(const char *path)
{
	int ret = sysLv2FsStat(path, &stat1);
	if(ret == SUCCESS && S_ISDIR(stat1.st_mode)) return SUCCESS;
	return FAILED;
}

int unlink_secure(void *path)
{
	if(file_exists(path)==SUCCESS)
	{
		sysLv2FsChmod(path, FS_S_IFMT | 0777);
		return sysLv2FsUnlink(path);
	}
	return FAILED;
}

//----------------------------------------
//POWER UTILS
//----------------------------------------


int sys_shutdown()
{
	unlink_secure("/dev_hdd0/tmp/turnoff");

	lv2syscall4(379,0x1100,0,0,0);
	return_to_user_prog(int);
}

int sys_reboot()
{
	unlink_secure("/dev_hdd0/tmp/turnoff");

	lv2syscall4(379,0x1200,0,0,0);
	return_to_user_prog(int);
}
