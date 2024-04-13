#include "arena.h"

void init_arena(Arena *arena, void* memory, u64 size) 
{
    arena->base = (u8*) memory;
    arena->curr = 0;
    arena->size = size;
}

void* push_size(Arena* arena, u64 size, u32 align = 1)
{
    if (align == 0) {
        align = 1;
    }

    u64 start = (arena->curr + align - 1) & ~((u64) (align - 1));
    u64 end = start + size;
    assert(start + size <= arena->size);
    arena->curr = end;
    return arena->base + start;
}

TempMemoryRegion Arena::start_temp()
{
    return { this, curr };
}

void TempMemoryRegion::forget()
{
    arena->curr = start;
}
