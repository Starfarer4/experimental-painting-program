#if !defined(BASE_STRING_H)
/*  =======================================================================
    File: base_string.h
    Date: March 5th 2024 10:58 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define BASE_STRING_H

typedef enum string_match_flags
{
    STRING_MATCH_FLAG_CASE_INSENSITIVE = (1<<0),
    STRING_MATCH_FLAG_RIGHT_SIDE_SLOPPY = (1<<1),
    STRING_MATCH_FLAG_SLASH_INSENSITIVE = (1<<2)
}string_match_flags;

typedef struct string8 string8;
struct string8
{
    U8 *String;
    U64 Size;
};

typedef struct string16 string16;
struct string16
{
    U16 *String;
    U64 Size;
};

typedef struct string32 string32;
struct string32
{
    U32 *String;
    U64 Size;
};

typedef struct unicode_decode unicode_decode;
struct unicode_decode
{
    U32 Increment;
    U32 Codepoint;
};

const global_variable U8 Utf8Class[32] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,};

unicode_decode Utf8Decode(U8 *String, U64 Max);
U32 Utf16Encode(U16 *String, U32 Codepoint);

B8 CharIsLower(U8 Char);
U8 CharToUpper(U8 Char);
B8 CharIsSlash(U8 Char);
U8 CharToCorrectSlash(U8 Char);

// string8
#define Str8Lit(String) Str8((U8 *)(String), sizeof(String) - 1)
#define Str8LitComp(String) {(U8 *)(String), sizeof(String) - 1}
#define Str8VArg(Str) (I32)((Str).Size), ((Str).String)

string8 Str8(U8 *String, U64 Size);
string8 PushStr8FmtV(arena *Arena, string8 Format, va_list Args);
string8 PushStr8Fmt(arena *Arena, string8 Format, ...);
U64 Str8FindNeedle(string8 String, U64 StartPos, string8 Needle, string_match_flags Flags);
string8 Str8Range(U8 *First, U8 *OnePastLast);
B8 Str8Match(string8 First, string8 Second, string_match_flags Flags);
string8 Str8From16(arena *Arena, string16 In);

// string16
#define Str16Lit(String) Str16(String), (sizeof(String) / sizeof(U16)) - 1)
#define Str16LitComp(String) {(U16 *)(String), (sizeof(String) / sizeof(U16)) - 1}
#define Str16VArg(Str) (I32)((Str).Size), ((Str).String)

string16 Str16(U16 *String, U64 Size);

// NOTE: The resulting string is null-terminated.
string16 Str16From8(arena *Arena, string8 In);

// string32
string32 Str32(U32 *String, U64 Size);

string32 Str32From8(arena *Arena, string8 In);

#endif
