#if !defined(UI_BASIC_WIDGETS_H)
/*  =======================================================================
    File: ui_basic_widgets.h
    Date: March 25th 2024  9:40 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define UI_BASIC_WIDGETS_H

ui_signal UISpacer(ui_size Size);

ui_box *UIPaneBegin(rng2_f32 Rect, string8 String);
void UIPaneEnd();

void UINamedRowBegin(string8 String);
void UINamedRowEnd();
void UIRowBegin();
void UIRowEnd();
void UINamedColumnBegin(string8 String);
void UINamedColumnEnd();
void UIColumnBegin();
void UIColumnEnd();

ui_signal UILabel(string8 String);
ui_signal UILabelFmt(string8 Format, ...);

ui_signal UIButton(string8 String);
ui_signal UIButtonFmt(string8 Format, ...);

#define UIPane(Rect, String) DeferLoop(UIPaneBegin(Rect, String), UIPaneEnd())
#define UINamedRow(String) DeferLoop(UIRowBegin(String), UIRowEnd())
#define UIRow DeferLoop(UIRowBegin(), UIRowEnd())
#define UIPadding(Size) DeferLoop(UISpacer(Size), UISpacer(Size))
#define UIColumn DeferLoop(UIColumnBegin(), UIColumnEnd())

typedef enum ui_scroll_list_flags
{
    UI_SCROLL_LIST_FLAGS_NAV = (1<<0),
    UI_SCROLL_LIST_FLAGS_SNAP = (1<<1),
    UI_SCROLL_LIST_FLAGS_ALL = 0xffffffff,
}ui_scroll_list_flags;

typedef struct ui_scroll_list_row_block ui_scroll_list_row_block;
struct ui_scroll_list_row_block
{
    U64 RowCount;
    U64 ItemCount;
};

typedef struct ui_scroll_list_row_block_array ui_scroll_list_row_block_array;
struct ui_scroll_list_row_block_array
{
    ui_scroll_list_row_block *Blocks;
    U64 Count;
};

typedef struct ui_scroll_list_params ui_scroll_list_params;
struct ui_scroll_list_params
{
    ui_scroll_list_flags Flags;
    vec2_f32 DimPx;
    F32 RowHeightPx;
    ui_scroll_list_row_block_array RowBlocks;
    rng2_i64 CursorRange;
    vec2_i64 ItemRange;
    B8 CursorMinIsEmptySelection[AXIS2_COUNT];
};

typedef struct ui_scroll_list_signal ui_scroll_list_signal;
struct ui_scroll_list_signal
{
    B8 CursorMoved;
};

void UIScrollListBegin(ui_scroll_list_params *Params, ui_scroll_pt *ScrollPt, vec2_i64 *MarkOut,
                       vec2_i64 *VisibleRowRangeOut, ui_scroll_list_signal *SignalOut);
void UIScrollListEnd();

void UISliderEnable(string8 Name, string8 Units, F32 *Value, vec2_f32 Range, B8 Enabled);
void UISlider(string8 Name, string8 Units, F32 *Value, vec2_f32 Range);

#define UIScrollList(Params, ScrollPt, MarkOut, VisibleRowRangeOut, SignalOut) \
    DeferLoop(UIScrollListBegin(Params, ScrollPt, MarkOut, VisibleRowRangeOut, SignalOut), UIScrollListEnd())

#endif
