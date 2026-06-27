typedef enum gf_view_kind
{
GF_VIEW_KIND_TOOLS,
GF_VIEW_KIND_LAYERS,
GF_VIEW_KIND_CANVAS,
GF_VIEW_KIND_BRUSH_SETTINGS,
GF_VIEW_KIND_BRUSH_LISTER,
GF_VIEW_KIND_COUNT
}
gf_view_kind;

typedef enum gf_brush_settings_view_kind
{
GF_BRUSH_SETTINGS_VIEW_KIND_TIP_SHAPE,
GF_BRUSH_SETTINGS_VIEW_KIND_SHAPE_DYNAMICS,
GF_BRUSH_SETTINGS_VIEW_KIND_TRANSFER,
GF_BRUSH_SETTINGS_VIEW_KIND_COUNT
}
gf_brush_settings_view_kind;

extern string8 GFViewNames[5];

extern ui_build_view_hook GFBuildHookPtrList[5];

extern ui_start_up_view_hook GFStartUpHookPtrList[5];

extern ui_shut_down_view_hook GFShutDownHookPtrList[5];

extern string8 GFBrushSettingsViewNames[3];

UIBuildViewHook(GFToolsBuildView);
UIBuildViewHook(GFLayersBuildView);
UIBuildViewHook(GFCanvasBuildView);
UIBuildViewHook(GFBrushSettingsBuildView);
UIBuildViewHook(GFBrushListerBuildView);

UIStartUpViewHook(GFToolsStartUpView);
UIStartUpViewHook(GFLayersStartUpView);
UIStartUpViewHook(GFCanvasStartUpView);
UIStartUpViewHook(GFBrushSettingsStartUpView);
UIStartUpViewHook(GFBrushListerStartUpView);

UIShutDownViewHook(GFToolsShutDownView);
UIShutDownViewHook(GFLayersShutDownView);
UIShutDownViewHook(GFCanvasShutDownView);
UIShutDownViewHook(GFBrushSettingsShutDownView);
UIShutDownViewHook(GFBrushListerShutDownView);

#define GFDefineBuildHookPtrList \
GFToolsBuildView, \
GFLayersBuildView, \
GFCanvasBuildView, \
GFBrushSettingsBuildView, \
GFBrushListerBuildView, \

#define GFDefineStartUpHookPtrList \
GFToolsStartUpView, \
GFLayersStartUpView, \
GFCanvasStartUpView, \
GFBrushSettingsStartUpView, \
GFBrushListerStartUpView, \

#define GFDefineShutDownHookPtrList \
GFToolsShutDownView, \
GFLayersShutDownView, \
GFCanvasShutDownView, \
GFBrushSettingsShutDownView, \
GFBrushListerShutDownView, \

