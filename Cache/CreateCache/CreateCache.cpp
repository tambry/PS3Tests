#include <stdio.h>
#include <string.h>
#include <sys/process.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <sysutil/sysutil_syscache.h>

typedef int32_t s32;

int main(void)
{
	printf("TEST00002 by tambre.\n");
	printf("Mounting cache and creating several folders.\n\n");

	printf("Mounting system cache:\n");
	CellSysCacheParam param;
	strcpy(param.cacheId, "TEST00002");
	param.reserved = NULL;

	s32 ret = cellSysCacheMount(&param);

	if (ret < 0)
	{
		printf("cellSysCacheMount() returned error code: 0x%x\n", ret);
		sys_process_exit(1);
	}
	else if (ret == CELL_SYSCACHE_RET_OK_CLEARED)
	{
		printf("cellSysCacheMount() returned: CELL_SYSCACHE_RET_OK_CLEARED.\n");
		printf("getCachePath: %s\n\n", param.getCachePath);
	}
	else if (ret == CELL_SYSCACHE_RET_OK_RELAYED)
	{
		printf("cellSysCacheMount() returned: CELL_SYSCACHE_RET_OK_RELAYED.\n");
		printf("getCachePath: %s\n\n", param.getCachePath);
	}

	printf("Creating 4 folders in the cache directory:\n");
	printf("Loading CELL_SYSMODULE_FS...\n");

	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	if (ret)
	{
		printf("cellSysmoduleLoadModule(CELL_SYSMODULE_FS) returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	printf("Creating 4 folders in %s\n", param.getCachePath);

	ret = cellFsMkdir("/dev_hdd1/TEST00002/", CELL_FS_DEFAULT_CREATE_MODE_1);

	if (ret < 0)
	{
		printf("Creation of folder /dev_hdd1/TEST00002/ failed.\n");
		printf("Error code: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellFsMkdir("/dev_hdd1/BOOL12345/", CELL_FS_DEFAULT_CREATE_MODE_1);

	if (ret < 0)
	{
		printf("Creation of folder /dev_hdd1/BOOL12345/ failed.\n");
		printf("Error code: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellFsMkdir("/dev_hdd1/RPCS34567/", CELL_FS_DEFAULT_CREATE_MODE_1);

	if (ret < 0)
	{
		printf("Creation of folder /dev_hdd1/RPCS34567/ failed.\n");
		printf("Error code: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellFsMkdir("/dev_hdd1/VERYLONGFOLDERNAME/", CELL_FS_DEFAULT_CREATE_MODE_1);

	if (ret < 0)
	{
		printf("Creation of folder /dev_hdd1/VERYLONGFOLDERNAME/ failed.\n");
		printf("Error code: 0x%x\n", ret);
		sys_process_exit(1);
	}

	return(0);
}