#if !defined(UI_VIEW_H)
/*  =======================================================================
    File: ui_view.h
    Date: April 11th 2024 10:02 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define UI_VIEW_H

typedef struct ui_view ui_view;

#define UIBuildViewHook(FuncName) void FuncName(ui_view *View, vec2_f32 Size, os_handle Window, F32 Delta)
#define UIStartUpViewHook(FuncName) void FuncName(ui_view *View)
#define UIShutDownViewHook(FuncName) void FuncName(ui_view *View)
typedef void (*ui_build_view_hook)(ui_view *, vec2_f32, os_handle, F32);
typedef void (*ui_start_up_view_hook)(ui_view *);
typedef void (*ui_shut_down_view_hook)(ui_view *);

typedef enum ui_view_kind
{
    UI_VIEW_KIND_NORMAL = 0,
    UI_VIEW_KIND_CONTENT,
    UI_VIEW_KIND_FIXED,
}ui_view_kind;

typedef struct ui_view_spec ui_view_spec;
struct ui_view_spec
{
    string8 Name;
    ui_start_up_view_hook StartUpViewUIHook;
    ui_shut_down_view_hook ShutDownViewUIHook;
    ui_build_view_hook BuildViewUIHook;
};

typedef struct ui_view ui_view;
struct ui_view
{
    ui_view *Next;
    ui_view *Prev;

    void *UserData;

    ui_scroll_pt2 ScrollPos;

    ui_view_spec *Spec;
    string8 Name;
    ui_view_kind Kind;
    arena *Arena;
};

#define UIViewUserState(View, Type)(Type *)UIViewGetOrPushUserState((View), sizeof(Type))

void *UIViewGetOrPushUserState(ui_view *View, U64 Size);

#endif
