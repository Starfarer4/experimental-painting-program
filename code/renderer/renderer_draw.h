#if !defined(RENDERER_DRAW_H)
/*  =======================================================================
    File: renderer_draw.h
    Date: March 8th 2024 11:35 AM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define RENDERER_DRAW_H

#include "generated/renderer_draw.meta.h"

RndDefineBucketNodeNils

typedef struct rnd_fancy_run rnd_fancy_run;
struct rnd_fancy_run
{
    font_run Run;
    vec4_f32 Color;
};

typedef struct rnd_fancy_run_node rnd_fancy_run_node;
struct rnd_fancy_run_node
{
    rnd_fancy_run_node *Next;
    rnd_fancy_run_node *Prev;
    rnd_fancy_run Run;
};

typedef struct rnd_fancy_run_list rnd_fancy_run_list;
struct rnd_fancy_run_list
{
    rnd_fancy_run_node *First;
    rnd_fancy_run_node *Last;
    U64 NodeCount;
    vec2_f32 Dimensions;
};

typedef struct rnd_fancy_string rnd_fancy_string;
struct rnd_fancy_string
{
    font_handle Font;
    F32 Size;
    string8 String;
    vec4_f32 TextColor;
};

typedef struct rnd_fancy_string_node rnd_fancy_string_node;
struct rnd_fancy_string_node
{
    rnd_fancy_string_node *Next;
    rnd_fancy_string_node *Prev;
    rnd_fancy_string String;
};

typedef struct rnd_fancy_string_list rnd_fancy_string_list;
struct rnd_fancy_string_list
{
    rnd_fancy_string_node *First;
    rnd_fancy_string_node *Last;
};

typedef struct rnd_bucket rnd_bucket;
struct rnd_bucket
{
    rnd_bucket *Next;
    rnd_pass_list Passes;
    U64 StackGen;
    U64 LastCmdStackGen;
    RndDefineBucketStacks
};

void RndDrawStartUp();
void RndSubmitBucket(os_handle OSWindow, rnd_handle RndWindow, rnd_bucket *Bucket);
rnd_bucket *RndMakeBucket(arena *Arena);
void RndPushBucket(rnd_bucket *Bucket);
void RndPopBucket();
void RndSubBucket(rnd_bucket *Bucket);
rnd_bucket *RndTopBucket();
rnd_rect2d_inst *RndRect(rng2_f32 Pos, vec4_f32 Color, F32 CornerRadius, F32 BorderThickness, F32 EdgeSoftness);
rnd_rect2d_inst *RndImg(rng2_f32 Position, rng2_f32 TexCoords, rnd_handle Texture, vec4_f32 Color, F32 CornerRadius, F32 BorderThickness,
                        F32 EdgeSoftness);

rnd_fancy_run_list RndFancyRunListFromFancyStringList(arena *Arena, rnd_fancy_string_list *Strings);
void RndTextRun(vec2_f32 Position, font_run Run);

#define RndBucketScope(Bucket) DeferLoop(RndPushBucket(Bucket), RndPopBucket())

#endif
