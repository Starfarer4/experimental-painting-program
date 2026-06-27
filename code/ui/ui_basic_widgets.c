/*  =======================================================================
    File: ui_basic_widgets.c
    Date: March 25th 2024  9:40 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

ui_signal UISpacer(ui_size Size)
{
    ui_box *Parent = UITopParent();
    UISetNextPrefSize(Parent->ChildLayoutAxis, Size);
    ui_box *Box = UIMakeBoxFromString(0, Str8Lit(""));
    ui_signal Interact = UISignalFromBox(Box);
    return Interact;
}

ui_box *UIPaneBegin(rng2_f32 Rect, string8 String)
{
    UIPushRect(Rect);
    UISetNextChildLayoutAxis(AXIS2_Y);
    ui_box *Box = UIMakeBoxFromString(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_DRAW_BACKGROUND, String);
    UIPopRect();
    UIPushParent(Box);
    UIPushPrefWidth(UIPct(1.0f, 0.0f));
    return Box;
}

void UIPaneEnd()
{
    UIPopPrefWidth();
    UIPopParent();
}

void UINamedRowBegin(string8 String)
{
    UISetNextChildLayoutAxis(AXIS2_X);
    ui_box *Box = UIMakeBoxFromString(0, String);
    UIPushParent(Box);
}

void UINamedRowEnd()
{
    UIPopParent();
}

void UINamedColumnBegin(string8 String)
{
    UISetNextChildLayoutAxis(AXIS2_Y);
    ui_box *Box = UIMakeBoxFromString(0, String);
    UIPushParent(Box);
}

void UINamedColumnEnd()
{
    UIPopParent();
}

ui_signal UILabel(string8 String)
{
    ui_box *Box = UIMakeBoxFromString(UI_BOX_FLAG_DRAW_TEXT, String);
    ui_signal Interact = UISignalFromBox(Box);
    return Interact;
}

ui_signal UILabelFmt(string8 Format, ...)
{
    temp_arena Scratch = GetScratch(0, 0);
    va_list Args;
    va_start(Args, Format);
    string8 String = PushStr8FmtV(Scratch.Arena, Format, Args);
    va_end(Args);
    ui_signal Result = UILabel(String);
    ReleaseScratch(Scratch);
    return Result;
}

void UIRowBegin() {UINamedRowBegin(Str8Lit(""));}
void UIRowEnd() {UINamedRowEnd();}
void UIColumnBegin() {UINamedColumnBegin(Str8Lit(""));}
void UIColumnEnd() {UINamedColumnEnd();}

ui_signal UIButton(string8 String)
{
    ui_box *Box = UIMakeBoxFromString(UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_DRAW_BACKGROUND | UI_BOX_FLAG_DRAW_BORDER |
                                      UI_BOX_FLAG_DRAW_TEXT | UI_BOX_FLAG_DRAW_HOT_EFFECTS | UI_BOX_FLAG_DRAW_ACTIVE_EFFECTS, String);
    ui_signal Interact = UISignalFromBox(Box);
    return Interact;
}

ui_signal UIButtonFmt(string8 Format, ...)
{
    temp_arena Scratch = GetScratch(0, 0);
    va_list Args;
    va_start(Args, Format);
    string8 String = PushStr8FmtV(Scratch.Arena, Format, Args);
    va_end(Args);
    ui_signal Result = UIButton(String);
    ReleaseScratch(Scratch);
    return Result;
}

ui_scroll_pt UIScrollBar(axis2 Axis, ui_size OffAxisSize, ui_scroll_pt Pt, vec2_i64 IndexRange, I64 ViewNumIndices)
{
    I64 IndexRangeDimensions = Max((IndexRange.Y - IndexRange.X), 1);

    // Build main container
    UISetNextPrefSize(Axis2Flip(Axis), OffAxisSize);
    UISetNextChildLayoutAxis(Axis);
    ui_box *ContainerBox = UIMakeBoxFromKey(UI_BOX_FLAG_DRAW_BORDER, 0);

#if 0    
    // Build scroll-min button
    ui_signal MinScroll.Signal = {0};
    UIParent(ContainerBox)
        UIPrefSize(Axis, OffAxisSize)
        UIFlags(UI_BOX_FLAG_DRAW_BORDER)
        UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
    {
        string8 ArrowString = Axis == AXIS2_X ? Str8Lit("<") : Str8Lit("^");
        MinScrollSignal = UIButtonFmt(Str8Lit("%.*s##_MinScroll_%i"), ArrowString, Axis);
    }
#endif

    ui_signal SpaceBeforeSignal = {0};
    ui_signal SpaceAfterSignal = {0};
    ui_signal ScrollerSignal = {0};
    ui_box *ScrollAreaBox = 0;
    ui_box *ScrollerBox = 0;
    UIParent(ContainerBox)
    {
        UISetNextPrefSize(Axis, UIPct(1.0f, 0.0f));
        UISetNextChildLayoutAxis(Axis);
        ScrollAreaBox = UIMakeBoxFromStringFmt(0, Str8Lit("##ScrollArea_%i"), Axis);
        UIParent(ScrollAreaBox)
        {
            // Space before
            if(IndexRange.Y != IndexRange.X)
            {
                UISetNextPrefSize(Axis, UIPct((F32)((F64)(Pt.Index - IndexRange.X) / (F64)IndexRangeDimensions), 0.0f));
                ui_box *SpaceBeforeBox = UIMakeBoxFromStringFmt(UI_BOX_FLAG_CLICKABLE, Str8Lit("##ScrollAreaBefore"));
                SpaceBeforeSignal = UISignalFromBox(SpaceBeforeBox);
            }

            // Scroller
            UIPrefSize(Axis, UIPct(Clamp(0.01f, (F32)((F64)Max(ViewNumIndices, 1) / (F64)IndexRangeDimensions), 1.0f), 0.0f))
            {                
                ScrollerSignal = UIButtonFmt(Str8Lit("##Scroller_%i"), Axis);
                ScrollerBox = ScrollerSignal.Box;
            }

            // Space after
            if(IndexRange.Y != IndexRange.X)
            {
                UISetNextPrefSize(Axis, UIPct(1.0f - (F32)((F64)(Pt.Index - IndexRange.X) / (F64)IndexRangeDimensions), 0.0f));
                ui_box *SpaceAfterBox = UIMakeBoxFromStringFmt(UI_BOX_FLAG_CLICKABLE, Str8Lit("##ScrollAreaAfter"));
                SpaceAfterSignal = UISignalFromBox(SpaceAfterBox);
            }
        }
    }

    // Pt * Signals -> new Pt
    ui_scroll_pt NewPt = Pt;
    {
        if(ScrollerSignal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
        {
            if(ScrollerSignal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
            {
                F32 ScrollSpacePx = FloorF32(Dim2F32(ScrollAreaBox->Rect).V[Axis]) - FloorF32(Dim2F32(ScrollerBox->Rect).V[Axis]);
                vec4_f32 DragData = {0};
                F32 *DragDataPtr = DragData.V;
                *((I64 *)DragDataPtr) = Pt.Index; DragDataPtr += 2;
                *(DragDataPtr++) = Pt.Offset;
                *(DragDataPtr++) = ScrollSpacePx;
                UISetClickOffset(DragData);
            }
            
            ui_scroll_pt OriginalPt = {0};
            F32 ScrollSpacePx = 0.0f;
            {
                vec4_f32 DragData = UIDragClickOffset();
                F32 *DragDataPtr = DragData.V;
                OriginalPt.Index = *((I64 *)DragDataPtr); DragDataPtr += 2;
                OriginalPt.Offset = *(DragDataPtr++);
                ScrollSpacePx = *(DragDataPtr++);
            }
            F32 DragDelta = UIDragDelta().V[Axis];
            F32 DragPct = DragDelta / ScrollSpacePx;
            I64 NewIndex = OriginalPt.Index + (I64)(DragPct * IndexRangeDimensions);
            NewIndex = Clamp(IndexRange.X, NewIndex, IndexRange.Y);
            UIScrollPtTargetIndex(&NewPt, NewIndex);
            NewPt.Offset = 0;
        }

        if(SpaceBeforeSignal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
        {
            I64 NewIndex = NewPt.Index - 1;
            NewIndex = Clamp(IndexRange.X, NewIndex, IndexRange.Y);
            UIScrollPtTargetIndex(&NewPt, NewIndex);
        }
        
        if(SpaceAfterSignal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
        {
            I64 NewIndex = NewPt.Index + 1;
            NewIndex = Clamp(IndexRange.X, NewIndex, IndexRange.Y);
            UIScrollPtTargetIndex(&NewPt, NewIndex);
        }        
    }

    return NewPt;
}

per_thread ui_scroll_pt *UIScrollListScrollPtPtr = 0;
per_thread F32 UIScrollListScrollBarDimPx = 0;
per_thread vec2_f32 UIScrollListDimPx = {0};
per_thread vec2_i64 UIScrollListScrollIndexRange = {0};

void UIScrollListBegin(ui_scroll_list_params *Params, ui_scroll_pt *ScrollPt, vec2_i64 *MarkOut,
                       vec2_i64 *VisibleRowRangeOut, ui_scroll_list_signal *SignalOut)
{
    vec2_i64 ScrollRowIndexRange = Vec2I64(Params->ItemRange.X, ClampBot(Params->ItemRange.X, Params->ItemRange.Y - 1));
    I64 NumPossibleVisibleRows = (I64)(Params->DimPx.Y / Params->RowHeightPx);

    vec2_i64 VisibleRowRange = Vec2I64(ScrollPt->Index + (I64)(ScrollPt->Offset) + 0 - !!(ScrollPt->Offset < 0),
                                       ScrollPt->Index + (I64)(ScrollPt->Offset) + 0 + NumPossibleVisibleRows + 1);
    VisibleRowRange.X = Clamp(ScrollRowIndexRange.X, VisibleRowRange.X, ScrollRowIndexRange.Y);
    VisibleRowRange.Y = Clamp(ScrollRowIndexRange.X, VisibleRowRange.Y, ScrollRowIndexRange.Y);
    *VisibleRowRangeOut = VisibleRowRange;

    UIScrollListScrollBarDimPx = UITopFontSize() * 1.5f;
    UIScrollListScrollPtPtr = ScrollPt;
    UIScrollListDimPx = Params->DimPx;
    UIScrollListScrollIndexRange = ScrollRowIndexRange;

    // Build top-level container
    ui_box *ContainerBox = 0;
    UIFixedWidth(Params->DimPx.X) UIFixedHeight(Params->DimPx.Y) UIChildLayoutAxis(AXIS2_X)
    {
        ContainerBox = UIMakeBoxFromKey(0, 0);
    }

    // Build scrollable container
    ui_box *ScrollableContainerBox = 0;
    UIParent(ContainerBox) UIChildLayoutAxis(AXIS2_Y) UIFixedWidth(Params->DimPx.X - UIScrollListScrollBarDimPx)
        UIFixedHeight(Params->DimPx.Y)
    {
        ScrollableContainerBox = UIMakeBoxFromString(UI_BOX_FLAG_CLIP | UI_BOX_FLAG_ALLOW_OVERFLOW_Y | UI_BOX_FLAG_SCROLL,
                                                     Str8Lit("###sp"));
        ScrollableContainerBox->ViewOffset.Y = ScrollableContainerBox->ViewOffsetTarget.Y =
            Params->RowHeightPx * ModF32(ScrollPt->Offset, 1.0f) + Params->RowHeightPx * (ScrollPt->Offset < 0) -
            Params->RowHeightPx * (ScrollPt->Offset == -1.0f && ScrollPt->Index == 1);
    }

    // Build vertical scroll bar
    UIParent(ContainerBox)
    {
        UISetNextFixedWidth(UIScrollListScrollBarDimPx);
        UISetNextFixedHeight(UIScrollListDimPx.Y);
        *UIScrollListScrollPtPtr = UIScrollBar(AXIS2_Y, UIPX(UIScrollListScrollBarDimPx, 1.0f), *UIScrollListScrollPtPtr,
                                               ScrollRowIndexRange, NumPossibleVisibleRows);
    }

    // Begin scrollable region
    UIPushParent(ContainerBox);
    UIPushParent(ScrollableContainerBox);
    UIPushPrefHeight(UIPX(Params->RowHeightPx, 1.0f));
}

void UIScrollListEnd()
{
    UIPopPrefHeight();
    ui_box *ScrollableContainerBox = UITopParent(); UIPopParent();
    ui_box *ContainerBox = UITopParent(); UIPopParent();

    ui_signal Signal = UISignalFromBox(ScrollableContainerBox);
    if(Signal.Scroll.Y != 0)
    {
        I64 NewIndex = UIScrollListScrollPtPtr->Index + Signal.Scroll.Y;
        NewIndex = Clamp(UIScrollListScrollIndexRange.X, NewIndex, UIScrollListScrollIndexRange.Y);
        UIScrollPtTargetIndex(UIScrollListScrollPtPtr, NewIndex);
    }
    UIScrollPtClampIndex(UIScrollListScrollPtPtr, UIScrollListScrollIndexRange);
}

void UISliderEnable(string8 Name, string8 Units, F32 *Value, vec2_f32 Range, B8 Enabled)
{
    UISetNextHoverCursor(Enabled ? OS_CURSOR_LEFT_RIGHT : OS_CURSOR_POINTER);
    F32 SliderPct = (*Value - Range.X) / (Range.Y - Range.X);
    ui_box_flags ExtraSliderFlags = Enabled ? UI_BOX_FLAG_DRAW_HOT_EFFECTS : 0;
    ui_box *SliderBox = UIMakeBoxFromString(UI_BOX_FLAG_DRAW_BORDER | UI_BOX_FLAG_DRAW_BACKGROUND | UI_BOX_FLAG_CLICKABLE |
                                            ExtraSliderFlags, Name);
    UIParent(SliderBox)
        UITextColor(GFRGBAFromThemeColor(Enabled ? GF_THEME_COLOR_PLAIN_TEXT : GF_THEME_COLOR_WEAK_TEXT))
    {
        UILabelFmt(Str8Lit("%.*s:"), Str8VArg(Name));

        UISetNextTextAlignment(UI_TEXT_ALIGNMENT_TYPE_RIGHT);
        UILabelFmt(Str8Lit("%.0f%.*s"), *Value, Str8VArg(Units));
        
        UIPrefWidth(UIPct(SliderPct, 1.0f)) UIHeightFill UIFixedX(0) UIFixedY(0)
            UIBackgroundColor(Vec4F32(0.835f, 0.917f, 0.925f, Enabled ? 0.1f : 0.05f))
        {
            UIMakeBoxFromKey(UI_BOX_FLAG_DRAW_BACKGROUND, 0);
        }
    }

    UISetNextClickOffset(Vec4F32(*Value, (Range.Y - Range.X), 0.0f, 0.0f));
    ui_signal Signal = UISignalFromBox(SliderBox);
    if((Signal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING) && Enabled)
    {
        F32 PreDragValue = UIDragClickOffset().X;
        vec2_f32 Delta = UIDragDelta();
        F32 PostDragValue = PreDragValue + (Delta.X * UIDragClickOffset().Y / SliderBox->FixedSize.X);
        PostDragValue = Clamp(Range.X, PostDragValue, Range.Y);
        *Value = PostDragValue;
    }
}

void UISlider(string8 Name, string8 Units, F32 *Value, vec2_f32 Range)
{
    UISliderEnable(Name, Units, Value, Range, 1);
}
