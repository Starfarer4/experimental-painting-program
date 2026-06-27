#if !defined(OS_CORE_H)
/*  =======================================================================
    File: os_core.h
    Date: March 5th 2024 11:08 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define OS_CORE_H

typedef U64 os_handle;

typedef enum os_access_flags
{
    OS_ACCESS_FLAG_READ = (1<<0),
    OS_ACCESS_FLAG_WRITE = (1<<1),
}os_access_flags;

typedef struct os_file_properties os_file_properties;
struct os_file_properties
{
    U64 Size;
};

void OSOutputDebugMessage(string8 String);

void *OSReserve(U64 Size);
B8 OSCommit(void *Pointer, U64 Size);
void OSDecommit(void *Pointer, U64 Size);
void OSRelease(void *Pointer, U64 Size);

os_handle OSFileOpen(os_access_flags Flags, string8 Path);
void OSFileClose(os_handle File);
U64 OSFileRead(os_handle File, U64 Start, U64 End, void *OutData);
void OSFileWrite(os_handle File, U64 Start, U64 End, void *Data);
os_file_properties OSPropertiesFromFile(os_handle File);

typedef void os_thread_function(void *Params);

void OSStartUp();
U64 OSNowMicroseconds();

os_handle OSThreadStart(void *Params, os_thread_function *Function);

os_handle OSMutexAlloc();
void OSMutexRelease(os_handle MutexHandle);
void OSMutexTake(os_handle MutexHandle);
void OSMutexDrop(os_handle MutexHandle);

#endif
