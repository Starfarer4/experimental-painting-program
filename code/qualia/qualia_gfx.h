#if !defined(QUALIA_GFX_H)
/*  =======================================================================
    File: qualia_gfx.h
    Date: March 7th 2024 10:49 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define QUALIA_GFX_H

#include "generated/qualia_gfx.meta.h"

typedef struct gf_window gf_window;
struct gf_window
{
    os_handle OSWindow;
    rnd_handle RndWindow;
    gf_window *Parent;

    rng2_f32 LastWindowRect;
    F32 LastDPI;
    
    ui_dock_node *RootDockNode;
    
    ui_state *UI;
    ui_animation_state *UIAnimationState;
    rnd_bucket *Bucket;
    
    gf_window *Next;
    gf_window *Prev;

    U64 FramesAlive;
};

arena *GFGetFrameArena();
gf_window *GFGetActiveWindows();
void GFStartUp(void (*RepaintHook)(os_handle RepaintWindow));
gf_window *GFWindowOpen(gf_window *Parent, U32 Width, U32 Height, U32 X, U32 Y);
F32 GFFontSizeFromPoints(os_handle OSWindow, F32 Points);

void GFBeginFrame(gf_cmd_list *Commands, F32 Delta);
void GFEndFrame();
void GFWindowUpdateAndRender(gf_window *Window, os_event_list *Events);

vec4_f32 GFRGBAFromThemeColor(gf_theme_color Color);
string8 GFIconFromKind(gf_icon_kind Kind);

void GFQueueDragDrop();
B8 GFDragIsActive();

#endif
