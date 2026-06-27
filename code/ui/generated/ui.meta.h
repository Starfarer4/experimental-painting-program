typedef struct ui_parent_node ui_parent_node; struct ui_parent_node{ui_parent_node *Next; ui_box * Data;};
typedef struct ui_flags_node ui_flags_node; struct ui_flags_node{ui_flags_node *Next; ui_box_flags Data;};
typedef struct ui_font_node ui_font_node; struct ui_font_node{ui_font_node *Next; font_handle Data;};
typedef struct ui_font_size_node ui_font_size_node; struct ui_font_size_node{ui_font_size_node *Next; F32 Data;};
typedef struct ui_pref_width_node ui_pref_width_node; struct ui_pref_width_node{ui_pref_width_node *Next; ui_size Data;};
typedef struct ui_pref_height_node ui_pref_height_node; struct ui_pref_height_node{ui_pref_height_node *Next; ui_size Data;};
typedef struct ui_text_padding_node ui_text_padding_node; struct ui_text_padding_node{ui_text_padding_node *Next; F32 Data;};
typedef struct ui_text_color_node ui_text_color_node; struct ui_text_color_node{ui_text_color_node *Next; vec4_f32 Data;};
typedef struct ui_fixed_width_node ui_fixed_width_node; struct ui_fixed_width_node{ui_fixed_width_node *Next; F32 Data;};
typedef struct ui_fixed_height_node ui_fixed_height_node; struct ui_fixed_height_node{ui_fixed_height_node *Next; F32 Data;};
typedef struct ui_fixed_x_node ui_fixed_x_node; struct ui_fixed_x_node{ui_fixed_x_node *Next; F32 Data;};
typedef struct ui_fixed_y_node ui_fixed_y_node; struct ui_fixed_y_node{ui_fixed_y_node *Next; F32 Data;};
typedef struct ui_backgrond_color_node ui_backgrond_color_node; struct ui_backgrond_color_node{ui_backgrond_color_node *Next; vec4_f32 Data;};
typedef struct ui_border_color_node ui_border_color_node; struct ui_border_color_node{ui_border_color_node *Next; vec4_f32 Data;};
typedef struct ui_overlay_color_node ui_overlay_color_node; struct ui_overlay_color_node{ui_overlay_color_node *Next; vec4_f32 Data;};
typedef struct ui_child_layout_axis_node ui_child_layout_axis_node; struct ui_child_layout_axis_node{ui_child_layout_axis_node *Next; axis2 Data;};
typedef struct ui_text_alignment_node ui_text_alignment_node; struct ui_text_alignment_node{ui_text_alignment_node *Next; ui_text_alignment_type Data;};
typedef struct ui_hover_cursor_node ui_hover_cursor_node; struct ui_hover_cursor_node{ui_hover_cursor_node *Next; os_cursor Data;};
typedef struct ui_corner_radius0_node ui_corner_radius0_node; struct ui_corner_radius0_node{ui_corner_radius0_node *Next; F32 Data;};
typedef struct ui_corner_radius1_node ui_corner_radius1_node; struct ui_corner_radius1_node{ui_corner_radius1_node *Next; F32 Data;};
typedef struct ui_corner_radius2_node ui_corner_radius2_node; struct ui_corner_radius2_node{ui_corner_radius2_node *Next; F32 Data;};
typedef struct ui_corner_radius3_node ui_corner_radius3_node; struct ui_corner_radius3_node{ui_corner_radius3_node *Next; F32 Data;};
typedef struct ui_squish_node ui_squish_node; struct ui_squish_node{ui_squish_node *Next; F32 Data;};

