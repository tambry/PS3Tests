#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/process.h>
#include <cell/sysmodule.h>
#include <cell/sail.h>
#include "Player.h"

void printPlayer(Player* pSelf)
{
	CellSailPlayer* player = &pSelf->player;

	for (int i = 0; i < sizeof(CellSailPlayer); i++)
	{
		printf("[%X]: %x", i, *player++);
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
	printf("Test for checking LLE-level accuracy of the cellSail module.\n\n");

	printf("Loading CELL_SYSMODULE_SAIL...\n");
	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SAIL);

    if (ret != CELL_OK)
	{
        printf("cellSysmoduleLoadModule(CELL_SYSMODULE_SAIL) returned: 0x%x\n", ret);
        sys_process_exit(1);
    }

	Player player;

	CellSailMemAllocatorFuncs memFuncs = {
		(CellSailMemAllocatorFuncAlloc)memAlloc,
		(CellSailMemAllocatorFuncFree) memFree,
	};

	ret = cellSailMemAllocatorInitialize(&player.allocator, &memFuncs, &player);

	if (ret != CELL_OK)
	{
		printf("cellSailMemAllocatorInitialize() returned: 0x%x\n", ret);
        sys_process_exit(1);
	}

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

	return 0;
}