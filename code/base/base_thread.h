#if !defined(BASE_THREAD_H)
/*  =======================================================================
    File: base_thread.h
    Date: June 29th 2024  3:34 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define BASE_THREAD_H

typedef struct thread_context thread_context;
struct thread_context
{
    U8 Name[256];
    U64 NameSize;

    rand_state RandomState;
    arena *Arenas[2];
};

void ThreadContextInitAndEquip(thread_context *Context);
void ThreadContextRelease();
thread_context *ThreadContextGetEquipped();
arena *ThreadContextGetScratch(arena **Conflicts, U64 ConflictCount);
void ThreadContextSetThreadName(string8 Name);
string8 ThreadContextGetThreadName();
U32 ThreadContextRandNextU32();
F32 ThreadContextRandNextF32();

#define GetScratch(Conflicts, ConflictCount) TempArenaBegin(ThreadContextGetScratch((Conflicts), (ConflictCount)))
#define ReleaseScratch(Scratch) TempArenaEnd(Scratch)

#define RandNextU32() ThreadContextRandNextU32()
#define RandNextF32() ThreadContextRandNextF32()

#endif
