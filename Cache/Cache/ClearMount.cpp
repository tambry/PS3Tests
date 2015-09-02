#include <stdio.h>
#include <string.h>
#include <sysutil/sysutil_syscache.h>

typedef int32_t s32;

int main(void)
{
	printf("TEST00001 by tambre.\n");
	printf("Removing cache before mounting and then mounting the cache.\n\n");

	printf("Trying to delete system cache before mounting:\n");

	s32 ret = cellSysCacheClear();
	if (ret < 0)
	{
		printf("cellSysCacheClear() returned error code: 0x%x\n\n", ret);
	}
	else
	{
		printf("cellSysCacheClear() returned CELL_OK.\n\n");
	}

	printf("Mounting system cache:\n");
	CellSysCacheParam param;
	strcpy(param.cacheId, "TEST00001");
	param.reserved = NULL;

	ret = cellSysCacheMount(&param);
	if (ret < 0)
	{
		printf("cellSysCacheMount() returned error code: 0x%x\n", ret);
	}
	else if (ret == CELL_SYSCACHE_RET_OK_CLEARED)
	{
		printf("cellSysCacheMount() returned: CELL_SYSCACHE_RET_OK_CLEARED.\n");
		printf("getCachePath: %s\n", param.getCachePath);
	}
	else if (ret == CELL_SYSCACHE_RET_OK_RELAYED)
	{
		printf("cellSysCacheMount() returned: CELL_SYSCACHE_RET_OK_RELAYED.\n");
		printf("getCachePath: %s\n", param.getCachePath);
	}

	return(0);
}