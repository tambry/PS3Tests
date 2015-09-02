#include <stdio.h>
#include <sys/process.h>
#include <cell/sysmodule.h>
#include <cell/fs/cell_fs_file_api.h>

typedef int32_t s32;

int main(void)
{
	printf("TEST00005 by tambre.\n");
	printf("Trying to open a file from USRDIR, without specifying a base path.\n\n");

	printf("Loading CELL_SYSMODULE_FS...\n");
	s32 ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
    if (ret)
	{
        printf("cellSysmoduleLoadModule(CELL_SYSMODULE_FS) returned: 0x%x\n\n", ret);
        sys_process_exit(1);
    }

	printf("Opening '/Dir/hello.txt'...\n");
	s32 fd;
	ret = cellFsOpen("/Dir/hello.txt", CELL_FS_O_RDONLY, &fd, NULL, 0);
	if (ret != CELL_OK)
	{
		printf("Failed to open '/Dir/hello.txt'\n");
		printf("Error code: 0x%x\n", ret);
	}
	else
	{
		printf("'/Dir/hello.txt' successfully opened.\n");
	}
}