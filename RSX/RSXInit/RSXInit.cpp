#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <cell/gcm.h>

#define HOST_SIZE (1024 * 1024)

int main(void)
{
	// Disable printf buffering
	setbuf(stdout, NULL);

	printf("TEST00006 by tambre.\n");
	printf("Initializing GCM.\n\n");

	void* host_addr = memalign(1024 * 1024, HOST_SIZE);
	printf("host_addr: 0x%x\n", host_addr);

	int32_t ret = cellGcmInit(CELL_GCM_IO_PAGE_SIZE, HOST_SIZE, host_addr);

	if (ret != CELL_OK)
	{
		printf("cellGcmInit() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}
	else
	{
		printf("cellGcmInit() succeeded\n\n");
	}

	return (0);
}