typedef struct ui_parent_stack ui_parent_stack; struct ui_parent_stack{ui_parent_node *Top; B8 AutoPop;};
typedef struct ui_flags_stack ui_flags_stack; struct ui_flags_stack{ui_flags_node *Top; B8 AutoPop;};
typedef struct ui_font_stack ui_font_stack; struct ui_font_stack{ui_font_node *Top; B8 AutoPop;};
typedef struct ui_font_size_stack ui_font_size_stack; struct ui_font_size_stack{ui_font_size_node *Top; B8 AutoPop;};
typedef struct ui_pref_width_stack ui_pref_width_stack; struct ui_pref_width_stack{ui_pref_width_node *Top; B8 AutoPop;};
typedef struct ui_pref_height_stack ui_pref_height_stack; struct ui_pref_height_stack{ui_pref_height_node *Top; B8 AutoPop;};
typedef struct ui_text_padding_stack ui_text_padding_stack; struct ui_text_padding_stack{ui_text_padding_node *Top; B8 AutoPop;};
typedef struct ui_text_color_stack ui_text_color_stack; struct ui_text_color_stack{ui_text_color_node *Top; B8 AutoPop;};
typedef struct ui_fixed_width_stack ui_fixed_width_stack; struct ui_fixed_width_stack{ui_fixed_width_node *Top; B8 AutoPop;};
typedef struct ui_fixed_height_stack ui_fixed_height_stack; struct ui_fixed_height_stack{ui_fixed_height_node *Top; B8 AutoPop;};
typedef struct ui_fixed_x_stack ui_fixed_x_stack; struct ui_fixed_x_stack{ui_fixed_x_node *Top; B8 AutoPop;};
typedef struct ui_fixed_y_stack ui_fixed_y_stack; struct ui_fixed_y_stack{ui_fixed_y_node *Top; B8 AutoPop;};
typedef struct ui_backgrond_color_stack ui_backgrond_color_stack; struct ui_backgrond_color_stack{ui_backgrond_color_node *Top; B8 AutoPop;};
typedef struct ui_border_color_stack ui_border_color_stack; struct ui_border_color_stack{ui_border_color_node *Top; B8 AutoPop;};
typedef struct ui_overlay_color_stack ui_overlay_color_stack; struct ui_overlay_color_stack{ui_overlay_color_node *Top; B8 AutoPop;};
typedef struct ui_child_layout_axis_stack ui_child_layout_axis_stack; struct ui_child_layout_axis_stack{ui_child_layout_axis_node *Top; B8 AutoPop;};
typedef struct ui_text_alignment_stack ui_text_alignment_stack; struct ui_text_alignment_stack{ui_text_alignment_node *Top; B8 AutoPop;};
typedef struct ui_hover_cursor_stack ui_hover_cursor_stack; struct ui_hover_cursor_stack{ui_hover_cursor_node *Top; B8 AutoPop;};
typedef struct ui_corner_radius0_stack ui_corner_radius0_stack; struct ui_corner_radius0_stack{ui_corner_radius0_node *Top; B8 AutoPop;};
typedef struct ui_corner_radius1_stack ui_corner_radius1_stack; struct ui_corner_radius1_stack{ui_corner_radius1_node *Top; B8 AutoPop;};
typedef struct ui_corner_radius2_stack ui_corner_radius2_stack; struct ui_corner_radius2_stack{ui_corner_radius2_node *Top; B8 AutoPop;};
typedef struct ui_corner_radius3_stack ui_corner_radius3_stack; struct ui_corner_radius3_stack{ui_corner_radius3_node *Top; B8 AutoPop;};
typedef struct ui_squish_stack ui_squish_stack; struct ui_squish_stack{ui_squish_node *Top; B8 AutoPop;};

#define UIDefineStacks \
ui_parent_stack ParentStack; \
ui_flags_stack FlagsStack; \
ui_font_stack FontStack; \
ui_font_size_stack FontSizeStack; \
ui_pref_width_stack PrefWidthStack; \
ui_pref_height_stack PrefHeightStack; \
ui_text_padding_stack TextPaddingStack; \
ui_text_color_stack TextColorStack; \
ui_fixed_width_stack FixedWidthStack; \
ui_fixed_height_stack FixedHeightStack; \
ui_fixed_x_stack FixedXStack; \
ui_fixed_y_stack FixedYStack; \
ui_backgrond_color_stack BackgroundColorStack; \
ui_border_color_stack BorderColorStack; \
ui_overlay_color_stack OverlayColorStack; \
ui_child_layout_axis_stack ChildLayoutAxisStack; \
ui_text_alignment_stack TextAlignmentStack; \
ui_hover_cursor_stack HoverCursorStack; \
ui_corner_radius0_stack CornerRadius0Stack; \
ui_corner_radius1_stack CornerRadius1Stack; \
ui_corner_radius2_stack CornerRadius2Stack; \
ui_corner_radius3_stack CornerRadius3Stack; \
ui_squish_stack SquishStack; \

