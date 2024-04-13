#ifndef ARENA_H
#define ARENA_H

#define push_struct(arena, type) (type*) push_size(arena, sizeof(type), alignof(type))
#define push_arr(arena, type, count) (type*) push_size(arena, sizeof(type) * count, alignof(type))
#define push_bytes(arena, count) (u8*) push_size(arena, count, alignof(u8))


struct Arena;

struct TempMemoryRegion
{
    Arena *arena;
    u64 start;

    void forget();
};

struct Arena
{
    u8* base;
    u64 curr;
    u64 size;

    TempMemoryRegion start_temp();
};

#endif
