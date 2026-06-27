/*  =======================================================================
    File: ps_kind_checkerboard.hlsl
    Date: June 13th 2024  4:56 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

cbuffer uniform_buffer : register(b0)
{
    float2 Resolution;
    float Turns;
    float Size;
    float4 Pos;
    float2 Offset;
    float2 Zoom;
    row_major float3x3 Transform;
    float4 Color1;
    float4 Color2;
    float4 Background;
};

struct ps_input
{
    float2 Center : CENTER;
    float2 Dim : DIM;
    float2 uv : UVSTUFF;
    float4 Position : SV_POSITION;
};

bool PointInRotatedRect(float2 Point, float2 Center, float2 Dimensions, float Angle)
{
    float2 Translated = Point - Center;

    float CosTheta = cos(Angle);
    float SinTheta = sin(Angle);
    float rx = Translated.x * CosTheta + Translated.y * SinTheta;
    float ry = Translated.y * CosTheta - Translated.x * SinTheta;

    float2 HalfDim = Dimensions / 2;
    
    return (rx >= -HalfDim.x && rx <= HalfDim.x && ry >= -HalfDim.y && ry <= HalfDim.y);
}

float4 main(ps_input Input) : SV_TARGET
{    
    if(!PointInRotatedRect(Input.Position.xy, Input.Center, Input.Dim, Turns))
        discard;    
    
    float2 Pixel = Input.uv * Resolution / Size;
    if((frac(Pixel.x) > 0.5) == (frac(Pixel.y) > 0.5))
        return float4(lerp(Color1.rgb, Background.rgb, Background.a), 1);
    else
        return float4(lerp(Color2.rgb, Background.rgb, Background.a), 1);
}
