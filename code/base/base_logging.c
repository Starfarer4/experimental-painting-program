/*  =======================================================================
    File: base_logging.c
    Date: March 6th 2024  2:44 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

void LogStr8(string8 Message, ...)
{
    temp_arena Scratch = GetScratch(0, 0);
    
    va_list Args;
    va_start(Args, Message);
    string8 Output = PushStr8FmtV(Scratch.Arena, Message, Args);
    va_end(Args);
    
    OSOutputDebugMessage(Output);
    
    ReleaseScratch(Scratch);
}

void WarningStr8(string8 Message, ...)
{
    temp_arena Scratch = GetScratch(0, 0);
    
    va_list Args;
    va_start(Args, Message);
    string8 Buffer = PushStr8FmtV(Scratch.Arena, Message, Args);
    va_end(Args);

    string8 Output = PushStr8Fmt(Scratch.Arena, Str8Lit("WARNING: %.*s"), Str8VArg(Buffer));
    OSOutputDebugMessage(Output);
    
    ReleaseScratch(Scratch);
}

void ErrorStr8(string8 Message, ...)
{
    temp_arena Scratch = GetScratch(0, 0);
    
    va_list Args;
    va_start(Args, Message);
    string8 Buffer = PushStr8FmtV(Scratch.Arena, Message, Args);
    va_end(Args);

    string8 Output = PushStr8Fmt(Scratch.Arena, Str8Lit("ERROR: %.*s"), Str8VArg(Buffer));
    OSOutputDebugMessage(Output);
    
    ReleaseScratch(Scratch);
}

void FatalErrorStr8(string8 Message, ...)
{
    temp_arena Scratch = GetScratch(0, 0);
    
    va_list Args;
    va_start(Args, Message);
    string8 Buffer = PushStr8FmtV(Scratch.Arena, Message, Args);
    va_end(Args);

    string8 Output = PushStr8Fmt(Scratch.Arena, Str8Lit("FATAL ERROR: %.*s"), Str8VArg(Buffer));
    OSOutputDebugMessage(Output);
    
    ReleaseScratch(Scratch);
}