#define UIDefineNodeNils \
global_variable ui_parent_node NilParentNode = {0, 0}; \
global_variable ui_flags_node NilFlagsNode = {0, 0}; \
global_variable ui_font_node NilFontNode = {0, 0}; \
global_variable ui_font_size_node NilFontSizeNode = {0, 0.0f}; \
global_variable ui_pref_width_node NilPrefWidthNode = {0, {0, 0.0f, 0.0f}}; \
global_variable ui_pref_height_node NilPrefHeightNode = {0, {0, 0.0f, 0.0f}}; \
global_variable ui_text_padding_node NilTextPaddingNode = {0, 0.0f}; \
global_variable ui_text_color_node NilTextColorNode = {0, {1.0f, 1.0f, 1.0f, 1.0f}}; \
global_variable ui_fixed_width_node NilFixedWidthNode = {0, 0.0f}; \
global_variable ui_fixed_height_node NilFixedHeightNode = {0, 0.0f}; \
global_variable ui_fixed_x_node NilFixedXNode = {0, 0.0f}; \
global_variable ui_fixed_y_node NilFixedYNode = {0, 0.0f}; \
global_variable ui_backgrond_color_node NilBackgroundColorNode = {0, {0.0f, 0.0f, 0.0f, 0.0f}}; \
global_variable ui_border_color_node NilBorderColorNode = {0, {0.0f, 0.0f, 0.0f, 0.0f}}; \
global_variable ui_overlay_color_node NilOverlayColorNode = {0, {0.0f, 0.0f, 0.0f, 0.0f}}; \
global_variable ui_child_layout_axis_node NilChildLayoutAxisNode = {0, 0}; \
global_variable ui_text_alignment_node NilTextAlignmentNode = {0, 0}; \
global_variable ui_hover_cursor_node NilHoverCursorNode = {0, OS_CURSOR_POINTER}; \
global_variable ui_corner_radius0_node NilCornerRadius0Node = {0, 0.0f}; \
global_variable ui_corner_radius1_node NilCornerRadius1Node = {0, 0.0f}; \
global_variable ui_corner_radius2_node NilCornerRadius2Node = {0, 0.0f}; \
global_variable ui_corner_radius3_node NilCornerRadius3Node = {0, 0.0f}; \
global_variable ui_squish_node NilSquishNode = {0, 0.0f}; \

