#include "arena.h"

void InitArena(Arena *arena, void* memory, u64 size)
{
    arena->base = (u8*) memory;
    arena->curr = 0;
    arena->size = size;
}

void* PushSize(Arena* arena, u64 size, u32 align = 1, bool zero = false)
{
    if (align == 0) {
        align = 1;
    }

    u64 start = (arena->curr + align - 1) & ~((u64) (align - 1));
    u64 end = start + size;
    assert(start + size <= arena->size);
    arena->curr = end;

    void *result = arena->base + start;
    if (zero) {
        memset(result, 0, size);
    }

    return result;
}

TempMemoryRegion Arena::StartTemp()
{
    return { this, curr };
}

void TempMemoryRegion::Forget()
{
    arena->curr = start;
}
