#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/process.h>
#include <cell/sail.h>
#include <cell/sysmodule.h>
#include "SailCheck.h"

void printStruct(void* player, uint32_t size)
{
	for (int i = 0; i < size; i++)
	{
		printf("[%X]: 0x%x\n", i, *((uint8_t*)player + i));
	}
}

void* memAlloc(Player* pSelf, size_t boundary, size_t size)
{
	void* memory = memalign(boundary, size);
	printf("memAlloc(0x%x, %d, 0x%X)\n", memory, boundary, size);
	return memory;
}

void memFree(Player* pSelf, size_t boundary, void* memory)
{
	printf("memFree(0x%x, %d)\n", memory, boundary);
	free(memory);
}

void getParameter(CellSailPlayer* player, int32_t parameter, uint64_t* param0, uint64_t* param1)
{
	int32_t ret = cellSailPlayerGetParameter(player, parameter, param0, param1);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerSetParameter() error: 0x%x\n", ret);
		sys_process_exit(1);
	}
}

void setParameter(CellSailPlayer* player, int32_t parameter, uint64_t param0, uint64_t param1)
{
	int32_t ret = cellSailPlayerSetParameter(player, parameter, param0, param1);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerSetParameter() error: 0x%\n", ret);
		sys_process_exit(1);
	}
}

void test8(CellSailPlayer* player, uint32_t offset, uint8_t expected)
{
	if (*((uint8_t*)player + offset) != expected)
	{
		printf("Incorrect value at offset 0x%X:\n", offset);
		printf("Value: 0x%X\n", *((uint8_t*)player + offset));
		printf("Expected: 0x%X\n", expected);
		sys_process_exit(1);
	}
}

void test32(CellSailPlayer* player, uint32_t offset, uint32_t expected)
{
	if (*(uint32_t*)((uint8_t*)player + offset) != expected)
	{
		printf("Incorrect value at offset 0x%X:\n", offset);
		printf("Value: 0x%X\n", *(uint32_t*)((uint8_t*)player + offset));
		printf("Expected: 0x%X\n", expected);
		sys_process_exit(1);
	}
}

void testParameter(CellSailPlayer* player, int32_t parameter, uint64_t value, uint32_t offset)
{
	setParameter(player, parameter, value, 0);
	test32(player, offset, value);

	uint64_t param0;
	getParameter(player, parameter, &param0, NULL);

	if (param0 != value)
	{
		printf("Parameter (%d) value not same as the one passed set previously:\n", parameter);
		printf("Value: 0x%llX\n", param0);
		printf("Expected: 0x%llX\n", value);
		sys_process_exit(1);
	}
}

void callback(void* pArg, CellSailEvent evnt, uint64_t arg, uint64_t arg1)
{
	Player& player = *(Player*)pArg;
	printf("major: %d\n", evnt.u32x2.major);
	printf("minor: %d\n", evnt.u32x2.minor);

	if (player.call_count == 0)
	{
		// Test the callback parameter
		printf("Hi! I'm alive.\n");
		if (arg1 != 0xB00B)
		{
			printf("Incorrect callback user-defined parameter.\n");
			printf("Value: 0x%X\n", arg1);
			printf("Expected: 0xB00B\n");
			sys_process_exit(1);
		}
	}

	player.call_count++;

	if (evnt.u32x2.major == CELL_SAIL_EVENT_PLAYER_CALL_COMPLETED)
	{
		CellSailFuture* future = (CellSailFuture*)arg1;
		int32_t ret = cellSailFutureSet(future, arg);

		if (ret != CELL_OK)
		{
			printf("cellSailFutureSet() returned: 0x%x\n", ret);
			sys_process_exit(1);
		}
	}
}

