void UIPushParent(ui_box * Parent) {UIStackPushImpl(UIState, ui_parent_node, ParentStack, Parent)}
void UIPushFlags(ui_box_flags Flags) {UIStackPushImpl(UIState, ui_flags_node, FlagsStack, Flags)}
void UIPushFont(font_handle Font) {UIStackPushImpl(UIState, ui_font_node, FontStack, Font)}
void UIPushFontSize(F32 FontSize) {UIStackPushImpl(UIState, ui_font_size_node, FontSizeStack, FontSize)}
void UIPushPrefWidth(ui_size PrefWidth) {UIStackPushImpl(UIState, ui_pref_width_node, PrefWidthStack, PrefWidth)}
void UIPushPrefHeight(ui_size PrefHeight) {UIStackPushImpl(UIState, ui_pref_height_node, PrefHeightStack, PrefHeight)}
void UIPushTextPadding(F32 TextPadding) {UIStackPushImpl(UIState, ui_text_padding_node, TextPaddingStack, TextPadding)}
void UIPushTextColor(vec4_f32 TextColor) {UIStackPushImpl(UIState, ui_text_color_node, TextColorStack, TextColor)}
void UIPushFixedWidth(F32 FixedWidth) {UIStackPushImpl(UIState, ui_fixed_width_node, FixedWidthStack, FixedWidth)}
void UIPushFixedHeight(F32 FixedHeight) {UIStackPushImpl(UIState, ui_fixed_height_node, FixedHeightStack, FixedHeight)}
void UIPushFixedX(F32 FixedX) {UIStackPushImpl(UIState, ui_fixed_x_node, FixedXStack, FixedX)}
void UIPushFixedY(F32 FixedY) {UIStackPushImpl(UIState, ui_fixed_y_node, FixedYStack, FixedY)}
void UIPushBackgroundColor(vec4_f32 BackgroundColor) {UIStackPushImpl(UIState, ui_backgrond_color_node, BackgroundColorStack, BackgroundColor)}
void UIPushBorderColor(vec4_f32 BorderColor) {UIStackPushImpl(UIState, ui_border_color_node, BorderColorStack, BorderColor)}
void UIPushOverlayColor(vec4_f32 OverlayColor) {UIStackPushImpl(UIState, ui_overlay_color_node, OverlayColorStack, OverlayColor)}
void UIPushChildLayoutAxis(axis2 ChildLayoutAxis) {UIStackPushImpl(UIState, ui_child_layout_axis_node, ChildLayoutAxisStack, ChildLayoutAxis)}
void UIPushTextAlignment(ui_text_alignment_type TextAlignment) {UIStackPushImpl(UIState, ui_text_alignment_node, TextAlignmentStack, TextAlignment)}
void UIPushHoverCursor(os_cursor HoverCursor) {UIStackPushImpl(UIState, ui_hover_cursor_node, HoverCursorStack, HoverCursor)}
void UIPushCornerRadius0(F32 CornerRadius0) {UIStackPushImpl(UIState, ui_corner_radius0_node, CornerRadius0Stack, CornerRadius0)}
void UIPushCornerRadius1(F32 CornerRadius1) {UIStackPushImpl(UIState, ui_corner_radius1_node, CornerRadius1Stack, CornerRadius1)}
void UIPushCornerRadius2(F32 CornerRadius2) {UIStackPushImpl(UIState, ui_corner_radius2_node, CornerRadius2Stack, CornerRadius2)}
void UIPushCornerRadius3(F32 CornerRadius3) {UIStackPushImpl(UIState, ui_corner_radius3_node, CornerRadius3Stack, CornerRadius3)}
void UIPushSquish(F32 Squish) {UIStackPushImpl(UIState, ui_squish_node, SquishStack, Squish)}
void UIPopParent() {UIStackPopImpl(UIState, ParentStack)}
void UIPopFlags() {UIStackPopImpl(UIState, FlagsStack)}
void UIPopFont() {UIStackPopImpl(UIState, FontStack)}
void UIPopFontSize() {UIStackPopImpl(UIState, FontSizeStack)}
void UIPopPrefWidth() {UIStackPopImpl(UIState, PrefWidthStack)}
void UIPopPrefHeight() {UIStackPopImpl(UIState, PrefHeightStack)}
void UIPopTextPadding() {UIStackPopImpl(UIState, TextPaddingStack)}
void UIPopTextColor() {UIStackPopImpl(UIState, TextColorStack)}
void UIPopFixedWidth() {UIStackPopImpl(UIState, FixedWidthStack)}
void UIPopFixedHeight() {UIStackPopImpl(UIState, FixedHeightStack)}
void UIPopFixedX() {UIStackPopImpl(UIState, FixedXStack)}
void UIPopFixedY() {UIStackPopImpl(UIState, FixedYStack)}
void UIPopBackgroundColor() {UIStackPopImpl(UIState, BackgroundColorStack)}
void UIPopBorderColor() {UIStackPopImpl(UIState, BorderColorStack)}
void UIPopOverlayColor() {UIStackPopImpl(UIState, OverlayColorStack)}
void UIPopChildLayoutAxis() {UIStackPopImpl(UIState, ChildLayoutAxisStack)}
void UIPopTextAlignment() {UIStackPopImpl(UIState, TextAlignmentStack)}
void UIPopHoverCursor() {UIStackPopImpl(UIState, HoverCursorStack)}
void UIPopCornerRadius0() {UIStackPopImpl(UIState, CornerRadius0Stack)}
void UIPopCornerRadius1() {UIStackPopImpl(UIState, CornerRadius1Stack)}
void UIPopCornerRadius2() {UIStackPopImpl(UIState, CornerRadius2Stack)}
void UIPopCornerRadius3() {UIStackPopImpl(UIState, CornerRadius3Stack)}
void UIPopSquish() {UIStackPopImpl(UIState, SquishStack)}
void UISetNextParent(ui_box * Parent) {UIStackSetNextImpl(UIState, ui_parent_node, ParentStack, Parent)}
void UISetNextFlags(ui_box_flags Flags) {UIStackSetNextImpl(UIState, ui_flags_node, FlagsStack, Flags)}
void UISetNextFont(font_handle Font) {UIStackSetNextImpl(UIState, ui_font_node, FontStack, Font)}
void UISetNextFontSize(F32 FontSize) {UIStackSetNextImpl(UIState, ui_font_size_node, FontSizeStack, FontSize)}
void UISetNextPrefWidth(ui_size PrefWidth) {UIStackSetNextImpl(UIState, ui_pref_width_node, PrefWidthStack, PrefWidth)}
void UISetNextPrefHeight(ui_size PrefHeight) {UIStackSetNextImpl(UIState, ui_pref_height_node, PrefHeightStack, PrefHeight)}
void UISetNextTextPadding(F32 TextPadding) {UIStackSetNextImpl(UIState, ui_text_padding_node, TextPaddingStack, TextPadding)}
void UISetNextTextColor(vec4_f32 TextColor) {UIStackSetNextImpl(UIState, ui_text_color_node, TextColorStack, TextColor)}
void UISetNextFixedWidth(F32 FixedWidth) {UIStackSetNextImpl(UIState, ui_fixed_width_node, FixedWidthStack, FixedWidth)}
void UISetNextFixedHeight(F32 FixedHeight) {UIStackSetNextImpl(UIState, ui_fixed_height_node, FixedHeightStack, FixedHeight)}
void UISetNextFixedX(F32 FixedX) {UIStackSetNextImpl(UIState, ui_fixed_x_node, FixedXStack, FixedX)}
void UISetNextFixedY(F32 FixedY) {UIStackSetNextImpl(UIState, ui_fixed_y_node, FixedYStack, FixedY)}
void UISetNextBackgroundColor(vec4_f32 BackgroundColor) {UIStackSetNextImpl(UIState, ui_backgrond_color_node, BackgroundColorStack, BackgroundColor)}
void UISetNextBorderColor(vec4_f32 BorderColor) {UIStackSetNextImpl(UIState, ui_border_color_node, BorderColorStack, BorderColor)}
void UISetNextOverlayColor(vec4_f32 OverlayColor) {UIStackSetNextImpl(UIState, ui_overlay_color_node, OverlayColorStack, OverlayColor)}
void UISetNextChildLayoutAxis(axis2 ChildLayoutAxis) {UIStackSetNextImpl(UIState, ui_child_layout_axis_node, ChildLayoutAxisStack, ChildLayoutAxis)}
void UISetNextTextAlignment(ui_text_alignment_type TextAlignment) {UIStackSetNextImpl(UIState, ui_text_alignment_node, TextAlignmentStack, TextAlignment)}
void UISetNextHoverCursor(os_cursor HoverCursor) {UIStackSetNextImpl(UIState, ui_hover_cursor_node, HoverCursorStack, HoverCursor)}
void UISetNextCornerRadius0(F32 CornerRadius0) {UIStackSetNextImpl(UIState, ui_corner_radius0_node, CornerRadius0Stack, CornerRadius0)}
void UISetNextCornerRadius1(F32 CornerRadius1) {UIStackSetNextImpl(UIState, ui_corner_radius1_node, CornerRadius1Stack, CornerRadius1)}
void UISetNextCornerRadius2(F32 CornerRadius2) {UIStackSetNextImpl(UIState, ui_corner_radius2_node, CornerRadius2Stack, CornerRadius2)}
void UISetNextCornerRadius3(F32 CornerRadius3) {UIStackSetNextImpl(UIState, ui_corner_radius3_node, CornerRadius3Stack, CornerRadius3)}
void UISetNextSquish(F32 Squish) {UIStackSetNextImpl(UIState, ui_squish_node, SquishStack, Squish)}
ui_box * UITopParent() {return UIState->ParentStack.Top->Data;}
ui_box_flags UITopFlags() {return UIState->FlagsStack.Top->Data;}
font_handle UITopFont() {return UIState->FontStack.Top->Data;}
F32 UITopFontSize() {return UIState->FontSizeStack.Top->Data;}
ui_size UITopPrefWidth() {return UIState->PrefWidthStack.Top->Data;}
ui_size UITopPrefHeight() {return UIState->PrefHeightStack.Top->Data;}
F32 UITopTextPadding() {return UIState->TextPaddingStack.Top->Data;}
vec4_f32 UITopTextColor() {return UIState->TextColorStack.Top->Data;}
F32 UITopFixedWidth() {return UIState->FixedWidthStack.Top->Data;}
F32 UITopFixedHeight() {return UIState->FixedHeightStack.Top->Data;}
F32 UITopFixedX() {return UIState->FixedXStack.Top->Data;}
F32 UITopFixedY() {return UIState->FixedYStack.Top->Data;}
vec4_f32 UITopBackgroundColor() {return UIState->BackgroundColorStack.Top->Data;}
vec4_f32 UITopBorderColor() {return UIState->BorderColorStack.Top->Data;}
vec4_f32 UITopOverlayColor() {return UIState->OverlayColorStack.Top->Data;}
axis2 UITopChildLayoutAxis() {return UIState->ChildLayoutAxisStack.Top->Data;}
ui_text_alignment_type UITopTextAlignment() {return UIState->TextAlignmentStack.Top->Data;}
os_cursor UITopHoverCursor() {return UIState->HoverCursorStack.Top->Data;}
F32 UITopCornerRadius0() {return UIState->CornerRadius0Stack.Top->Data;}
F32 UITopCornerRadius1() {return UIState->CornerRadius1Stack.Top->Data;}
F32 UITopCornerRadius2() {return UIState->CornerRadius2Stack.Top->Data;}
F32 UITopCornerRadius3() {return UIState->CornerRadius3Stack.Top->Data;}
F32 UITopSquish() {return UIState->SquishStack.Top->Data;}

