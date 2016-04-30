#include <stdio.h>
#include <string.h>
#include <sys/process.h>
#include <sysutil/sysutil_syscache.h>

int32_t main()
{
	// Disable printf buffering
	setbuf(stdout, NULL);

	printf("TEST00003 by tambre.\n");
	printf("Removing cache before mounting and then mounting the cache.\n\n");

	printf("Mounting system cache:\n");
	CellSysCacheParam param;
	strcpy(param.cacheId, "TEST00003");
	param.reserved = NULL;

	int32_t ret = cellSysCacheMount(&param);

	if (ret < 0)
	{
		printf("cellSysCacheMount() error code: 0x%x\n", ret);
		sys_process_exit(1);
	}
	else if (ret == CELL_SYSCACHE_RET_OK_CLEARED)
	{
		printf("cellSysCacheMount(): CELL_SYSCACHE_RET_OK_CLEARED.\n\n");
	}
	else if (ret == CELL_SYSCACHE_RET_OK_RELAYED)
	{
		printf("cellSysCacheMount(): CELL_SYSCACHE_RET_OK_RELAYED.\n\n");
	}

	printf("Deleting system cache:\n");

	ret = cellSysCacheClear();

	if (ret < 0)
	{
		printf("cellSysCacheClear() error code: 0x%x\n", ret);
		sys_process_exit(1);
	}
	else
	{
		printf("cellSysCacheClear() returned CELL_OK.\n");
	}

	return 0;
}