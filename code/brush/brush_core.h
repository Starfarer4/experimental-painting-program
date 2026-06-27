#if !defined(BRUSH_CORE_H)
/*  =======================================================================
    File: brush_core.h
    Date: June 17th 2024  2:36 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define BRUSH_CORE_H

string8 BRControlNames[] =
{
    Str8LitComp("Off"),
    Str8LitComp("Pen Pressure")
};

typedef enum br_control_type
{
    BR_CONTROL_TYPE_OFF,
    BR_CONTROL_TYPE_PEN_PRESSURE,
    BR_CONTROL_TYPE_COUNT
}br_control_type;

typedef struct br_brush_shape_dynamics br_brush_shape_dynamics;
struct br_brush_shape_dynamics
{
    B8 Enabled;

    F32 SizeJitter;
    br_control_type SizeControl;
    F32 MinimumDiameter;
};

typedef struct br_brush_transfer br_brush_transfer;
struct br_brush_transfer
{
    B8 Enabled;

    F32 OpacityJitter;
    br_control_type OpacityControl;
    F32 OpacityControlMinimum;
    
    F32 FlowJitter;
    br_control_type FlowControl;
    F32 FlowControlMinimum;
};

typedef struct br_computed_tip br_computed_tip;
struct br_computed_tip
{
    F32 Hardness;
    F32 Angle;
    F32 Roundness;
    B8 FlipX;
    B8 FlipY;
};

typedef struct br_bristle_tip br_bristle_tip;
struct br_bristle_tip
{
    F32 Asdf;
};

typedef enum br_tip_kind
{
    BR_TIP_KIND_COMPUTED,
    BR_TIP_KIND_BRISTLE,
}br_tip_kind;

typedef struct br_brush br_brush;
struct br_brush
{
    // Static state
    string8 Name;
    
    F32 SizePx;
    F32 SpacingPct;    

    br_tip_kind TipKind;
    union
    {
        br_computed_tip ComputedTip;
        br_bristle_tip BristleTip;
    };

    br_brush_shape_dynamics ShapeDynamics;
    br_brush_transfer Transfer;

    // Dynamic state
    vec2_f32 LastBrushPosition;
    F32 LastBrushPressure;
    F32 LastBrushSize;
    im_image *CurrentImage;
};

typedef struct br_brush_node br_brush_node;
struct br_brush_node
{
    br_brush_node *Next;
    br_brush_node *Prev;
    br_brush Brush;
};

typedef struct br_brush_list br_brush_list;
struct br_brush_list
{
    br_brush_node *First;
    br_brush_node *Last;
    U64 Count;
};

void BRResetDraw(br_brush *Brush, im_image *Image);
void BRAddBrushStroke(br_brush *Brush, im_image *Image, vec2_f32 Location, F32 CanvasTurns, F32 Pressure);
void BRStartState();

#endif
