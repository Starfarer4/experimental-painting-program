string8 GFViewNames[5] =
{
Str8LitComp("Tools"),
Str8LitComp("Layers"),
Str8LitComp("Canvas"),
Str8LitComp("Brush Settings"),
Str8LitComp("Brush Lister"),
};

ui_build_view_hook GFBuildHookPtrList[5] =
{
GFToolsBuildView,
GFLayersBuildView,
GFCanvasBuildView,
GFBrushSettingsBuildView,
GFBrushListerBuildView,
};

ui_start_up_view_hook GFStartUpHookPtrList[5] =
{
GFToolsStartUpView,
GFLayersStartUpView,
GFCanvasStartUpView,
GFBrushSettingsStartUpView,
GFBrushListerStartUpView,
};

ui_shut_down_view_hook GFShutDownHookPtrList[5] =
{
GFToolsShutDownView,
GFLayersShutDownView,
GFCanvasShutDownView,
GFBrushSettingsShutDownView,
GFBrushListerShutDownView,
};

string8 GFBrushSettingsViewNames[3] =
{
Str8LitComp("Brush Tip Shape"),
Str8LitComp("Shape Dynamics"),
Str8LitComp("Transfer"),
};

