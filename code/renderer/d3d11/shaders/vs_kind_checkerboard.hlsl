/*  =======================================================================
    File: vs_kind_checkerboard.hlsl
    Date: June 13th 2024  4:55 PM
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

ps_input main(uint VertexID : SV_VERTEXID)
{
    uint x = VertexID / 2;
    uint y = VertexID % 2;
    float2 Position = float2((float)x, (float)y) * 2.0f - 1.0f;

    float4 Dest = float4(mul(Transform, float3((Pos.xy * Zoom + Offset + 1), 1)).xy,
                         mul(Transform, float3((Pos.zw * Zoom + Offset - 1), 1)).xy);
    
    ps_input Output;
    Output.Dim = Dest.zw - Dest.xy;
    Output.Center = Dest.xy + Output.Dim / 2;
    Output.uv = Position;
    Output.Position = float4(Position, 0, 1);
    return Output;
}
