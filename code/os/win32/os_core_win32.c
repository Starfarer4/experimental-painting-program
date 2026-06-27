/*  =======================================================================
    File: os_core_win32.c
    Date: March 5th 2024 11:07 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

typedef struct win32_thread win32_thread;
struct win32_thread
{
    win32_thread *Next;
    win32_thread *Prev;
    
    void *Params;
    os_thread_function *Function;
    DWORD ThreadID;
    HANDLE Handle;
};

typedef struct win32_mutex win32_mutex;
struct win32_mutex
{
    win32_mutex *Next;
    win32_mutex *Prev;

    CRITICAL_SECTION Mutex;
};

typedef struct win32_os_state win32_os_state;
struct win32_os_state
{
    U64 MicrosecondResolution;

    win32_thread *FirstFreeThread;
    win32_thread *LastFreeThread;

    win32_mutex *FirstFreeMutex;
    win32_mutex *LastFreeMutex;

    CRITICAL_SECTION Mutex;
    
    arena *Arena;
};

global_variable win32_os_state *Win32OSState = 0;

win32_thread *Win32ThreadAlloc()
{
    EnterCriticalSection(&Win32OSState->Mutex);    
    win32_thread *Result = 0;

    if(Win32OSState->FirstFreeThread)
    {
        Result = Win32OSState->FirstFreeThread;
        DLLRemove(Win32OSState->FirstFreeThread, Win32OSState->LastFreeThread, Result);
        MemoryZero(Result, sizeof(win32_thread));
    }
    else
        Result = PushStruct(Win32OSState->Arena, win32_thread);
    LeaveCriticalSection(&Win32OSState->Mutex);
    
    return Result;
}

void Win32ThreadFree(win32_thread *Thread)
{
    DLLPushFront(Win32OSState->FirstFreeThread, Win32OSState->LastFreeThread, Thread);
}

DWORD Win32ThreadEntryPoint(void *Params)
{
    win32_thread *Thread = (win32_thread *)Params;
    thread_context Context;
    ThreadContextInitAndEquip(&Context);
    Thread->Function(Thread->Params);
    ThreadContextRelease();
    Win32ThreadFree(Thread);
    
    return 0;
}

void OSStartUp()
{
    arena *Arena = ArenaAlloc(Megabytes(4));
    Win32OSState = PushStruct(Arena, win32_os_state);
    Win32OSState->Arena = Arena;

    LARGE_INTEGER LargeIntResolution;
    if(QueryPerformanceCounter(&LargeIntResolution))
        Win32OSState->MicrosecondResolution = LargeIntResolution.QuadPart;
    else
        Win32OSState->MicrosecondResolution = 1;

    InitializeCriticalSection(&Win32OSState->Mutex);
}

void OSOutputDebugMessage(string8 String)
{
    temp_arena Scratch = GetScratch(0, 0);
    string16 WString = Str16From8(Scratch.Arena, String);
    OutputDebugStringW(WString.String);
    ReleaseScratch(Scratch);
}

void *OSReserve(U64 Size)
{
    return VirtualAlloc(0, Size, MEM_RESERVE, PAGE_READWRITE);
}

B8 OSCommit(void *Pointer, U64 Size)
{
    return (VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE) != 0);
}

void OSDecommit(void *Pointer, U64 Size)
{
    VirtualFree(Pointer, Size, MEM_DECOMMIT);
}

void OSRelease(void *Pointer, U64 Size)
{
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

os_handle OSFileOpen(os_access_flags Flags, string8 Path)
{
    os_handle Result = {0};

    temp_arena Scratch = GetScratch(0, 0);
    string16 WPath = Str16From8(Scratch.Arena, Path);

    DWORD AccessFlags = 0;
    DWORD ShareMode = 0;
    DWORD CreationDisposition = OPEN_EXISTING;

    if(Flags & OS_ACCESS_FLAG_READ) AccessFlags |= GENERIC_READ;
    if(Flags & OS_ACCESS_FLAG_WRITE) AccessFlags |= GENERIC_WRITE;

    HANDLE File = CreateFileW((WCHAR *)WPath.String, AccessFlags, ShareMode, 0, CreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);

    if(File != INVALID_HANDLE_VALUE)
        Result = (U64)File;

    ReleaseScratch(Scratch);

    return Result;
}

void OSFileClose(os_handle File)
{
    if(File)
    {
        HANDLE Handle = (HANDLE)File;
        CloseHandle(Handle);
    }
}

U64 OSFileRead(os_handle File, U64 Start, U64 End, void *OutData)
{
    U64 TotalReadSize = 0;
    if(File)
    {
        HANDLE Handle = (HANDLE)File;
        
        U64 Size = 0;
        GetFileSizeEx(Handle, (LARGE_INTEGER *)&Size);
        U64 StartClamped = ClampTop(Start, Size);
        U64 EndClamped = ClampBot(End, Size);

        U64 ToRead = EndClamped - StartClamped;
        for(U64 Offset = Start; TotalReadSize < ToRead;)
        {
            U64 Amount64 = ToRead - TotalReadSize;
            U32 Amount32 = U32FromU64Satruate(Amount64);
            DWORD ReadSize = 0;
            OVERLAPPED Overlapped = {0};
            Overlapped.Offset = (Offset & 0x00000000ffffffffull);
            Overlapped.OffsetHigh = (Offset & 0xffffffff00000000ull) >> 32;
            ReadFile(Handle, (U8 *)OutData + TotalReadSize, Amount32, &ReadSize, &Overlapped);
            Offset += ReadSize;
            TotalReadSize += ReadSize;
            if(ReadSize != Amount32)
                break;
        }
    }
    return TotalReadSize;
}

void OSFileWrite(os_handle File, U64 Start, U64 End, void *Data)
{
    if(File)
    {
        HANDLE Handle = (HANDLE)File;
        U64 SrcOffset = 0;
        U64 DstOffset = Start;
        U64 BytesToWriteTotal = End - Start;
        while(SrcOffset < BytesToWriteTotal)
        {
            void *BytesSrc = (void *)((U8 *)Data + SrcOffset);
            U64 BytesToWrite64 = (BytesToWriteTotal - SrcOffset);
            U32 BytesToWrite32 = U32FromU64Satruate(BytesToWrite64);
            U32 BytesWritten = 0;
            OVERLAPPED Overlapped = {0};
            Overlapped.Offset = (DstOffset & 0x00000000ffffffffull);
            Overlapped.OffsetHigh = (DstOffset & 0xffffffff00000000ull) >> 32;
            BOOL Success = WriteFile(Handle, BytesSrc, BytesToWrite32, (DWORD *)&BytesWritten, &Overlapped);
            if(Success == 0)
                break;
            SrcOffset += BytesWritten;
            DstOffset += BytesWritten;
        }
    }
}

os_file_properties OSPropertiesFromFile(os_handle File)
{
    os_file_properties Properties = {0};
    if(File)
    {
        HANDLE Handle = (HANDLE)File;
        BY_HANDLE_FILE_INFORMATION Info;
        BOOL InfoGood = GetFileInformationByHandle(Handle, &Info);
        if(InfoGood)
        {
            U32 SizeLow = Info.nFileSizeLow;
            U32 SizeHigh = Info.nFileSizeHigh;
            Properties.Size = (U64)SizeLow | (((U64)SizeHigh) << 32);
        }
    }
    return Properties;
}

U64 OSNowMicroseconds()
{
    U64 Result = 0;
    LARGE_INTEGER LargeIntCounter;
    if(QueryPerformanceCounter(&LargeIntCounter))
        Result = (LargeIntCounter.QuadPart * Million(1)) / Win32OSState->MicrosecondResolution;
    return Result;
}

os_handle OSThreadStart(void *Params, os_thread_function *Function)
{
    win32_thread *Thread = Win32ThreadAlloc();
    Thread->Params = Params;
    Thread->Function = Function;
    Thread->Handle = CreateThread(0, 0, Win32ThreadEntryPoint, Thread, 0, &Thread->ThreadID);
    os_handle Result = (os_handle)Thread;
    return Result;
}

os_handle OSMutexAlloc()
{
    EnterCriticalSection(&Win32OSState->Mutex);
    win32_mutex *Result = 0;

    if(Win32OSState->FirstFreeMutex)
    {
        Result = Win32OSState->FirstFreeMutex;
        DLLRemove(Win32OSState->FirstFreeMutex, Win32OSState->LastFreeMutex, Result);
        MemoryZero(Result, sizeof(win32_mutex));
    }
    else
        Result = PushStruct(Win32OSState->Arena, win32_mutex);
    LeaveCriticalSection(&Win32OSState->Mutex);

    InitializeCriticalSection(&Result->Mutex);
    
    return (os_handle)Result;
}

void OSMutexRelease(os_handle MutexHandle)
{
    win32_mutex *Mutex = (win32_mutex *)MutexHandle;

    DeleteCriticalSection(&Mutex->Mutex);
    
    EnterCriticalSection(&Win32OSState->Mutex);
    DLLPushFront(Win32OSState->FirstFreeMutex, Win32OSState->LastFreeMutex, Mutex);
    LeaveCriticalSection(&Win32OSState->Mutex);
}

void OSMutexTake(os_handle MutexHandle)
{
    win32_mutex *Mutex = (win32_mutex *)MutexHandle;
    EnterCriticalSection(&Mutex->Mutex);
}

void OSMutexDrop(os_handle MutexHandle)
{
    win32_mutex *Mutex = (win32_mutex *)MutexHandle;
    LeaveCriticalSection(&Mutex->Mutex);
}
