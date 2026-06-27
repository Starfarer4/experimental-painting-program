/*  =======================================================================
    File: codegen.c
    Date: March 27th 2024  3:11 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

#include <md.h>
#include <md.c>

#include "codegen.h"

#include "codegen_table.h"
#include "codegen_table.c"

file_pair FilePairFromNode(MD_Node *Node)
{
    file_pair Result = {0};
    MD_CodeLoc Loc = MD_CodeLocFromNode(Node);
    MD_String8 Filename = Loc.filename;
    MD_b32 Found = 0;

    for(int Pair = 0; Pair < CodegenState->FilePairCount; ++Pair)
    {
        if(MD_S8Match(Filename, CodegenState->FilePairs[Pair].SrcFilename, 0))
        {
            Result = CodegenState->FilePairs[Pair];
            Found = 1;
            break;
        }
    }
    
    if(Found == 0)
    {
        MD_String8 Folder = MD_PathChopLastSlash(Filename);
        MD_String8 LayerName = MD_PathChopLastPeriod(MD_PathSkipLastSlash(Loc.filename));
        MD_String8 GenFolder = MD_S8Fmt(CodegenState->Arena, "%.*s/generated", MD_S8VArg(Folder));
        MD_String8 HFilename = MD_S8Fmt(CodegenState->Arena, "%.*s/%.*s.meta.h", MD_S8VArg(GenFolder), MD_S8VArg(LayerName));
        MD_String8 CFilename = MD_S8Fmt(CodegenState->Arena, "%.*s/%.*s.meta.c", MD_S8VArg(GenFolder), MD_S8VArg(LayerName));
        Result.SrcFilename = Filename;
        Result.H = fopen((char *)HFilename.str, "w");
        Result.C = fopen((char *)CFilename.str, "w");
        CodegenState->FilePairs[CodegenState->FilePairCount] = Result;
        ++CodegenState->FilePairCount;
    }

    return Result;
}

MD_String8 EscapedFromString(MD_Arena *Arena, MD_String8 String)
{
    MD_ArenaTemp Scratch = MD_GetScratch(&Arena, 1);
    MD_String8List Strings = {0};
    MD_b32 Escaped = 0;
    MD_u64 Start = 0;
    
    for(MD_u64 Index = 0; Index <= String.size; ++Index)
    {
        if(Index < String.size && Escaped)
        {
            Escaped = 0;
            Start = Index + 1;
            MD_u8 ReplaceChar = 0;
            switch(String.str[Index])
            {
                default: break;
                case 'a': ReplaceChar = 0x07; break;
                case 'b': ReplaceChar = 0x08; break;
                case 'e': ReplaceChar = 0x1b; break;
                case 'f': ReplaceChar = 0x0c; break;
                case 'n': ReplaceChar = 0x0a; break;
                case 'r': ReplaceChar = 0x0d; break;
                case 't': ReplaceChar = 0x09; break;
                case 'v': ReplaceChar = 0x0b; break;
                case '\\': ReplaceChar = 0x5c; break;
                case '\'': ReplaceChar = 0x27; break;
                case '\"': ReplaceChar = 0x22; break;
                case '\?': ReplaceChar = 0x3f; break;
            }
            
            if(ReplaceChar)
            {
                MD_String8 String = MD_S8Copy(Scratch.arena, MD_S8(&ReplaceChar, 1));
                MD_S8ListPush(Scratch.arena, &Strings, String);
            }
        }
        else if(Index == String.size || String.str[Index] == '\\')
        {
            Escaped = (String.str[Index] == '\\');
            MD_String8 Part = MD_S8Substring(String, Start, Index);
            MD_S8ListPush(Scratch.arena, &Strings, Part);
            Start = Index;
        }
    }
    
    MD_String8 Result = MD_S8ListJoin(Arena, Strings, 0);
    MD_ReleaseScratch(Scratch);
    return Result;
}

MD_String8 ArrayLiteralContentsFromData(MD_Arena *Arena, MD_String8 Data, MD_u64 *Count)
{
    MD_ArenaTemp Scratch = MD_GetScratch(0, 0);
    MD_String8List Strings = {0};
    *Count = 0;
    for(MD_u64 Offset = 0; Offset < Data.size;)
    {
        MD_u64 ChunkSize = MD_Min(Data.size - Offset, 64);
        MD_u8 *ChunkBytes = Data.str + Offset;
        MD_String8 ChunkTextString = {0};
        ChunkTextString.size = ChunkSize * 5;
        ChunkTextString.str = MD_PushArrayZero(Arena, MD_u8, ChunkTextString.size);
        for(MD_u64 ByteIndex = 0; ByteIndex < ChunkSize; ++ByteIndex)
        {
            MD_String8 ByteString = MD_S8Fmt(Scratch.arena, "0x%02x,", ChunkBytes[ByteIndex]);
            MD_MemoryCopy(ChunkTextString.str + ByteIndex * 5, ByteString.str, ByteString.size);
            if(Count) *Count += 1;
        }
        Offset += ChunkSize;
        MD_S8ListPush(Arena, &Strings, ChunkTextString);
        MD_S8ListPush(Arena, &Strings, MD_S8Lit("\n"));
    }
    MD_String8 Result = MD_S8ListJoin(Arena, Strings, 0);
    MD_ReleaseScratch(Scratch);
    return Result;
}

int main(int argc, char **argv)
{
    MD_Arena *Arena = MD_ArenaAlloc();
    CodegenState = MD_PushArray(Arena, codegen_state, 1);
    CodegenState->Arena = Arena;

    MD_String8List Options = MD_StringListFromArgCV(Arena, argc, argv);
    MD_CmdLine CmdLine = MD_MakeCmdLineFromOptions(Arena, Options);

    MD_Node *FileList = MD_MakeList(Arena);
    for(MD_String8Node *Node = CmdLine.inputs.first; Node; Node = Node->next)
    {
        MD_String8 CodeDir = Node->string;
        printf("Searching %.*s for metacode...\n", MD_S8VArg(CodeDir));
        MD_FileIter Iterator = {0};
        MD_FileIterBegin(&Iterator, CodeDir);
        
        for(MD_FileInfo Info = {0};;)
        {
            Info = MD_FileIterNext(Arena, &Iterator);
            if(Info.filename.size == 0)
                break;
            
            if(!(Info.flags & MD_FileFlag_Directory) &&
               MD_S8Match(MD_PathSkipLastPeriod(Info.filename), MD_S8Lit("mdesk"), MD_StringMatchFlag_CaseInsensitive))
            {
                printf("parsing %.*s...\n", MD_S8VArg(Info.filename));
                MD_String8 Path = MD_S8Fmt(Arena, "%.*s/%.*s", MD_S8VArg(CodeDir), MD_S8VArg(Info.filename));
                MD_ParseResult Parse = MD_ParseWholeFile(Arena, Path);
                MD_PushNewReference(Arena, FileList, Parse.node);
            }
        }
    }

    TableGenerate(FileList);

    // Generate embeds
    for(MD_EachNode(FileRef, FileList->first_child))
    {
        MD_Node *File = MD_ResolveNodeFromReference(FileRef);
        MD_String8 LayerName = File->string;
        MD_MapKey LayerKey = MD_MapKeyStr(LayerName);

        for(MD_EachNode(Node, File->first_child))
        {
            if(MD_NodeHasTag(Node, MD_S8Lit("embed_file"), 0))
            {
                MD_String8 HDeclSpecifier = MD_S8Lit("extern");
                MD_String8 Data = MD_LoadEntireFile(Arena, Node->first_child->string);
                MD_u64 Count = 0;
                MD_String8 GenString = ArrayLiteralContentsFromData(Arena, Data, &Count);
            
                file_pair FilePair = FilePairFromNode(Node);
                fprintf(FilePair.H, "%.*s U8 %.*s[%llu];\n\n", MD_S8VArg(HDeclSpecifier), MD_S8VArg(Node->string), Count);
                fprintf(FilePair.C, "U8 %.*s[%llu] = \n{\n", MD_S8VArg(Node->string), Count);
                fprintf(FilePair.C, "%.*s", MD_S8VArg(GenString));
                fprintf(FilePair.C, "};\n");
                fprintf(FilePair.C, "\n");
            }
        }
    }

    return 0;
}
