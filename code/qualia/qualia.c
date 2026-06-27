/*  =======================================================================
    File: qualia.c
    Date: March 5th 2024 10:55 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

#ifdef PX_PLATFORM_WINDOWS

// OS Specific Header Files
#define COBJMACROS
#define NOMINMAX

#include <windows.h>

#include <d3d11_1.h>
#include <dxgi1_3.h>

#include <wincodec.h>
#include <wincodecsdk.h>
#include <shobjidl_core.h>

#endif

// Third party
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

// Header Files ------------------------------------------
#include "base/base_types.h"
#include "base/base_arena.h"
#include "base/base_string.h"
#include "base/base_math.h"
#include "base/base_logging.h"
#include "base/base_thread.h"

#include "os/os_core.h"
#include "os/os_gfx.h"

#include "renderer/renderer.h"
#include "renderer/renderer_font.h"
#include "renderer/renderer_draw.h"

#include "ui/ui_core.h"
#include "ui/ui_basic_widgets.h"
#include "ui/ui_view.h"
#include "ui/ui_docking.h"
#include "ui/ui_animation.h"

#include "image/image_core.h"

#include "brush/brush_core.h"
#include "brush/brush_bristles.h"
#include "brush/brush_abr_reader.h"

#include "qualia/qualia_core.h"
#include "qualia/qualia_views.h"
#include "qualia/qualia_gfx.h"

// C Files -----------------------------------------------
#include "base/base_arena.c"
#include "base/base_string.c"
#include "base/base_math.c"
#include "base/base_logging.c"
#include "base/base_thread.c"

#include "os/os_core.c"
#include "os/os_gfx.c"

#include "renderer/renderer.c"
#include "renderer/renderer_font.c"
#include "renderer/renderer_draw.c"

#include "ui/ui_core.c"
#include "ui/ui_basic_widgets.c"
#include "ui/ui_view.c"
#include "ui/ui_docking.c"
#include "ui/ui_animation.c"

#include "image/image_core.c"

#include "brush/brush_core.c"
#include "brush/brush_bristles.c"
#include "brush/brush_abr_reader.c"

#include "qualia/qualia_core.c"
#include "qualia/qualia_views.c"
#include "qualia/qualia_gfx.c"

// OS Specific C Files
#ifdef PX_PLATFORM_WINDOWS

#include "os/win32/os_core_win32.c"
#include "os/win32/os_gfx_win32.c"

#include "renderer/d3d11/d3d11_renderer.c"

#endif

void UpdateAndRender(os_handle RepaintWindow)
{
    temp_arena Scratch = GetScratch(0, 0);

    // Calculate frame delta time
    local_persist U64 FrameTimeUSHistory[64] = {0};
    local_persist U64 FrameTimeUSHistoryIdx = 0;
    F32 TargetHz = OSDefaultRefreshRate();
    if(FrameTimeUSHistoryIdx > 32)
    {
        U64 NumFramesInHistory = Min(ArrayCount(FrameTimeUSHistory), FrameTimeUSHistoryIdx);
        U64 FrameTimeHistorySumUS = 0;
        for(U64 Index = 0; Index < NumFramesInHistory; ++Index)
            FrameTimeHistorySumUS += FrameTimeUSHistory[Index];
        U64 FrameTimeHistoryAvgUS = FrameTimeHistorySumUS / NumFramesInHistory;

        F32 PossibleAlternateHzTargets[] = {TargetHz, 60.0f, 120.0f, 144.0f, 240.0f};
        F32 BestTargetHz = TargetHz;
        I64 BestTargetHzFrameTimeUSDiff = I64Max;
        for(I64 Index = 0; Index < ArrayCount(PossibleAlternateHzTargets); ++Index)
        {
            F32 Candidate = PossibleAlternateHzTargets[Index];
            if(Candidate <= TargetHz)
            {
                U64 CandidateFrameTimeUS = 1000000/(U64)Candidate;
                I64 FrameTimeUSDiff = (I64)FrameTimeHistoryAvgUS - (I64)CandidateFrameTimeUS;
                if(AbsI64(FrameTimeUSDiff) < BestTargetHzFrameTimeUSDiff)
                {
                    BestTargetHz = Candidate;
                    BestTargetHzFrameTimeUSDiff = FrameTimeUSDiff;
                }
            }
        }
        TargetHz = BestTargetHz;
    }
    F32 Delta = 1.0f / TargetHz;
    
    os_event_list Events = {0};
    if(RepaintWindow == 0)
        Events = OSGetEvents(Scratch.Arena);

    U64 BeginTimeUS = OSNowMicroseconds();

    UIDockingUpdate(Delta);
    
    // Drag/drop release functionality 
    if(GFDragIsActive())
    {
        for(os_event *Event = Events.First; Event; Event = Event->Next)
        {
            if(Event->Kind == OS_EVENT_KIND_RELEASE && Event->Key == OS_KEY_LEFT_MOUSE_BUTTON)
            {
                GFQueueDragDrop();
                break;
            }
        }
    }

    // Focus window for dragging/dropping
    if(GFDragIsActive())
    {
        B8 OverFocusedParent = 0;
        B8 OverFocusedChild = 0;
        
        for(gf_window *Window = GFGetActiveWindows(); Window; Window = Window->Next)
        {            
            vec2_f32 Mouse = OSMouseFromWindow(Window->OSWindow);
            rng2_f32 Rect = OSClientRectFromWindow(Window->OSWindow);
            if(OSWindowIsFocused(Window->OSWindow) && Contains2F32(Rect, Mouse))
            {
                OverFocusedParent = Window->Parent == 0;
                OverFocusedChild = Window->Parent != 0;
                break;
            }
        }

        if(!OverFocusedChild || !OverFocusedParent)
        {
            for(gf_window *Window = GFGetActiveWindows(); Window; Window = Window->Next)
            {
                vec2_f32 Mouse = OSMouseFromWindow(Window->OSWindow);
                rng2_f32 Rect = OSClientRectFromWindow(Window->OSWindow);
                if(!OSWindowIsFocused(Window->OSWindow) && Contains2F32(Rect, Mouse))
                {
                    if((Window->Parent == 0 && !OverFocusedParent && !OverFocusedChild) ||
                       (Window->Parent && !OverFocusedChild))
                    {
                        OSWindowFocus(Window->OSWindow);
                        break;
                    }
                }
            }
        }
    }
    
    RndDrawBeginFrame();

    gf_cmd_list Commands = GFGatherRootCmds(Scratch.Arena);
    GFCoreBeginFrame(&Commands);
    GFBeginFrame(&Commands, Delta);
        
    for(gf_window *Window = GFGetActiveWindows(); Window; Window = Window->Next)
    {
        GFWindowUpdateAndRender(Window, &Events);
    }

    for(gf_window *Window = GFGetActiveWindows(); Window; Window = Window->Next)
    {
        RndWindowBeginFrame(Window->OSWindow, Window->RndWindow);
        RndSubmitBucket(Window->OSWindow, Window->RndWindow, Window->Bucket);
        RndWindowEndFrame(Window->OSWindow, Window->RndWindow);
    }

    GFEndFrame();
    RndDrawEndFrame();

    // Process window closing event
    for(os_event *Event = Events.First, *Next = 0; Event; Event = Next)
    {
        Next = Event->Next;
        if(Event->Kind == OS_EVENT_KIND_WINDOW_DESTROY)
        {
            for(gf_window *Window = GFGetActiveWindows(); Window; Window = Window->Next)
            {
                if(Event->Window == Window->OSWindow)
                {
                    gf_cmd_params Params = {0};
                    Params.Window = (gf_handle)Window;
                    GFPushCmdRoot(&Params, GF_CMD_KIND_WINDOW_DESTROY);
                }
            }
        }
    }
    
    U64 EndTimeUS = OSNowMicroseconds();
    U64 FrameTimeUS = EndTimeUS - BeginTimeUS;
    FrameTimeUSHistory[FrameTimeUSHistoryIdx++ % ArrayCount(FrameTimeUSHistory)] = FrameTimeUS;
    
    ReleaseScratch(Scratch);
}

void EntryPoint()
{
    thread_context Thread;
    ThreadContextInitAndEquip(&Thread);
    ThreadContextSetThreadName(Str8Lit("Main"));

    OSStartUp();
    OSGraphicsStartUp();
    RndStartUp();
    FontStartUp();
    RndDrawStartUp();
    UIDockingStartUp();
    GFCoreStartUp();
    GFViewsStartUp();
    GFStartUp(UpdateAndRender);

    br_bristle Bristle = BRBristleInit(1.0f, 12);
    BRBristleUpdateElasticForces(&Bristle);
    
    for(;;)
    {
        UpdateAndRender(0);

        if(GFGetActiveWindows() == 0)
        {
            break;
        }
    }

    ThreadContextRelease();
}

#ifdef PX_PLATFORM_WINDOWS

int APIENTRY
WinMain(HINSTANCE Instance,
        HINSTANCE PreviousInstance,
        PSTR CommandLine,
        int CommandShow)
{
    EntryPoint();
    return 0;
}

#endif
