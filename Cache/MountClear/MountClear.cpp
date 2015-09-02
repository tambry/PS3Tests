#include <stdio.h>
#include <string.h>
#include <sysutil/sysutil_syscache.h>

typedef int32_t s32;

int main(void)
{
	printf("TEST00003 by tambre.\n");
	printf("Removing cache before mounting and then mounting the cache.\n\n");

	printf("Mounting system cache:\n");
	CellSysCacheParam param;
	strcpy(param.cacheId, "TEST00003");
	param.reserved = NULL;

	s32 ret = cellSysCacheMount(&param);
	if (ret < 0)
	{
		printf("cellSysCacheMount() returned error code: 0x%x\n\n", ret);
	}
	else if (ret == CELL_SYSCACHE_RET_OK_CLEARED)
	{
		printf("cellSysCacheMount() returned: CELL_SYSCACHE_RET_OK_CLEARED.\n\n");
	}
	else if (ret == CELL_SYSCACHE_RET_OK_RELAYED)
	{
		printf("cellSysCacheMount() returned: CELL_SYSCACHE_RET_OK_RELAYED.\n\n");
	}

	printf("Deleting system cache:\n");

	ret = cellSysCacheClear();
	if (ret < 0)
	{
		printf("cellSysCacheClear() returned error code: 0x%x\n", ret);
	}
	else
	{
		printf("cellSysCacheClear() returned CELL_OK.\n");
	}

	return(0);
}