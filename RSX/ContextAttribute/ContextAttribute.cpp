#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <sys/syscall.h>
#include <cell/gcm.h>

typedef int32_t s32;
typedef uint32_t u32;

#define HOST_SIZE (1024 * 1024)

int main(void)
{
	printf("TEST00005 by tambre.\n");
	printf("Getting different context attribute values.\n\n");

	printf("Initializing.\n");
	void* host_addr = memalign(1024 * 1024, HOST_SIZE);
	printf("host_addr: 0x%x\n", host_addr);

	s32 ret = cellGcmInit(CELL_GCM_IO_PAGE_SIZE, HOST_SIZE, host_addr);

	if (ret != CELL_OK)
	{
		printf("cellGcmInit() failed (0x%x)\n", ret);
		sys_process_exit(1);
	}
	else
	{
		printf("cellGcmInit() succeeded\n\n");
	}

	ret = system_call_6(SYS_RSX_CONTEXT_ATTRIBUTE, 0x55555555, 0x106, 0x1, 0x0, 0x0, 0x0);

	return (0);
}