#define UIClearStacks \
UIState->ParentStack.Top = &NilParentNode; UIState->ParentStack.AutoPop = 0; \
UIState->FlagsStack.Top = &NilFlagsNode; UIState->FlagsStack.AutoPop = 0; \
UIState->FontStack.Top = &NilFontNode; UIState->FontStack.AutoPop = 0; \
UIState->FontSizeStack.Top = &NilFontSizeNode; UIState->FontSizeStack.AutoPop = 0; \
UIState->PrefWidthStack.Top = &NilPrefWidthNode; UIState->PrefWidthStack.AutoPop = 0; \
UIState->PrefHeightStack.Top = &NilPrefHeightNode; UIState->PrefHeightStack.AutoPop = 0; \
UIState->TextPaddingStack.Top = &NilTextPaddingNode; UIState->TextPaddingStack.AutoPop = 0; \
UIState->TextColorStack.Top = &NilTextColorNode; UIState->TextColorStack.AutoPop = 0; \
UIState->FixedWidthStack.Top = &NilFixedWidthNode; UIState->FixedWidthStack.AutoPop = 0; \
UIState->FixedHeightStack.Top = &NilFixedHeightNode; UIState->FixedHeightStack.AutoPop = 0; \
UIState->FixedXStack.Top = &NilFixedXNode; UIState->FixedXStack.AutoPop = 0; \
UIState->FixedYStack.Top = &NilFixedYNode; UIState->FixedYStack.AutoPop = 0; \
UIState->BackgroundColorStack.Top = &NilBackgroundColorNode; UIState->BackgroundColorStack.AutoPop = 0; \
UIState->BorderColorStack.Top = &NilBorderColorNode; UIState->BorderColorStack.AutoPop = 0; \
UIState->OverlayColorStack.Top = &NilOverlayColorNode; UIState->OverlayColorStack.AutoPop = 0; \
UIState->ChildLayoutAxisStack.Top = &NilChildLayoutAxisNode; UIState->ChildLayoutAxisStack.AutoPop = 0; \
UIState->TextAlignmentStack.Top = &NilTextAlignmentNode; UIState->TextAlignmentStack.AutoPop = 0; \
UIState->HoverCursorStack.Top = &NilHoverCursorNode; UIState->HoverCursorStack.AutoPop = 0; \
UIState->CornerRadius0Stack.Top = &NilCornerRadius0Node; UIState->CornerRadius0Stack.AutoPop = 0; \
UIState->CornerRadius1Stack.Top = &NilCornerRadius1Node; UIState->CornerRadius1Stack.AutoPop = 0; \
UIState->CornerRadius2Stack.Top = &NilCornerRadius2Node; UIState->CornerRadius2Stack.AutoPop = 0; \
UIState->CornerRadius3Stack.Top = &NilCornerRadius3Node; UIState->CornerRadius3Stack.AutoPop = 0; \
UIState->SquishStack.Top = &NilSquishNode; UIState->SquishStack.AutoPop = 0; \

void UIPushParent(ui_box * Parent);
void UIPushFlags(ui_box_flags Flags);
void UIPushFont(font_handle Font);
void UIPushFontSize(F32 FontSize);
void UIPushPrefWidth(ui_size PrefWidth);
void UIPushPrefHeight(ui_size PrefHeight);
void UIPushTextPadding(F32 TextPadding);
void UIPushTextColor(vec4_f32 TextColor);
void UIPushFixedWidth(F32 FixedWidth);
void UIPushFixedHeight(F32 FixedHeight);
void UIPushFixedX(F32 FixedX);
void UIPushFixedY(F32 FixedY);
void UIPushBackgroundColor(vec4_f32 BackgroundColor);
void UIPushBorderColor(vec4_f32 BorderColor);
void UIPushOverlayColor(vec4_f32 OverlayColor);
void UIPushChildLayoutAxis(axis2 ChildLayoutAxis);
void UIPushTextAlignment(ui_text_alignment_type TextAlignment);
void UIPushHoverCursor(os_cursor HoverCursor);
void UIPushCornerRadius0(F32 CornerRadius0);
void UIPushCornerRadius1(F32 CornerRadius1);
void UIPushCornerRadius2(F32 CornerRadius2);
void UIPushCornerRadius3(F32 CornerRadius3);
void UIPushSquish(F32 Squish);
void UIPopParent();
void UIPopFlags();
void UIPopFont();
void UIPopFontSize();
void UIPopPrefWidth();
void UIPopPrefHeight();
void UIPopTextPadding();
void UIPopTextColor();
void UIPopFixedWidth();
void UIPopFixedHeight();
void UIPopFixedX();
void UIPopFixedY();
void UIPopBackgroundColor();
void UIPopBorderColor();
void UIPopOverlayColor();
void UIPopChildLayoutAxis();
void UIPopTextAlignment();
void UIPopHoverCursor();
void UIPopCornerRadius0();
void UIPopCornerRadius1();
void UIPopCornerRadius2();
void UIPopCornerRadius3();
void UIPopSquish();
void UISetNextParent(ui_box * Parent);
void UISetNextFlags(ui_box_flags Flags);
void UISetNextFont(font_handle Font);
void UISetNextFontSize(F32 FontSize);
void UISetNextPrefWidth(ui_size PrefWidth);
void UISetNextPrefHeight(ui_size PrefHeight);
void UISetNextTextPadding(F32 TextPadding);
void UISetNextTextColor(vec4_f32 TextColor);
void UISetNextFixedWidth(F32 FixedWidth);
void UISetNextFixedHeight(F32 FixedHeight);
void UISetNextFixedX(F32 FixedX);
void UISetNextFixedY(F32 FixedY);
void UISetNextBackgroundColor(vec4_f32 BackgroundColor);
void UISetNextBorderColor(vec4_f32 BorderColor);
void UISetNextOverlayColor(vec4_f32 OverlayColor);
void UISetNextChildLayoutAxis(axis2 ChildLayoutAxis);
void UISetNextTextAlignment(ui_text_alignment_type TextAlignment);
void UISetNextHoverCursor(os_cursor HoverCursor);
void UISetNextCornerRadius0(F32 CornerRadius0);
void UISetNextCornerRadius1(F32 CornerRadius1);
void UISetNextCornerRadius2(F32 CornerRadius2);
void UISetNextCornerRadius3(F32 CornerRadius3);
void UISetNextSquish(F32 Squish);
ui_box * UITopParent();
ui_box_flags UITopFlags();
font_handle UITopFont();
F32 UITopFontSize();
ui_size UITopPrefWidth();
ui_size UITopPrefHeight();
F32 UITopTextPadding();
vec4_f32 UITopTextColor();
F32 UITopFixedWidth();
F32 UITopFixedHeight();
F32 UITopFixedX();
F32 UITopFixedY();
vec4_f32 UITopBackgroundColor();
vec4_f32 UITopBorderColor();
vec4_f32 UITopOverlayColor();
axis2 UITopChildLayoutAxis();
ui_text_alignment_type UITopTextAlignment();
os_cursor UITopHoverCursor();
F32 UITopCornerRadius0();
F32 UITopCornerRadius1();
F32 UITopCornerRadius2();
F32 UITopCornerRadius3();
F32 UITopSquish();

