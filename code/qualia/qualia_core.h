#if !defined(QUALIA_CORE_H)
/*  =======================================================================
    File: qualia_core.h
    Date: March 16th 2024  9:11 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define QUALIA_CORE_H

typedef U64 gf_handle;

typedef enum gf_cmd_kind
{
    GF_CMD_KIND_WINDOW_DESTROY,
    GF_CMD_KIND_NEW_PANEL_LEFT,
    GF_CMD_KIND_NEW_PANEL_RIGHT,
    GF_CMD_KIND_NEW_PANEL_UP,
    GF_CMD_KIND_NEW_PANEL_DOWN,
    GF_CMD_KIND_MOVE_TAB,
    GF_CMD_KIND_CLOSE_TAB,
    GF_CMD_KIND_MAKE_POPOUT_WINDOW,
    GF_CMD_KIND_MAKE_MAIN_WINDOW,
    GF_CMD_KIND_OPEN,
}gf_cmd_kind;

typedef struct gf_cmd_params gf_cmd_params;
struct gf_cmd_params
{
    gf_handle Window;
    
    ui_dock_node *Dock;
    ui_dock_node *DestDock;
    ui_view *View;
    ui_view *PrevView;
    F32 Size;
};

typedef struct gf_cmd gf_cmd;
struct gf_cmd
{
    gf_cmd_kind Kind;
    gf_cmd_params *Params;
};

typedef struct gf_cmd_node gf_cmd_node;
struct gf_cmd_node
{
    gf_cmd_node *Next;
    gf_cmd_node *Prev;
    gf_cmd Command;
};

typedef struct gf_cmd_list gf_cmd_list;
struct gf_cmd_list
{
    gf_cmd_node *First;
    gf_cmd_node *Last;
    U64 Count;
};

typedef enum gf_tool_type
{
    GF_TOOL_TYPE_BRUSH,
    GF_TOOL_TYPE_MOVE,
    GF_TOOL_TYPE_ZOOM,
    GF_TOOL_TYPE_ROTATE,
}gf_tool_type;

typedef struct gf_painting_state gf_painting_state;
struct gf_painting_state
{
    gf_painting_state *Next;
    gf_painting_state *Prev;
    im_image Image;
    ui_view *Canvas;
};

void GFCoreStartUp();
void GFCoreBeginFrame(gf_cmd_list *Commands);
void GFCmdListPush(arena *Arena, gf_cmd_list *Commands, gf_cmd_params *Params, gf_cmd_kind Kind);
void GFPushCmdRoot(gf_cmd_params *Params, gf_cmd_kind Kind);
gf_cmd_list GFGatherRootCmds(arena *Arena);

#endif