void testParameters(Player* player)
{
	int32_t ret = cellSailPlayerInitialize2(&player->player, &player->allocator, &callback, &player, &player->attribute, &player->resource);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerInitialize2() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	// Values for testing
	uint64_t dmux = 0x1010101;
	uint64_t dmuxA = 0x1020304;
	spu_priorities dmuxC;
	dmuxC.priorities[0] = 0x1;
	dmuxC.priorities[1] = 0x2;
	dmuxC.priorities[2] = 0x3;
	dmuxC.priorities[3] = 0x4;
	dmuxC.priorities[4] = 0x5;
	dmuxC.priorities[5] = 0x6;
	dmuxC.priorities[6] = 0x7;
	dmuxC.priorities[7] = 0x8;
	uint64_t param0;

	// Check default parameter values
	test32(&player->player, 0x390, 0x384);  // CELL_SAIL_PARAMETER_CONTROL_PPU_THREAD_PRIORITY
	test32(&player->player, 0x394, 0x3000); // CELL_SAIL_PARAMETER_CONTROL_PPU_THREAD_STACK_SIZE
	test32(&player->player, 0x398, 0x4);    // CELL_SAIL_PARAMETER_SPURS_NUM_OF_SPUS
	test32(&player->player, 0x39C, 0x64);   // CELL_SAIL_PARAMETER_SPURS_SPU_THREAD_PRIORITY
	test32(&player->player, 0x3A0, 0x12C);  // CELL_SAIL_PARAMETER_SPURS_PPU_THREAD_PRIORITY
	test8(&player->player, 0x3A4, true);    // CELL_SAIL_PARAMETER_SPURS_EXIT_IF_NO_WORK
	test32(&player->player, 0x3A8, 0x0);    // CELL_SAIL_PARAMETER_IO_PPU_THREAD_PRIORITY (Set to 0 at first)
	test32(&player->player, 0x3AC, 0x0);    // CELL_SAIL_PARAMETER_IO_PPU_THREAD_STACK_SIZE (Set to 0 at first)
	test32(&player->player, 0x360, 0x2BC);  // CELL_SAIL_PARAMETER_DMUX_PPU_THREAD_PRIORITY
	test32(&player->player, 0x36C, dmux);   // CELL_SAIL_PARAMETER_DMUX_SPURS_TASK_PRIORITIES
	test32(&player->player, 0x370, dmux);   // CELL_SAIL_PARAMETER_DMUX_SPURS_TASK_PRIORITIES
	test32(&player->player, 0x374, 0x1);    // CELL_SAIL_PARAMETER_DMUX_NUM_OF_SPUS

	// Change and test the parameters
	testParameter(&player->player, CELL_SAIL_PARAMETER_CONTROL_PPU_THREAD_PRIORITY, 0x3386D, 0x390);
	testParameter(&player->player, CELL_SAIL_PARAMETER_CONTROL_PPU_THREAD_STACK_SIZE, 0x45678, 0x394);
	testParameter(&player->player, CELL_SAIL_PARAMETER_SPURS_NUM_OF_SPUS, 0x7, 0x398);
	testParameter(&player->player, CELL_SAIL_PARAMETER_SPURS_SPU_THREAD_PRIORITY, 0x97694, 0x39C);
	testParameter(&player->player, CELL_SAIL_PARAMETER_SPURS_PPU_THREAD_PRIORITY, 0x12D2F, 0x3A0);
	testParameter(&player->player, CELL_SAIL_PARAMETER_SPURS_EXIT_IF_NO_WORK, false, 0x3A4);
	testParameter(&player->player, CELL_SAIL_PARAMETER_IO_PPU_THREAD_PRIORITY, 0xEFD6C1F, 0x3A8);
	testParameter(&player->player, CELL_SAIL_PARAMETER_IO_PPU_THREAD_STACK_SIZE, 0x4F3ADB, 0x3AC);
	testParameter(&player->player, CELL_SAIL_PARAMETER_DMUX_PPU_THREAD_PRIORITY, 0x15AC5F, 0x360);
	testParameter(&player->player, CELL_SAIL_PARAMETER_DMUX_NUM_OF_SPUS, 0x8, 0x374);
	setParameter(&player->player, CELL_SAIL_PARAMETER_DMUX_SPURS_TASK_PRIORITIES, dmuxC.priority, 0);
	test32(&player->player, 0x36C, dmuxA);
	test32(&player->player, 0x370, dmuxC.priority);
	getParameter(&player->player, CELL_SAIL_PARAMETER_DMUX_SPURS_TASK_PRIORITIES, &param0, NULL);

	if (param0 != dmuxC.priority)
	{
		printf("Parameter (%d) value not same as the one set previously:\n", CELL_SAIL_PARAMETER_DMUX_SPURS_TASK_PRIORITIES);
		printf("Value: 0x%llX\n", param0);
		printf("Expected: 0x%llX\n", dmuxC.priority);
		sys_process_exit(1);
	}

	ret = cellSailPlayerFinalize(&player->player);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerFinalize() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}
}

int main(void)
{
	// Disable printf buffering
	setbuf(stdout, NULL);

	printf("TEST00007 by tambre.\n");
	printf("Test for checking accuracy of cellSail module emulation.\n\n");

	printf("Loading CELL_SYSMODULE_SAIL...\n");

	int32_t ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SAIL);

	if (ret != CELL_OK)
	{
		printf("cellSysmoduleLoadModule(CELL_SYSMODULE_SAIL) error: 0x%x\n", ret);
		sys_process_exit(1);
	}

	Player player;

	memset(&player, 0, sizeof(Player));
	memset(&player.player, 0, sizeof(CellSailPlayer));
	memset(&player.resource, 0, sizeof(CellSailPlayerResource));
	memset(&player.attribute, 0, sizeof(CellSailPlayerAttribute));
	player.attribute.playerPreset = CELL_SAIL_PLAYER_PRESET_AV_SYNC_AUTO_DETECT;
	player.attribute.maxAudioStreamNum = 1;
	player.attribute.maxVideoStreamNum = 1;
	player.attribute.maxUserStreamNum = 1;
	player.attribute.queueDepth = 0;
	player.resource.pSpurs = NULL;
	player.call_count = 0;

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

	printf("Testing");
	testParameters(&player);
	printf("Testing2");

	ret = cellSailPlayerInitialize2(&player.player, &player.allocator, &callback, &player, &player.attribute, &player.resource);

	if (ret != CELL_OK)
	{
		printf("cellSailPlayerInitialize2() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	// Create CellSailFuture to ease asychronous processing
	ret = cellSailFutureInitialize(&player.future);

	if (ret != CELL_OK)
	{
		printf("cellSailFutureInitialize() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	//ret = cellSailPlayerCreateDescriptor(&player.player, CELL_SAIL_STREAM_MP4, NULL, "x-cell-fs:///app_home/SampleVideo.mp4", &player.descriptor);
	ret = cellSailPlayerBoot(&player.player, (uint64_t)(intptr_t)&player.future);
	
	if (ret != CELL_OK)
	{
		printf("cellSailPlayerBoot() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	// Wait for completion
	int32_t result;
	ret = cellSailFutureGet(&player.future, 10, &result);

	if (ret != CELL_OK)
	{
		printf("cellSailFutureGet() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	if (result != CELL_OK)
	{
		printf("Error from asynchronous function: 0x%X\n", result);
		sys_process_exit(1);
	}

	printStruct(&player.player, sizeof(player.player));

	ret = cellSailFutureFinalize(&player.future);

	if (ret != CELL_OK)
	{
		printf("cellSailFutureFinalize() returned: 0x%x\n", ret);
		sys_process_exit(1);
	}

	printf("The memory structure of CellSailPlayer is valid!\n");

	return 0;
}