#define UIParent(Parent) DeferLoop(UIPushParent(Parent), UIPopParent())
#define UIFlags(Flags) DeferLoop(UIPushFlags(Flags), UIPopFlags())
#define UIFont(Font) DeferLoop(UIPushFont(Font), UIPopFont())
#define UIFontSize(FontSize) DeferLoop(UIPushFontSize(FontSize), UIPopFontSize())
#define UIPrefWidth(PrefWidth) DeferLoop(UIPushPrefWidth(PrefWidth), UIPopPrefWidth())
#define UIPrefHeight(PrefHeight) DeferLoop(UIPushPrefHeight(PrefHeight), UIPopPrefHeight())
#define UITextPadding(TextPadding) DeferLoop(UIPushTextPadding(TextPadding), UIPopTextPadding())
#define UITextColor(TextColor) DeferLoop(UIPushTextColor(TextColor), UIPopTextColor())
#define UIFixedWidth(FixedWidth) DeferLoop(UIPushFixedWidth(FixedWidth), UIPopFixedWidth())
#define UIFixedHeight(FixedHeight) DeferLoop(UIPushFixedHeight(FixedHeight), UIPopFixedHeight())
#define UIFixedX(FixedX) DeferLoop(UIPushFixedX(FixedX), UIPopFixedX())
#define UIFixedY(FixedY) DeferLoop(UIPushFixedY(FixedY), UIPopFixedY())
#define UIBackgroundColor(BackgroundColor) DeferLoop(UIPushBackgroundColor(BackgroundColor), UIPopBackgroundColor())
#define UIBorderColor(BorderColor) DeferLoop(UIPushBorderColor(BorderColor), UIPopBorderColor())
#define UIOverlayColor(OverlayColor) DeferLoop(UIPushOverlayColor(OverlayColor), UIPopOverlayColor())
#define UIChildLayoutAxis(ChildLayoutAxis) DeferLoop(UIPushChildLayoutAxis(ChildLayoutAxis), UIPopChildLayoutAxis())
#define UITextAlignment(TextAlignment) DeferLoop(UIPushTextAlignment(TextAlignment), UIPopTextAlignment())
#define UIHoverCursor(HoverCursor) DeferLoop(UIPushHoverCursor(HoverCursor), UIPopHoverCursor())
#define UICornerRadius0(CornerRadius0) DeferLoop(UIPushCornerRadius0(CornerRadius0), UIPopCornerRadius0())
#define UICornerRadius1(CornerRadius1) DeferLoop(UIPushCornerRadius1(CornerRadius1), UIPopCornerRadius1())
#define UICornerRadius2(CornerRadius2) DeferLoop(UIPushCornerRadius2(CornerRadius2), UIPopCornerRadius2())
#define UICornerRadius3(CornerRadius3) DeferLoop(UIPushCornerRadius3(CornerRadius3), UIPopCornerRadius3())
#define UISquish(Squish) DeferLoop(UIPushSquish(Squish), UIPopSquish())

