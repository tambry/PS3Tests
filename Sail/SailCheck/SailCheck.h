#pragma once

class Player
{
public:
	CellSailPlayer player;
	CellSailMemAllocator allocator;
	CellSailPlayerAttribute attribute;
	CellSailPlayerResource resource;
	CellSailDescriptor* descriptor;
	CellSailDescriptor* descriptor2;
	CellSailFuture future;

	uint32_t call_count;
};

union spu_priorities
{
	uint8_t priorities[8];
	uint64_t priority;
};