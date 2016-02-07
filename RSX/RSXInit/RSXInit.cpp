#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <cell/gcm.h>

typedef int32_t s32;
typedef uint32_t u32;

#define HOST_SIZE (1024 * 1024)

int main(void)
{
	printf("TEST00006 by tambre.\n");
	printf("Initializing GCM.\n\n");

	void* host_addr = memalign(1024 * 1024, HOST_SIZE);
	printf("host_addr: 0x%x\n", host_addr);

	s32 ret = cellGcmInit(CELL_GCM_IO_PAGE_SIZE, HOST_SIZE, host_addr);

	if (ret != CELL_OK)
	{
		printf("cellGcmInit() error code: 0x%x)\n", ret);
		sys_process_exit(1);
	}
	else
	{
		printf("cellGcmInit() succeeded\n\n");
	}

	return (0);
}