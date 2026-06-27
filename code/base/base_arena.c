/*  =======================================================================
    File: base_arena.c
    Date: March 5th 2024 11:05 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

arena *ArenaAlloc(U64 Size)
{
    U64 SizeRoundupGranularity = Megabytes(64);
    Size += SizeRoundupGranularity - 1;
    Size -= Size % SizeRoundupGranularity;
    void *Memory = OSReserve(Size);
    Assert(Memory);
    U64 InitialCommitSize = ARENA_COMMIT_GRANULARITY;
    Assert(InitialCommitSize >= sizeof(arena));
    OSCommit(Memory, InitialCommitSize);
    
    arena *Arena = (arena *)Memory;
    Arena->Pos = sizeof(arena);
    Arena->CommitPos = InitialCommitSize;
    Arena->Size = Size;
    Arena->Align = 8;
    
    return Arena;
}

void ArenaRelease(arena *Arena)
{
    OSRelease((void *)Arena, Arena->Size);
}

void *ArenaPushNoZero(arena *Arena, U64 Size)
{
    void *Result = 0;
    
    if(Arena->Pos + Size <= Arena->Size)
    {
        U8 *Base = (U8 *)Arena;
        U64 PostAlignPos = (Arena->Pos + (Arena->Align - 1));
        PostAlignPos -= PostAlignPos % Arena->Align;
        U64 Align = PostAlignPos - Arena->Pos;
        Result = Base + Arena->Pos + Align;
        Arena->Pos += Size + Align;
        
        if(Arena->CommitPos < Arena->Pos)
        {
            U64 CommitSize = Arena->Pos - Arena->CommitPos;
            CommitSize += ARENA_COMMIT_GRANULARITY - 1;
            CommitSize -= CommitSize % ARENA_COMMIT_GRANULARITY;
            OSCommit(Base + Arena->CommitPos, CommitSize);
            Arena->CommitPos += CommitSize;
        }
    }
    else
    {
        Error("Arena size exceeds its memory reserve!\n");
    }
    
    return Result;
}

void *ArenaPushAligner(arena *Arena, U64 Alignment)
{
    U64 Pos = Arena->Pos;
    U64 PosRoundedUp = Pos + Alignment - 1;
    PosRoundedUp -= PosRoundedUp % Alignment;
    U64 AllocSize = PosRoundedUp - Pos;
    void *Result = 0;
    if(AllocSize != 0)
    {
        Result = ArenaPushNoZero(Arena, AllocSize);
    }
    return Result;
}

void *ArenaPush(arena *Arena, U64 Size)
{
    void *Result = ArenaPushNoZero(Arena, Size);
    MemoryZero(Result, Size);
    return Result;
}

void ArenaPopTo(arena *Arena, U64 Pos)
{
    U64 MinPos = sizeof(arena);
    U64 NewPos = Max(MinPos, Pos);
    Arena->Pos = NewPos;
    
    U64 PosAlignedToCommitChunks = Arena->Pos + ARENA_COMMIT_GRANULARITY - 1;
    PosAlignedToCommitChunks -= PosAlignedToCommitChunks % ARENA_COMMIT_GRANULARITY;
    if(PosAlignedToCommitChunks + ARENA_DECOMMIT_THRESHOLD <= Arena->CommitPos)
    {
        U8 *Base = (U8 *)Arena;
        U64 SizeToDecommit = Arena->CommitPos - PosAlignedToCommitChunks;
        OSDecommit(Base + PosAlignedToCommitChunks, SizeToDecommit);
        Arena->CommitPos -= SizeToDecommit;
    }
}

void ArenaSetAutoAlign(arena *Arena, U64 Align)
{
    Arena->Align = Align;
}

void ArenaPop(arena *Arena, U64 Size)
{
    U64 MinPos = sizeof(arena);
    U64 SizeToPop = Min(Size, Arena->Pos);
    U64 NewPos = Arena->Pos - SizeToPop;
    NewPos = Max(NewPos, MinPos);
    ArenaPopTo(Arena, NewPos);
}

void ArenaClear(arena *Arena)
{
    ArenaPopTo(Arena, sizeof(Arena));
}

U64 ArenaPos(arena *Arena)
{
    return Arena->Pos;
}

temp_arena TempArenaBegin(arena *Arena)
{
    temp_arena Temp = {0};
    Temp.Arena = Arena;
    Temp.Pos = Arena->Pos;
    return Temp;
}

void TempArenaEnd(temp_arena Temp)
{
    ArenaPopTo(Temp.Arena, Temp.Pos);
}
