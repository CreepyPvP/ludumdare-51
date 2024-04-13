#ifndef ARENA_H
#define ARENA_H

#define PushStruct(arena, type) (type*) PushSize(arena, sizeof(type), alignof(type))
#define PushArray(arena, type, count) (type*) PushSize(arena, sizeof(type) * count, alignof(type))
#define PushZeroArray(arena, type, count) (type*) PushSize(arena, sizeof(type) * count, alignof(type), true)
#define PushBytes(arena, count) (u8*) PushSize(arena, count, alignof(u8))


struct Arena;

struct TempMemoryRegion
{
    Arena *arena;
    u64 start;

    void Forget();
};

struct Arena
{
    u8* base;
    u64 curr;
    u64 size;

    TempMemoryRegion StartTemp();
};

#endif