#define UIAutoPopStacks \
if(UIState->ParentStack.AutoPop) {UIPopParent(); UIState->ParentStack.AutoPop = 0;} \
if(UIState->FlagsStack.AutoPop) {UIPopFlags(); UIState->FlagsStack.AutoPop = 0;} \
if(UIState->FontStack.AutoPop) {UIPopFont(); UIState->FontStack.AutoPop = 0;} \
if(UIState->FontSizeStack.AutoPop) {UIPopFontSize(); UIState->FontSizeStack.AutoPop = 0;} \
if(UIState->PrefWidthStack.AutoPop) {UIPopPrefWidth(); UIState->PrefWidthStack.AutoPop = 0;} \
if(UIState->PrefHeightStack.AutoPop) {UIPopPrefHeight(); UIState->PrefHeightStack.AutoPop = 0;} \
if(UIState->TextPaddingStack.AutoPop) {UIPopTextPadding(); UIState->TextPaddingStack.AutoPop = 0;} \
if(UIState->TextColorStack.AutoPop) {UIPopTextColor(); UIState->TextColorStack.AutoPop = 0;} \
if(UIState->FixedWidthStack.AutoPop) {UIPopFixedWidth(); UIState->FixedWidthStack.AutoPop = 0;} \
if(UIState->FixedHeightStack.AutoPop) {UIPopFixedHeight(); UIState->FixedHeightStack.AutoPop = 0;} \
if(UIState->FixedXStack.AutoPop) {UIPopFixedX(); UIState->FixedXStack.AutoPop = 0;} \
if(UIState->FixedYStack.AutoPop) {UIPopFixedY(); UIState->FixedYStack.AutoPop = 0;} \
if(UIState->BackgroundColorStack.AutoPop) {UIPopBackgroundColor(); UIState->BackgroundColorStack.AutoPop = 0;} \
if(UIState->BorderColorStack.AutoPop) {UIPopBorderColor(); UIState->BorderColorStack.AutoPop = 0;} \
if(UIState->OverlayColorStack.AutoPop) {UIPopOverlayColor(); UIState->OverlayColorStack.AutoPop = 0;} \
if(UIState->ChildLayoutAxisStack.AutoPop) {UIPopChildLayoutAxis(); UIState->ChildLayoutAxisStack.AutoPop = 0;} \
if(UIState->TextAlignmentStack.AutoPop) {UIPopTextAlignment(); UIState->TextAlignmentStack.AutoPop = 0;} \
if(UIState->HoverCursorStack.AutoPop) {UIPopHoverCursor(); UIState->HoverCursorStack.AutoPop = 0;} \
if(UIState->CornerRadius0Stack.AutoPop) {UIPopCornerRadius0(); UIState->CornerRadius0Stack.AutoPop = 0;} \
if(UIState->CornerRadius1Stack.AutoPop) {UIPopCornerRadius1(); UIState->CornerRadius1Stack.AutoPop = 0;} \
if(UIState->CornerRadius2Stack.AutoPop) {UIPopCornerRadius2(); UIState->CornerRadius2Stack.AutoPop = 0;} \
if(UIState->CornerRadius3Stack.AutoPop) {UIPopCornerRadius3(); UIState->CornerRadius3Stack.AutoPop = 0;} \
if(UIState->SquishStack.AutoPop) {UIPopSquish(); UIState->SquishStack.AutoPop = 0;} \

