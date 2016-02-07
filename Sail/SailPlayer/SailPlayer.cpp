#include <stdlib.h>
#include <string.h>
#include <sys/process.h>
#include <cell/sysmodule.h>
#include <cell/sail.h>
#include "SailPlayer.h"

void printPlayer(Player* pSelf)
{
	CellSailPlayer* player = &pSelf->player;

	for (int i = 0; i < sizeof(CellSailPlayer); i++)
	{
		printf("[%X]: 0x%x\n", i, *((unsigned char*)player + i));
	}
}

void* memAlloc(Player* pSelf, size_t boundary, size_t size)
{
	void* memory = memalign(boundary, size);
	printf("memAlloc(0x%x, %d, %d)\n", memory, boundary, size);
	return memory;
}

void memFree(Player* pSelf, size_t boundary, void* memory)
{
	printf("memFree(0x%x, %d)\n", memory, boundary);
	free(memory);
}

void callback(void* pSelf, CellSailEvent event, uint64_t arg, uint64_t arg1)
{
	Player* player = (Player*)pSelf;
	printf("major: %d\n", event.u32x2.major);
	printf("minor: %d\n", event.u32x2.minor);
	printPlayer(player);
}

int main(void)
{
	printf("TEST00007 by tambre.\n");
	printf("Test for checking memory structure of the CellSailPlayer structure.\n\n");

	printf("Loading CELL_SYSMODULE_SAIL...\n");
	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SAIL);

	if (ret != CELL_OK)
	{
		printf("cellSysmoduleLoadModule(CELL_SYSMODULE_SAIL) error: 0x%x\n", ret);
		sys_process_exit(1);
	}

	Player player;

	CellSailMemAllocatorFuncs memFuncs =
	{
		(CellSailMemAllocatorFuncAlloc)memAlloc,
		(CellSailMemAllocatorFuncFree) memFree,
	};

	ret = cellSailMemAllocatorInitialize(&player.allocator, &memFuncs, &player);

	if (ret != CELL_OK)
	{
		printf("cellSailMemAllocatorInitialize() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	memset(&player, 0, sizeof(CellSailPlayer));
	memset(&player.attribute, 0, sizeof(CellSailPlayerAttribute));
	player.attribute.playerPreset = CELL_SAIL_PLAYER_PRESET_AV_SYNC_AUTO_DETECT;
	player.attribute.maxAudioStreamNum = 2;
	player.attribute.maxVideoStreamNum = 2;
	player.attribute.maxUserStreamNum = 2;
	player.attribute.queueDepth = 0;

	memset(&player.resource, 0, sizeof(CellSailPlayerResource));

	ret = cellSailPlayerInitialize2(&player.player, &player.allocator, callback, &player, &player.attribute, &player.resource);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerInitialize2() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	printPlayer(&player);

	ret = cellSailPlayerSetParameter(&player.player, CELL_SAIL_PARAMETER_SPURS_NUM_OF_SPUS, 400, 0);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerSetParameter() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	//printPlayer(&player);

	return 0;
}