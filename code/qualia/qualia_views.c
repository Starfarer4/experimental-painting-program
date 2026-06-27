/*  =======================================================================
    File: qualia_views.c
    Date: April 11th 2024 10:14 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

#include "generated/qualia_views.meta.c"

typedef struct gf_views_state gf_views_state;
struct gf_views_state
{
    ui_view_spec *ViewSpecs[GF_VIEW_KIND_COUNT];

    ui_view *FirstFreeView;
    ui_view *LastFreeView;
    
    arena *Arena;
};

global_variable gf_views_state *GFViewsState = 0;

#define GF_VELOCITY_RESOLUTION 32

typedef struct gf_canvas_view_state gf_canvas_view_state;
struct gf_canvas_view_state
{
    gf_painting_state *PaintingState;
    vec2_f32 Offset;
    F32 Zoom;
    F32 Turns;

    F32 Velocity;
    vec2_f32 VelocityDir;
    U32 PastVelocityCount;
    F32 Velocities[GF_VELOCITY_RESOLUTION];
};

typedef struct gf_brush_settings_view_state gf_brush_settings_view_state;
struct gf_brush_settings_view_state
{
    gf_brush_settings_view_kind Kind;
};

void GFViewsStartUp()
{
    arena *Arena = ArenaAlloc(Megabytes(4));
    GFViewsState = PushStruct(Arena, gf_views_state);
    GFViewsState->Arena = Arena;
}

ui_view *GFCreateUIView(gf_view_kind Kind)
{
    if(GFViewsState->ViewSpecs[Kind] == 0)
    {
        ui_view_spec *Spec = PushStruct(GFViewsState->Arena, ui_view_spec);
        Spec->Name = GFViewNames[Kind];
        Spec->StartUpViewUIHook = GFStartUpHookPtrList[Kind];
        Spec->ShutDownViewUIHook = GFShutDownHookPtrList[Kind];
        Spec->BuildViewUIHook = GFBuildHookPtrList[Kind];
        GFViewsState->ViewSpecs[Kind] = Spec;
    }

    ui_view *View = GFViewsState->FirstFreeView;
    if(View)
    {
        DLLRemove(GFViewsState->FirstFreeView, GFViewsState->LastFreeView, View);
        MemoryZero(View, sizeof(ui_view));
    }
    else
        View = PushStruct(GFViewsState->Arena, ui_view);
    
    View->Arena = ArenaAlloc(Kilobytes(4));
    View->Spec = GFViewsState->ViewSpecs[Kind];
    View->Name = View->Spec->Name;
    View->Spec->StartUpViewUIHook(View);

    return View;
}

void GFDeleteUIView(ui_view *View)
{
    View->Spec->ShutDownViewUIHook(View);
    ArenaRelease(View->Arena);
    DLLPushFront(GFViewsState->FirstFreeView, GFViewsState->LastFreeView, View);
}

ui_signal GFToolButton(B8 Selected)
{
    F32 ButtonSize = UITopFontSize();
    
    ui_box_flags Flags = UI_BOX_FLAG_DRAW_BACKGROUND | UI_BOX_FLAG_DRAW_HOT_EFFECTS | UI_BOX_FLAG_DRAW_ACTIVE_EFFECTS |
        UI_BOX_FLAG_CLICKABLE;
    if(Selected) Flags |= UI_BOX_FLAG_DRAW_BORDER_FOCUS;
    
    ui_box *Box = 0;
    UIFixedHeight(ButtonSize) UIRow
    {
        UISpacer(UIPX(2.0f, 1.0f));
            
        UIFixedWidth(ButtonSize)
            UICornerRadius(ButtonSize * 0.15f)
        {
            Box = UIMakeBoxFromString(Flags, Str8Lit("Brush"));
        }
            
        UIParent(Box)
            UIWidthFill
            UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
            UIFont(UIIconFont()) UIFontSize(ButtonSize / 2.0f)
        {
            ui_box *Icon = UIMakeBoxFromString(UI_BOX_FLAG_DRAW_TEXT, GFIconFromKind(GF_ICON_KIND_BRUSH));
        }

        UISpacer(UIPX(2.0f, 1.0f));
    }
    
    ui_signal Signal = UISignalFromBox(Box);
    return Signal;
}

UIStartUpViewHook(GFToolsStartUpView)
{
    View->Kind = UI_VIEW_KIND_FIXED;
}

UIShutDownViewHook(GFToolsShutDownView)
{
}

UIBuildViewHook(GFToolsBuildView)
{
    UIWidthFill UIHeightFill UIColumn UIFontSize(Max(Size.X - 5.0f, 0.0f))
    {
        UISpacer(UIPX(2.0f, 1.0f));
        for(U32 Button = 0; Button < 5; ++Button)
        {
            UISetNextPrefHeight(UIChildrenSum(1.0f));
            ui_box *ButtonContainer = UIMakeBoxFromStringFmt(0, Str8Lit("###StrFmt_%u"), Button);
            UIParent(ButtonContainer)
            {
                GFToolButton(Button == 1);
            }
            UISpacer(UIPX(2.0f, 1.0f));
        }
    }
}

UIStartUpViewHook(GFLayersStartUpView) {}
UIShutDownViewHook(GFLayersShutDownView) {}
UIBuildViewHook(GFLayersBuildView)
{
    ui_box *Box = UIMakeBoxFromString(UI_BOX_FLAG_DRAW_TEXT, Str8Lit("Layers!"));
}

UIStartUpViewHook(GFBrushListerStartUpView)
{
}

UIShutDownViewHook(GFBrushListerShutDownView)
{
}

UIBuildViewHook(GFBrushListerBuildView)
{
    F32 RowHeightPx = FloorF32(UITopFontSize() * 2.5f);

    temp_arena Scratch = GetScratch(0, 0);

    U64 BrushListCount = GFCoreState->LoadedBrushes.Count;
    br_brush **BrushList = PushArray(Scratch.Arena, br_brush *, BrushListCount);
    {
        U64 NodeIndex = 0;
        for(br_brush_node *BrushNode = GFCoreState->LoadedBrushes.First;
            BrushNode;
            BrushNode = BrushNode->Next, ++NodeIndex)
        {
            BrushList[NodeIndex] = &BrushNode->Brush;
        }
    }
    
    ui_scroll_list_params ScrollListParams = {0};
    ScrollListParams.Flags = UI_SCROLL_LIST_FLAGS_ALL;
    ScrollListParams.RowHeightPx = RowHeightPx;
    ScrollListParams.DimPx = Size;
    ScrollListParams.ItemRange = Vec2I64(0, BrushListCount);
    ui_scroll_list_signal ScrollListSignal = {0};
    vec2_i64 VisibleRowRange;
    UIScrollList(&ScrollListParams, &View->ScrollPos.Y, 0, &VisibleRowRange, &ScrollListSignal)
    {
        for(I64 Brush = VisibleRowRange.X; Brush <= VisibleRowRange.Y; ++Brush)
        {
            U32 BackgroundColor = GF_THEME_COLOR_PLAIN_BACKGROUND;
            if(GFCoreState->CurrentBrushLoc == BrushList[Brush])
            {
                U64 BrushSize = OffsetOf(br_brush, LastBrushPosition) - OffsetOf(br_brush, Name);
                if(HashData((U8 *)GFCoreState->CurrentBrushLoc, BrushSize) == HashData((U8 *)&GFCoreState->CurrentBrush, BrushSize))
                    BackgroundColor = GF_THEME_COLOR_ALT_HIGHLIGHT;
                else
                    BackgroundColor = GF_THEME_COLOR_ERR_HIGHLIGHT;
            }
            
            UISetNextBackgroundColor(GFRGBAFromThemeColor(BackgroundColor));
            ui_signal Signal = UIButtonFmt(Str8Lit("%.*s###%i"), Str8VArg(BrushList[Brush]->Name), Brush);
            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_CLICKED)
            {
                GFCoreState->CurrentBrushLoc = BrushList[Brush];
                MemoryCopy(&GFCoreState->CurrentBrush, GFCoreState->CurrentBrushLoc, sizeof(br_brush));
            }
        }
    }

    ReleaseScratch(Scratch);
}

void GFBrushSettingsControlSelector(br_control_type *Type, ui_key Key, B8 Enabled)
{
    ui_box *ControlBox = UIMakeBoxFromStringFmt(UI_BOX_FLAG_DRAW_BORDER | UI_BOX_FLAG_DRAW_BACKGROUND,
                                                Str8Lit("###Control_%uul"), Key);
    UIParent(ControlBox)
        UITextColor(GFRGBAFromThemeColor(Enabled ? GF_THEME_COLOR_PLAIN_TEXT : GF_THEME_COLOR_WEAK_TEXT))
    {
        UIPrefWidth(UITextDim(10.0f, 1.0f))
        {
            UILabel(Str8Lit("Control: "));
        }

        ui_box_flags ExtraSelectorFlags = Enabled ? UI_BOX_FLAG_DRAW_HOT_EFFECTS | UI_BOX_FLAG_DRAW_ACTIVE_EFFECTS : 0;
        ui_box *SelectorBox = UIMakeBoxFromStringFmt(UI_BOX_FLAG_DRAW_BORDER | UI_BOX_FLAG_CLICKABLE | UI_BOX_FLAG_DRAW_BACKGROUND |
                                                     ExtraSelectorFlags, Str8Lit("###Selector_%uul"), Key);
        UIParent(SelectorBox)
        {
            UIWidthFill
            {
                UILabel(BRControlNames[*Type]);
            }

            UIPrefWidth(UIEM(2.35f, 1.0f)) UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
                UIFont(UIIconFont())
            {
                UILabel(GFIconFromKind(GF_ICON_KIND_ARROW_DOWN));
            }
        }
    
        UICtxMenu(Key) UIPrefWidth(UIEM(20.0f, 1.0f))
        {
            for(U32 ControlType = 0; ControlType < BR_CONTROL_TYPE_COUNT; ++ControlType)
            {
                ui_signal Signal = UIButton(BRControlNames[ControlType]);
                if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
                {
                    *Type = ControlType;
                    UICtxMenuClose();
                }
            }
        }

        ui_signal Signal = UISignalFromBox(SelectorBox);
        if((Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED) && Enabled)
        {
            if(UICtxMenuIsOpen(Key))
                UICtxMenuClose();
            else
                UICtxMenuOpen(Key, Signal.Box->Key, Vec2F32(0.0f, Signal.Box->Rect.Bottom - Signal.Box->Rect.Top));
        }
    }
}

UIStartUpViewHook(GFBrushSettingsStartUpView)
{
    gf_brush_settings_view_state *ViewState = UIViewUserState(View, gf_brush_settings_view_state);
    ViewState->Kind = GF_BRUSH_SETTINGS_VIEW_KIND_TIP_SHAPE;
}

UIShutDownViewHook(GFBrushSettingsShutDownView)
{
}

UIBuildViewHook(GFBrushSettingsBuildView)
{
    gf_brush_settings_view_state *ViewState = UIViewUserState(View, gf_brush_settings_view_state);
    
    br_brush *Brush = &GFCoreState->CurrentBrush;
    UITextPadding(4.0f) UIPrefHeight(UIEM(2.5f, 1.0f))
    {
        UIRow
        {
            B8 *BrushKindEnabled[] = {0, &Brush->ShapeDynamics.Enabled, &Brush->Transfer.Enabled};
            
            ui_key BrushMenuKey = UIKeyFromStringFmt(0, Str8Lit("_PropertiesListKey_%p"), View);
            UICtxMenu(BrushMenuKey) UIPrefWidth(UIEM(20.0f, 1.0f))
            {
                for(U32 Kind = 0; Kind < GF_BRUSH_SETTINGS_VIEW_KIND_COUNT; ++Kind)
                {
                    ui_box *BrushKindBox = UIMakeBoxFromStringFmt(0, Str8Lit("###PropertyBox_%i"), Kind);
                    UIParent(BrushKindBox)
                        UIBackgroundColor(GFRGBAFromThemeColor(Kind == ViewState->Kind ? GF_THEME_COLOR_HIGHLIGHT :
                                                               GF_THEME_COLOR_PLAIN_BACKGROUND))
                    {
                        if(BrushKindEnabled[Kind])
                        {
                            UIPrefWidth(UIEM(2.35f, 1.0f)) UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
                                UIFont(UIIconFont())
                            {
                                ui_signal Signal = UIButton(GFIconFromKind(*BrushKindEnabled[Kind] ? GF_ICON_KIND_CHECKED :
                                                                           GF_ICON_KIND_UNCHECKED));
                                if(Signal.Flags & UI_SIGNAL_FLAGS_HOVERING)
                                    ViewState->Kind = Kind;
                                if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_CLICKED)
                                {
                                    ViewState->Kind = Kind;
                                    *BrushKindEnabled[Kind] ^= 1;
                                }
                            }
                        }
                    
                        UIWidthFill
                        {
                            ui_signal Signal = UIButton(GFBrushSettingsViewNames[Kind]);
                            if(Signal.Flags & UI_SIGNAL_FLAGS_HOVERING)
                                ViewState->Kind = Kind;
                            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
                            {
                                ViewState->Kind = Kind;
                                UICtxMenuClose();
                            }
                        }
                    }
                }
            }

            {
                ui_signal Signal = {0};
                UIPrefWidth(UIEM(2.35f, 1.0f)) UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
                    UIFont(UIIconFont())
                    UIBackgroundColor(GFRGBAFromThemeColor(GF_THEME_COLOR_ALT_HIGHLIGHT))
                {
                    Signal = UIButton(GFIconFromKind(GF_ICON_KIND_LIST));
                }

                if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
                {
                    if(UICtxMenuIsOpen(BrushMenuKey))
                        UICtxMenuClose();
                    else
                        UICtxMenuOpen(BrushMenuKey, Signal.Box->Key, Vec2F32(0.0f, Signal.Box->Rect.Bottom - Signal.Box->Rect.Top));
                }
            
                if(Signal.Flags & UI_SIGNAL_FLAGS_HOVERING && !UICtxMenuIsOpen(BrushMenuKey))
                {
                    UITooltip
                    {
                        UILabel(Str8Lit("Brush Properties List"));
                    }
                }
            }

            UIWidthFill
            {
                UILabel(GFBrushSettingsViewNames[ViewState->Kind]);
            }

            if(BrushKindEnabled[ViewState->Kind])
            {
                UIPrefWidth(UIEM(2.35f, 1.0f)) UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
                    UIFont(UIIconFont())
                {
                    ui_signal Signal = UIButton(GFIconFromKind(*BrushKindEnabled[ViewState->Kind] ? GF_ICON_KIND_CHECKED :
                                                               GF_ICON_KIND_UNCHECKED));
                    if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_CLICKED)
                        *BrushKindEnabled[ViewState->Kind] ^= 1;
                }
            }
        }

        switch(ViewState->Kind)
        {
            case GF_BRUSH_SETTINGS_VIEW_KIND_TIP_SHAPE:
            {
                {
                    string8 Name = Str8Lit("Size");
                    F32 MaxValue = (450.0f / 475.0f) * Brush->SizePx + (5000.0f / 18.0f);
                    MaxValue = Max(500.0f, MaxValue);
                    UISlider(Name, Str8Lit("px"), &Brush->SizePx, Vec2F32(1.0f, MaxValue));
                    Brush->SizePx = Min(5000.0f, RoundF32(Brush->SizePx));
                }

                UIRow
                {
                    {
                        string8 Name = Str8Lit("Flip X");
                        ui_box *CheckBox = UIMakeBoxFromString(UI_BOX_FLAG_DRAW_BORDER | UI_BOX_FLAG_DRAW_BACKGROUND, Name);
                        UIParent(CheckBox)
                        {
                            UIWidthFill
                            {
                                UILabel(Name);
                            }
                
                            UIPrefWidth(UIEM(2.35f, 1.0f)) UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
                                UIFont(UIIconFont())
                            {
                                ui_signal Signal = UIButton(GFIconFromKind(Brush->ComputedTip.FlipX ? GF_ICON_KIND_CHECKED :
                                                                           GF_ICON_KIND_UNCHECKED));
                                if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
                                    Brush->ComputedTip.FlipX ^= 1;
                            }
                        }
                    }

                    {
                        string8 Name = Str8Lit("Flip Y");
                        ui_box *CheckBox = UIMakeBoxFromString(UI_BOX_FLAG_DRAW_BORDER | UI_BOX_FLAG_DRAW_BACKGROUND, Name);
                        UIParent(CheckBox)
                        {
                            UIWidthFill
                            {
                                UILabel(Name);
                            }
                
                            UIPrefWidth(UIEM(2.35f, 1.0f)) UITextAlignment(UI_TEXT_ALIGNMENT_TYPE_CENTER)
                                UIFont(UIIconFont())
                            {
                                ui_signal Signal = UIButton(GFIconFromKind(Brush->ComputedTip.FlipY ? GF_ICON_KIND_CHECKED :
                                                                           GF_ICON_KIND_UNCHECKED));
                                if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
                                    Brush->ComputedTip.FlipY ^= 1;
                            }
                        }
                    }
                }
            
                {
                    string8 Name = Str8Lit("Angle");
                    U8 DegreeSymbol[2] = {0xC2, 0xB0};
                    UISlider(Name, Str8(DegreeSymbol, sizeof(DegreeSymbol)), &Brush->ComputedTip.Angle, Vec2F32(-180.0f, 180.0f));
                }

                {
                    string8 Name = Str8Lit("Roundness");
                    F32 Value = Brush->ComputedTip.Roundness * 100.0f;
                    UISlider(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f));
                    Brush->ComputedTip.Roundness = Value / 100.0f;
                }
            
                {
                    string8 Name = Str8Lit("Hardness");
                    F32 Value = Brush->ComputedTip.Hardness * 100.0f;
                    UISlider(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f));
                    Brush->ComputedTip.Hardness = Value / 100.0f;
                }
            
                {
                    string8 Name = Str8Lit("Spacing");
                    F32 Value = Brush->SpacingPct * 100.0f;
                    UISlider(Name, Str8Lit("%"), &Value, Vec2F32(1.0f, 100.0f));
                    Brush->SpacingPct = Value / 100.0f;
                }
            }break;

            case GF_BRUSH_SETTINGS_VIEW_KIND_SHAPE_DYNAMICS:
            {
                B8 Enabled = Brush->ShapeDynamics.Enabled;

                {
                    string8 Name = Str8Lit("Size Jitter");
                    F32 Value = Brush->ShapeDynamics.SizeJitter * 100.0f;
                    UISliderEnable(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f), Enabled);
                    Brush->ShapeDynamics.SizeJitter = Value / 100.0f;
                }

                {
                    ui_key SelectorOptionsKey = UIKeyFromStringFmt(0, Str8Lit("_SizeCtrlSelectListKey_%p"), View);
                    GFBrushSettingsControlSelector(&Brush->ShapeDynamics.SizeControl, SelectorOptionsKey, Enabled);
                }

                {
                    string8 Name = Str8Lit("Minimum");
                    F32 Value = Brush->ShapeDynamics.MinimumDiameter * 100.0f;
                    B8 MinDiameterEnabled = Brush->ShapeDynamics.SizeControl == BR_CONTROL_TYPE_PEN_PRESSURE;
                    UISliderEnable(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f), Enabled && MinDiameterEnabled);
                    Brush->ShapeDynamics.MinimumDiameter = Value / 100.0f;
                }
            }break;

            case GF_BRUSH_SETTINGS_VIEW_KIND_TRANSFER:
            {                
                B8 Enabled = Brush->Transfer.Enabled;

                {
                    string8 Name = Str8Lit("Opacity Jitter");
                    F32 Value = Brush->Transfer.OpacityJitter * 100.0f;
                    UISliderEnable(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f), Enabled);
                    Brush->Transfer.OpacityJitter = Value / 100.0f;
                }

                {
                    ui_key SelectorOptionsKey = UIKeyFromStringFmt(0, Str8Lit("_OpacityCtrlSelectListKey_%p"), View);
                    GFBrushSettingsControlSelector(&Brush->Transfer.OpacityControl, SelectorOptionsKey, Enabled);
                }

                {
                    string8 Name = Str8Lit("Minimum###Opacity");
                    F32 Value = Brush->Transfer.OpacityControlMinimum * 100.0f;
                    B8 MinOpacityEnabled = Brush->Transfer.OpacityControl == BR_CONTROL_TYPE_PEN_PRESSURE;
                    UISliderEnable(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f), Enabled && MinOpacityEnabled);
                    Brush->Transfer.OpacityControlMinimum = Value / 100.0f;
                }
                
                {
                    string8 Name = Str8Lit("Flow Jitter");
                    F32 Value = Brush->Transfer.FlowJitter * 100.0f;
                    UISliderEnable(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f), Enabled);
                    Brush->Transfer.FlowJitter = Value / 100.0f;
                }

                {
                    ui_key SelectorOptionsKey = UIKeyFromStringFmt(0, Str8Lit("_FlowCtrlSelectListKey_%p"), View);
                    GFBrushSettingsControlSelector(&Brush->Transfer.FlowControl, SelectorOptionsKey, Enabled);
                }

                {
                    string8 Name = Str8Lit("Minimum###Flow");
                    F32 Value = Brush->Transfer.FlowControlMinimum * 100.0f;
                    B8 MinFlowEnabled = Brush->Transfer.FlowControl == BR_CONTROL_TYPE_PEN_PRESSURE;
                    UISliderEnable(Name, Str8Lit("%"), &Value, Vec2F32(0.0f, 100.0f), Enabled && MinFlowEnabled);
                    Brush->Transfer.FlowControlMinimum = Value / 100.0f;
                }
            }break;
        }
    }
}

UIStartUpViewHook(GFCanvasStartUpView)
{
    gf_canvas_view_state *ViewState = UIViewUserState(View, gf_canvas_view_state);
    gf_painting_state *PaintingState = GFCoreAllocPaintingState();
    PaintingState->Canvas = View;
    ViewState->PaintingState = PaintingState;
    ViewState->Zoom = 1.0f;
    View->Kind = UI_VIEW_KIND_CONTENT;

    GFCoreState->CurrentPaintingState = PaintingState;
}

UIShutDownViewHook(GFCanvasShutDownView)
{
    gf_canvas_view_state *ViewState = UIViewUserState(View, gf_canvas_view_state);
    gf_painting_state *PaintingState = ViewState->PaintingState;

    GFCoreReleasePaintingState(PaintingState);
}

UIBuildViewHook(GFCanvasBuildView)
{   
    gf_canvas_view_state *ViewState = UIViewUserState(View, gf_canvas_view_state);
    gf_painting_state *PaintingState = ViewState->PaintingState;

    U32 Width = PaintingState->Image.Result.Width;
    U32 Height = PaintingState->Image.Result.Height;
    
    switch(GFCoreState->CurrentTool)
    {
        case GF_TOOL_TYPE_MOVE: UISetNextHoverCursor(OS_CURSOR_HAND_GRAB); break;
        case GF_TOOL_TYPE_ROTATE: UISetNextHoverCursor(OS_CURSOR_HAND_ROTATE); break;
        case GF_TOOL_TYPE_ZOOM: UISetNextHoverCursor(OS_CURSOR_ZOOM_IN); break;
    }
    
    ui_box *Box = 0;
    UIWidthFill UIHeightFill
    {
        Box = UIMakeBoxFromString(UI_BOX_FLAG_CLICKABLE, Str8Lit("###Canvas"));
    }

    ui_signal Signal = {0};

    // Calculate the mouse position on the canvas
    vec2_f32 PosOnCanvas = {0};
    {
        F32 Turns = ViewState->Turns;
        F32 Zoom = ViewState->Zoom * ViewState->Zoom;
        vec2_f32 TopLeft = Add2F32(Box->Rect.TopLeft, Div2F32(Size, Vec2F32(2.0f, 2.0f)));
        PosOnCanvas = Div2F32(Sub2F32(Sub2F32(UIMouse(), TopLeft), ViewState->Offset), Vec2F32(Zoom, Zoom));
        PosOnCanvas = Vec2F32(PosOnCanvas.X * CosF32(Turns) - PosOnCanvas.Y * SinF32(Turns),
                              PosOnCanvas.X * SinF32(Turns) + PosOnCanvas.Y * CosF32(Turns));
        PosOnCanvas = Add2F32(PosOnCanvas, Vec2F32((F32)Width / 2.0f, (F32)Height / 2.0f));
    }
    
    switch(GFCoreState->CurrentTool)
    {
        case GF_TOOL_TYPE_BRUSH:
        {
            UISetNextClickOffset(Vec4F32(UIMouse().X, UIMouse().Y, 0.0f, 0.0f));
            Signal = UISignalFromBox(Box);

            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
                BRResetDraw(&GFCoreState->CurrentBrush, &PaintingState->Image);
            else if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_RELEASED)
                BRResetDraw(&GFCoreState->CurrentBrush, &PaintingState->Image);
            
            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
            {
                os_tablet_state Tablet = OSGetTabletState();
                BRAddBrushStroke(&GFCoreState->CurrentBrush, &PaintingState->Image, PosOnCanvas, ViewState->Turns,
                                 Tablet.Pressure);

                ViewState->Velocity = 0.0f;
            }
        }break;
        
        case GF_TOOL_TYPE_MOVE:
        {
            UISetNextClickOffset(Vec4F32(ViewState->Offset.X, ViewState->Offset.Y, 0.0f, 0.0f));
            Signal = UISignalFromBox(Box);
            
            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
            {
                vec4_f32 DragClickOffset = UIDragClickOffset();
                vec2_f32 PosDelta = Sub2F32(Add2F32(UIDragDelta(), Vec2F32(DragClickOffset.X, DragClickOffset.Y)), ViewState->Offset);
                ViewState->Offset = Add2F32(ViewState->Offset, PosDelta);

                Assert(Length2F32(PosDelta) >= 0);
                ViewState->Velocities[ViewState->PastVelocityCount++ % GF_VELOCITY_RESOLUTION] = Length2F32(PosDelta) / Delta;
                F32 VelocitySum = 0.0f;
                for(U32 VelocityIdx = 0; VelocityIdx < Min(ViewState->PastVelocityCount, GF_VELOCITY_RESOLUTION); ++VelocityIdx)
                    VelocitySum += ViewState->Velocities[VelocityIdx];
                ViewState->Velocity = VelocitySum / Min(ViewState->PastVelocityCount, GF_VELOCITY_RESOLUTION);
                ViewState->VelocityDir = Normalize2F32(PosDelta);
            }
        }break;

        case GF_TOOL_TYPE_ZOOM:
        {
            UISetNextClickOffset(Vec4F32(ViewState->Zoom, 0.0f, 0.0f, 0.0f));
            Signal = UISignalFromBox(Box);
            
            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
            {
                F32 PrevZoom = ViewState->Zoom * ViewState->Zoom;
                    
                F32 PosDelta = (UIDragDelta().X / (Size.X / 8.0f)) + UIDragClickOffset().X - ViewState->Zoom;
                ViewState->Zoom += PosDelta;
                ViewState->Zoom = Clamp(0.1f, ViewState->Zoom, 20.0f);
                F32 Zoom = ViewState->Zoom * ViewState->Zoom;

                // THANK YOU CACTUS VIEWER FOR HELPING ME FIGURE THIS OUT!!!
                vec2_f32 Cursor = Sub2F32(Sub2F32(UIMouse(), UIDragDelta()), Add2F32(Div2F32(Size, Vec2F32(2.0f, 2.0f)),
                                                                                     Box->Rect.TopLeft));
                ViewState->Offset = Sub2F32(ViewState->Offset, Cursor);
                ViewState->Offset = Mul2F32(ViewState->Offset, Vec2F32(Zoom / PrevZoom, Zoom / PrevZoom));
                ViewState->Offset = Add2F32(ViewState->Offset, Cursor);
                
                ViewState->Velocity = 0.0f;
            }
        }break;

        case GF_TOOL_TYPE_ROTATE:
        {
            UISetNextClickOffset(Vec4F32(ViewState->Turns, 0.0f, ViewState->Offset.X, ViewState->Offset.Y));
            Signal = UISignalFromBox(Box);
            
            if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING)
            {
                vec4_f32 DragClickOffset = UIDragClickOffset();
                vec2_f32 MousePos = Sub2F32(UIMouse(), Add2F32(Box->Rect.TopLeft, Div2F32(Size, Vec2F32(2.0f, 2.0f))));
                F32 Rotation = ATan2F32(MousePos.X, MousePos.Y);
                
                vec2_f32 StartMouse = Sub2F32(Sub2F32(UIMouse(), UIDragDelta()),
                                              Add2F32(Box->Rect.TopLeft, Div2F32(Size, Vec2F32(2.0f, 2.0f))));
                F32 StartRotation = ATan2F32(StartMouse.X, StartMouse.Y);
                
                F32 PrevRotation = ViewState->Turns;
                F32 RotationDifference = Rotation - StartRotation;
                F32 RotationDelta = DragClickOffset.X + RotationDifference - ViewState->Turns;
                ViewState->Turns += RotationDelta;

                F32 SinDiff = SinF32(-RotationDifference);
                F32 CosDiff = CosF32(-RotationDifference);
                ViewState->Offset.X = DragClickOffset.Z * CosDiff - DragClickOffset.W * SinDiff;
                ViewState->Offset.Y = DragClickOffset.Z * SinDiff + DragClickOffset.W * CosDiff;

                ViewState->Velocity = 0.0f;
            }
        }break;

        default:
        {
            Signal = UISignalFromBox(Box);
        }break;
    }
    
    if(Signal.Flags & UI_SIGNAL_FLAGS_LEFT_PRESSED)
    {
        GFCoreState->CurrentPaintingState = PaintingState;
    }
    
    // Velocity calculations (TODO: allow for this to be turned off in settings)
    if((Signal.Flags & UI_SIGNAL_FLAGS_LEFT_DRAGGING) == 0 || GFCoreState->CurrentTool != GF_TOOL_TYPE_MOVE)
    {
        F32 Acceleration = GFFontSizeFromPoints(Window, 2000.0f);
        F32 VelocityLen = ViewState->Velocity;
        if(ViewState->Velocity > 0.1f)
        {
            F32 OffsetLen = ViewState->Velocity * Delta;
            vec2_f32 Offset = Mul2F32(ViewState->VelocityDir, Vec2F32(OffsetLen, OffsetLen));
            ViewState->Offset = Add2F32(ViewState->Offset, Offset);
            ViewState->Velocity = Max(ViewState->Velocity - (Acceleration * Delta), 0.0f);
        }
        else
            ViewState->PastVelocityCount = 0;
    }

    F32 Zoom = ViewState->Zoom * ViewState->Zoom;

    //rng2_f32 BoundingRect = {0};
    
    // Ensure the image doesn't stray too far from the origin
    {
        F32 FQAngle = FirstQuadF32(ViewState->Turns);
        vec2_f32 TopRight = Div2F32(Mul2F32(Vec2F32((F32)Width, (F32)Height),
                                            Vec2F32(Zoom, Zoom)), Vec2F32(2.0f, 2.0f));
        TopRight = Vec2F32(TopRight.X * CosF32(-FQAngle) - TopRight.Y * SinF32(-FQAngle),
                           TopRight.X * SinF32(FQAngle) + TopRight.Y * CosF32(FQAngle));
        
        vec2_f32 BoundingDimensions = Mul2F32(TopRight, Vec2F32(2.0f, 2.0f));
        F32 Visibility = Min(Min(BoundingDimensions.X, BoundingDimensions.Y) / 8.0f, 100.0f);
        for(U32 Axis = 0; Axis < AXIS2_COUNT; ++Axis)
        {
            F32 Stop = (Size.V[Axis] + BoundingDimensions.V[Axis]) / 2 - Visibility;
            if(AbsF32(ViewState->Offset.V[Axis]) >= Stop)
            {
                ViewState->Offset.V[Axis] = ViewState->Offset.V[Axis] >= 0.0f ? Stop : -Stop;
                ViewState->VelocityDir.V[Axis] = 0.0f;
            }
        }

        //BoundingRect = Rng2F32(Vec2F32(-TopRight.X, -TopRight.Y), TopRight);
    }

    mat3x3_f32 Canvas = Mul3x3F32(Mul3x3F32(Translate3x3F32(Vec2F32(Trunc(ViewState->Offset.X), Trunc(ViewState->Offset.Y))),
                                            Rotate3x3F32(ViewState->Turns)),
                                  Scale3x3F32(Vec2F32(Zoom, Zoom)));
    mat3x3_f32 CanvasToWorld = Translate3x3F32(Add2F32(Box->Rect.TopLeft, Div2F32(Size, Vec2F32(2.0f, 2.0f))));
    mat3x3_f32 Transform = Mul3x3F32(CanvasToWorld, Canvas);

    
    rnd_bucket *Bucket = RndMakeBucket(GFGetFrameArena());
    RndBucketScope(Bucket)
        RndSamplerKindScope(RND_TEX2D_SAMPLER_KIND_NEAREST)
        RndTransformScope(Transform)
    {
        F32 WidthF32 = (F32)Width;
        F32 HeightF32 = (F32)Height;
        F32 HalfWidth = WidthF32 / 2.0f;
        F32 HalfHeight = HeightF32 / 2.0f;
        rng2_f32 Pos = Rng2F32(Vec2F32(-HalfWidth, -HalfHeight), Vec2F32(HalfWidth, HalfHeight));
        
        rng2_f32 TexCoords = Rng2F32(Vec2F32(0.0f, 0.0f), Vec2F32(WidthF32, HeightF32));

        RndTransformScope(CanvasToWorld)
        {
            rnd_pass_params_checkerboard *Checkerboard = RndCheckerboard(Pos, ViewState->Offset, ViewState->Turns, Zoom, 100.0f,
                                                                         GFRGBAFromThemeColor(GF_THEME_COLOR_CHECKERBOARD_COLOR_1),
                                                                         GFRGBAFromThemeColor(GF_THEME_COLOR_CHECKERBOARD_COLOR_2),
                                                                         Vec4F32(0.0f, 0.0f, 0.0f, 0.0f));
        }
        
#if 0   // NOTE: Renders a bounding rect around the canvas
        mat3x3_f32 Canvas = Translate3x3F32(ViewState->Offset);
        RndTransformScope(Mul3x3F32(CanvasToWorld, Canvas))
        {
            rnd_rect2d_inst *Image = RndImg(BoundingRect, TexCoords, PaintingState->Image.Texture, Vec4F32(1.0f, 1.0f, 1.0f, 1.0f),
                                            0.0f, 0.0f, 0.0f);
        }
#endif
        
        rnd_rect2d_inst *Image = RndImg(Pos, TexCoords, PaintingState->Image.Texture, Vec4F32(1.0f, 1.0f, 1.0f, 1.0f),
                                        0.0f, 0.0f, 0.0f);
    }

    UIBoxEquipRndBucket(Box, Bucket);
}
