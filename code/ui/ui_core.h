 #if !defined(UI_CORE_H)
/*  =======================================================================
    File: ui_core.h
    Date: March 6th 2024  4:33 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define UI_CORE_H

typedef U64 ui_key;

typedef enum ui_box_flags
{
    UI_BOX_FLAG_CLICKABLE = (1<<0),
    UI_BOX_FLAG_DRAW_TEXT = (1<<1),
    UI_BOX_FLAG_DRAW_BACKGROUND = (1<<2),
    UI_BOX_FLAG_FLOATING_X = (1<<3),
    UI_BOX_FLAG_FLOATING_Y = (1<<4),
    UI_BOX_FLAG_FIXED_WIDTH = (1<<5),
    UI_BOX_FLAG_FIXED_HEIGHT = (1<<6),
    UI_BOX_FLAG_ALLOW_OVERFLOW_X = (1<<7),
    UI_BOX_FLAG_ALLOW_OVERFLOW_Y = (1<<8),
    UI_BOX_FLAG_FOCUS_HOT = (1<<9),
    UI_BOX_FLAG_DRAW_HOT_EFFECTS = (1<<10),
    UI_BOX_FLAG_DRAW_ACTIVE_EFFECTS = (1<<11),
    UI_BOX_FLAG_DRAW_BORDER = (1<<12),
    UI_BOX_FLAG_CLIP = (1<<13),
    UI_BOX_FLAG_VIEW_CLAMP_X = (1<<14),
    UI_BOX_FLAG_VIEW_CLAMP_Y = (1<<15),
    UI_BOX_FLAG_VIEW_CLAMP = (UI_BOX_FLAG_VIEW_CLAMP_X | UI_BOX_FLAG_VIEW_CLAMP_Y),
    UI_BOX_FLAG_VIEW_SCROLL_X = (1<<16),
    UI_BOX_FLAG_VIEW_SCROLL_Y = (1<<17),
    UI_BOX_FLAG_VIEW_SCROLL = (UI_BOX_FLAG_VIEW_SCROLL_X | UI_BOX_FLAG_VIEW_SCROLL_Y),
    UI_BOX_FLAG_ANIMATE_POS_X = (1<<18),
    UI_BOX_FLAG_ANIMATE_POS_Y = (1<<19),
    UI_BOX_FLAG_ANIMATE_POS = (UI_BOX_FLAG_ANIMATE_POS_X | UI_BOX_FLAG_ANIMATE_POS_Y),
    UI_BOX_FLAG_DRAW_BUCKET = (1<<20),
    UI_BOX_FLAG_DRAW_OVERLAY = (1<<21),
    UI_BOX_FLAG_ROUND_CHILD_BY_PARENT = (1<<22),
    UI_BOX_FLAG_DRAW_BORDER_FOCUS = (1<<23),
    UI_BOX_FLAG_DROP_SITE = (1<<24),
    UI_BOX_FLAG_SCROLL = (1<<25),
}ui_box_flags;

typedef enum ui_size_kind
{
    UI_SIZE_KIND_NULL,
    UI_SIZE_KIND_PIXELS,
    UI_SIZE_KIND_TEXT_CONTENT,
    UI_SIZE_KIND_PARENT_PCT,
    UI_SIZE_KIND_CHILDREN_SUM,
}ui_size_kind;

typedef struct ui_size ui_size;
struct ui_size
{
    ui_size_kind Kind;
    F32 Value;
    F32 Strictness;
};

typedef enum ui_text_alignment_type
{
    UI_TEXT_ALIGNMENT_TYPE_LEFT,
    UI_TEXT_ALIGNMENT_TYPE_RIGHT,
    UI_TEXT_ALIGNMENT_TYPE_CENTER,
}ui_text_alignment_type;

typedef struct ui_scroll_pt ui_scroll_pt;
struct ui_scroll_pt
{
    I64 Index;
    F32 Offset;
};

typedef union ui_scroll_pt2 ui_scroll_pt2;
union ui_scroll_pt2
{
    ui_scroll_pt V[2];
    struct
    {
        ui_scroll_pt X;
        ui_scroll_pt Y;
    };
};

typedef struct ui_box ui_box;
struct ui_box
{
    ui_box *Next;
    ui_box *Prev;
    ui_box *First;
    ui_box *Last;
    ui_box *Parent;
    
    U64 ChildCount;

    // Hash info retained after frame
    ui_box *HashNext;
    ui_box *HashPrev;

    // Per-build data
    ui_key Key;
    ui_box_flags Flags;
    string8 String;
    rnd_fancy_run_list StringRun;
    font_handle Font;
    F32 FontSize;
    F32 TextPadding;
    vec4_f32 TextColor;
    vec4_f32 BackgroundColor;
    vec4_f32 BorderColor;
    vec4_f32 OverlayColor;
    ui_text_alignment_type TextAlign;
    os_cursor HoverCursor;
    F32 CornerRadii[4];
    F32 Squish;

    rng2_f32 Rect;
    vec2_f32 FixedSize;
    vec2_f32 FixedPosition;
    vec2_f32 ViewOffset;
    vec2_f32 ViewOffsetTarget;
    vec2_f32 ViewBounds;
    axis2 ChildLayoutAxis;
    ui_size PrefSize[AXIS2_COUNT];

    vec2_f32 FixedPositionAnimated;

    rnd_bucket *RndBucket;
    
    F32 HotT;
    F32 ActiveT;

    U64 FirstTouchedBuildIndex;
    U64 LastTouchedBuildIndex;
};

typedef struct ui_box_hash_slot ui_box_hash_slot;
struct ui_box_hash_slot
{
    ui_box *First;
    ui_box *Last;
};

typedef struct ui_box_rec ui_box_rec;
struct ui_box_rec
{
    U32 PushCount;
    U32 PopCount;
    ui_box *Next;
};

typedef enum ui_mouse_button_kind
{
    UI_MOUSE_BUTTON_KIND_LEFT,
    UI_MOUSE_BUTTON_KIND_MIDDLE,
    UI_MOUSE_BUTTON_KIND_RIGHT,
    UI_MOUSE_BUTTON_KIND_COUNT
}ui_mouse_button_kind;

typedef enum ui_signal_flags
{
    UI_SIGNAL_FLAGS_LEFT_PRESSED = (1<<0),
    UI_SIGNAL_FLAGS_MIDDLE_PRESSED = (1<<1),
    UI_SIGNAL_FLAGS_RIGHT_PRESSED = (1<<2),

    UI_SIGNAL_FLAGS_LEFT_RELEASED = (1<<3),
    UI_SIGNAL_FLAGS_MIDDLE_RELEASED = (1<<4),
    UI_SIGNAL_FLAGS_RIGHT_RELEASED = (1<<5),

    UI_SIGNAL_FLAGS_LEFT_CLICKED = (1<<6),
    UI_SIGNAL_FLAGS_MIDDLE_CLICKED = (1<<7),
    UI_SIGNAL_FLAGS_RIGHT_CLICKED = (1<<8),

    UI_SIGNAL_FLAGS_HOVERING = (1<<9),
    UI_SIGNAL_FLAGS_MOUSE_OVER = (1<<10),

    UI_SIGNAL_FLAGS_LEFT_DRAGGING = (1<<11),
    UI_SIGNAL_FLAGS_MIDDLE_DRAGGING = (1<<12),
    UI_SIGNAL_FLAGS_RIGHT_DRAGGING = (1<<13),
}ui_signal_flags;

typedef struct ui_signal ui_signal;
struct ui_signal
{
    ui_box *Box;
    vec2_i16 Scroll;
    ui_signal_flags Flags;
};

#include "generated/ui.meta.h"

UIDefineNodeNils

#define UINodeComp(First, Second) ((First).Next == (Second).Next)

typedef struct ui_state ui_state;
struct ui_state
{
    ui_box *Root;
    ui_box *TooltipRoot;
    ui_box *CtxMenuRoot;
    ui_box_hash_slot *BoxTable;
    U64 BoxTableSize;
    B8 CtxMenuTouchedThisFrame;

    ui_box *FirstFreeBox;
    ui_box *LastFreeBox;

    os_event_list *Events;
    os_handle Window;
    vec2_f32 Mouse;
    
    vec2_f32 DragStartMouse;
    vec4_f32 DragClickOffset;
    vec4_f32 CurrentClickOffset;

    ui_key HotBoxKey;
    ui_key DropHotBoxKey;
    ui_key ActiveBoxKey[UI_MOUSE_BUTTON_KIND_COUNT];
    F32 AnimationDelta;
    F32 TooltipOpenT;

    U64 BuildIndex;

    B8 IsAnimating;
    B8 TooltipOpen;

    font_handle IconFont;
    
    ui_key CtxMenuAnchorKey;
    ui_key NextCtxMenuAnchorKey;
    vec2_f32 CtxMenuAnchorBoxLastPos;
    vec2_f32 CtxMenuAnchorOff;
    B8 CtxMenuOpen;
    B8 NextCtxMenuOpen;
    F32 CtxMenuOpenT;
    ui_key CtxMenuKey;
    B8 CtxMenuChanged;
    
    arena *BuildArena;
    arena *Arena;

    UIDefineStacks
};

ui_state *UIStateAlloc();
void UIStateRelease(ui_state *State);
void UISelectState(ui_state *State);

ui_box_rec UIGetBoxRecursionPost(ui_box *Box);

B8 UIIsFocusActive() {return 1;} // TODO: Implement!!!

void UIBeginBuild(os_event_list *Events, os_handle Window, font_handle IconFont, F32 AnimationDelta);
void UIEndBuild();
ui_box *UIMakeBoxFromKey(ui_box_flags Flags, ui_key Key);
ui_box *UIMakeBoxFromString(ui_box_flags Flags, string8 String);
ui_box *UIMakeBoxFromStringFmt(ui_box_flags Flags, string8 String, ...);
ui_signal UISignalFromBox(ui_box *Box);
ui_box *UIGetBoxRootFromState(ui_state *State);
ui_box *UIGetBoxFromKey(ui_key Key);
ui_key UIGenKey();
ui_key UIKeyFromString(ui_key Seed, string8 String);
ui_key UIKeyFromStringFmt(ui_key Seed, string8 String, ...);

void UICtxMenuOpen(ui_key Key, ui_key AnchorBoxKey, vec2_f32 AnchorOff);
void UICtxMenuClose();
B8 UIBeginCtxMenu(ui_key Key);
void UIEndCtxMenu();

void UITooltipBeginBase();
void UITooltipEndBase();
void UITooltipBegin();
void UITooltipEnd();

#define UITooltip DeferLoop(UITooltipBegin(), UITooltipEnd())
#define UICtxMenu(Key) DeferLoopChecked(UIBeginCtxMenu(Key), UIEndCtxMenu())

vec2_f32 UIMouse();
vec2_f32 UIDragDelta();
os_event_list *UIEvents();
void UISetNextClickOffset(vec4_f32 Offset);
void UISetClickOffset(vec4_f32 Offset);
vec4_f32 UIDragClickOffset();

ui_scroll_pt UIScrollPt(I64 Index, F32 Offset);
void UIScrollPtTargetIndex(ui_scroll_pt *Pt, I64 Index);
void UIScrollPtClampIndex(ui_scroll_pt *Pt, vec2_i64 Range);

void UIPushCornerRadius(F32 Rad);
void UIPopCornerRadius();
#define UICornerRadius(Rad) DeferLoop(UIPushCornerRadius(Rad), UIPopCornerRadius())
void UISetNextPrefSize(axis2 Axis, ui_size Size);

void UIPushPrefSize(axis2 Axis, ui_size Size);
void UIPopPrefSize(axis2 Axis);
#define UIPrefSize(Axis, Size) DeferLoop(UIPushPrefSize(Axis, Size), UIPopPrefSize(Axis))

ui_size UISize(ui_size_kind Kind, F32 Value, F32 Strictness);
#define UIPX(Value, Strictness)        UISize(UI_SIZE_KIND_PIXELS, Value, Strictness)
#define UIEM(Value, Strictness)        UISize(UI_SIZE_KIND_PIXELS, (Value) * UITopFontSize(), Strictness)
#define UITextDim(Padding, Strictness) UISize(UI_SIZE_KIND_TEXT_CONTENT, Padding, Strictness)
#define UIPct(Value, Strictness)       UISize(UI_SIZE_KIND_PARENT_PCT, Value, Strictness)
#define UIChildrenSum(Strictness)      UISize(UI_SIZE_KIND_CHILDREN_SUM, 0.0f, Strictness)

#define UIWidthFill UIPrefWidth(UIPct(1.0f, 0.0f))
#define UIHeightFill UIPrefHeight(UIPct(1.0f, 0.0f))

#define UIRect(Rect) DeferLoop(UIPushRect(Rect), UIPopRect())

#endif
