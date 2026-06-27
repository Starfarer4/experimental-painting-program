#if !defined(UI_DOCKING_H)
/*  =======================================================================
    File: ui_docking.h
    Date: April 8th 2024 12:53 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define UI_DOCKING_H

typedef enum
{
    UI_DOCKSPACE_SPLIT_DIR_LEFT,
    UI_DOCKSPACE_SPLIT_DIR_RIGHT,
    UI_DOCKSPACE_SPLIT_DIR_UP,
    UI_DOCKSPACE_SPLIT_DIR_DOWN,
    UI_DOCKSPACE_SPLIT_DIR_COUNT
}ui_dockspace_split_dir;

typedef enum
{
    UI_DOCK_NODE_KIND_CENTRAL,
    UI_DOCK_NODE_KIND_GLOBAL,
    UI_DOCK_NODE_KIND_LOCAL
}ui_dock_node_kind;

typedef enum
{
    UI_DOCK_NODE_FLAGS_FIXED_SIZE_X = (1<<0),
    UI_DOCK_NODE_FLAGS_FIXED_SIZE_Y = (1<<1),
    UI_DOCK_NODE_FLAGS_ONE_TAB = (1<<2),
}ui_dock_node_flags;

typedef struct ui_dock_node_tabs ui_dock_node_tabs;
struct ui_dock_node_tabs
{
    ui_view *FirstTabView;
    ui_view *LastTabView;
    U32 TabViewCount;
    ui_view *SelectedTabView;
};

typedef struct ui_dock_node ui_dock_node;
struct ui_dock_node
{
    ui_dock_node *Next;
    ui_dock_node *Prev;
    
    ui_dock_node *First;
    ui_dock_node *Last;

    ui_dock_node *Parent;

    ui_dock_node_kind Kind;
    ui_dock_node_flags Flags;
    ui_dockspace_split_dir GlobalSplitDir; // NOTE: Only applicable to global nodes
    axis2 SplitAxis;
    ui_dock_node_tabs Tabs;

    B8 WantLockSizeOnce;
    B8 ChangingDockBoundaries;

    vec2_f32 Size;
    vec2_f32 SizeRef;
    vec2_f32 Position;

    vec2_f32 AnimatedPos;
    vec2_f32 AnimatedSize;
};

typedef struct ui_dock_node_rec ui_dock_node_rec;
struct ui_dock_node_rec
{
    U32 PushCount;
    U32 PopCount;
    ui_dock_node *Next;
};

typedef struct ui_dock_node_list_node ui_dock_node_list_node;
struct ui_dock_node_list_node
{
    ui_dock_node *Node;
    ui_dock_node_list_node *Next;
    ui_dock_node_list_node *Prev;
};

typedef struct ui_dock_node_list ui_dock_node_list;
struct ui_dock_node_list
{
    ui_dock_node_list_node *First;
    ui_dock_node_list_node *Last;
    U32 Size;
};

void UIDockingStartUp();
ui_dock_node *UIDockBuilderAllocNode();
void UIDockBuilderFreeNode(ui_dock_node *Node);
void UIDockNodeTreeUpdateLocalPosSizeRec(ui_dock_node *Node, vec2_f32 Position, vec2_f32 Size);
void UIDockNodeTreeUpdateSplitterFindTouchingNodeRec(arena *Arena, ui_dock_node *Node, axis2 Axis, U32 Side,
                                                     ui_dock_node_list *TouchingNodes);
B8 UIDockingSplitterBehavior(rng2_f32 BoundingBox, ui_dock_node *Child1, ui_dock_node *Child2, axis2 Axis, F32 *Size1, F32 *Size2,
                             F32 MinSize1, F32 MinSize2);
void UILocalDockNodeTreeUpdateSplitterRec(ui_dock_node *Node);
void UIUpdateDockNodeForDPIChange(ui_dock_node *Node, rng2_f32 ContentRect, F32 DPIScaleFactor);
void UIDockNodeMoveChildNodes(ui_dock_node *DstNode, ui_dock_node *SrcNode);
void UIDockNodeMoveTabs(ui_dock_node *Dst, ui_dock_node *Src);
ui_dock_node *UIDockBuilderCreateNode(ui_dock_node_kind Kind, ui_dock_node_flags Flags, rng2_f32 ContentRect);
ui_dock_node *UIDockBuilderSplitNode(ui_dock_node *NodeToSplit, ui_dockspace_split_dir SplitDir, F32 DockSize, ui_dock_node **NewNode,
                                     ui_dock_node **OldNode, ui_dock_node_flags Flags, B8 SplitAsLocal);
void UIUpdateDockNodePosSize(ui_dock_node *Node, rng2_f32 ContentRect);
void UIUpdateDockNode(ui_dock_node *Node, rng2_f32 ContentRect, B8 SkipAnimation);
ui_dock_node *UIGetDockNodeRecursionPre(ui_dock_node *Node);
ui_dock_node_rec UIGetDockNodeRecursionPost(ui_dock_node *Node);
void UIDockNodeDockTabView(ui_dock_node *Node, ui_view *View);
void UIDockNodeRemoveTabView(ui_dock_node *Node, ui_view *View);
void UIDockNodeInsertTabView(ui_dock_node *Node, ui_view *PrevView, ui_view *View);
void UIDockNodeTreeMerge(ui_dock_node *Parent, ui_dock_node *MergeLeadChild);
void UIDockingRemoveNode(ui_dock_node *Node, B8 MergeSiblingIntoParent);

#endif
