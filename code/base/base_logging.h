#if !defined(BASE_LOGGING_H)
/*  =======================================================================
    File: base_logging.h
    Date: March 6th 2024  2:47 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define BASE_LOGGING_H

void LogStr8(string8 Message, ...);
void WarningStr8(string8 Message, ...);
void ErrorStr8(string8 Message, ...);
void FatalErrorStr8(string8 Message, ...);

#define Log(Message, ...) LogStr8(Str8Lit(Message), __VA_ARGS__)
#define Warning(Message, ...) WarningStr8(Str8Lit(Message), __VA_ARGS__)
#define Error(Message, ...) ErrorStr8(Str8Lit(Message), __VA_ARGS__)
#define FatalError(Message, ...) FatalErrorStr8(Str8Lit(Message), __VA_ARGS__)

#endif
