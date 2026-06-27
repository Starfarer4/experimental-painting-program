#if !defined(CODEGEN_H)
/*  =======================================================================
    File: codegen.h
    Date: March 27th 2024  3:54 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define CODEGEN_H

typedef struct file_pair file_pair;
struct file_pair
{
    MD_String8 SrcFilename;
    FILE *H;
    FILE *C;
};

typedef struct codegen_state codegen_state;
struct codegen_state
{
    int FilePairCount;
    file_pair FilePairs[4096];
    MD_Arena *Arena;
};

static codegen_state *CodegenState = 0;

file_pair FilePairFromNode(MD_Node *Node);
MD_String8 EscapedFromString(MD_Arena *Arena, MD_String8 String);

#endif
