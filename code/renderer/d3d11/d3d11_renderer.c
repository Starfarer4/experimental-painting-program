/*  =======================================================================
    File: d3d11_renderer.c
    Date: March 6th 2024  1:02 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

#include "renderer/d3d11/shaders/generated/vs_kind_rect.h"
#include "renderer/d3d11/shaders/generated/vs_kind_checkerboard.h"

#include "renderer/d3d11/shaders/generated/ps_kind_rect.h"
#include "renderer/d3d11/shaders/generated/ps_kind_checkerboard.h"

global_variable D3D11_INPUT_ELEMENT_DESC DD11RectInputElementDesc[] =
{
    {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"TEXCOORDS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"COLORS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"COLORS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"COLORS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"COLORS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"CORNERRAD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"BORDERTHICKNESS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"SOFTNESS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    {"WHITEOVERRIDE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
};

typedef struct d3d11_tex2d d3d11_tex2d;
struct d3d11_tex2d
{
    d3d11_tex2d *Next;
    d3d11_tex2d *Prev;
    
    ID3D11Texture2D *Texture;
    ID3D11ShaderResourceView *View;

    rnd_tex2d_kind Kind;
    vec2_i32 Size;
    rnd_tex2d_format Format;
    U64 PixelBytes;
};

typedef struct d3d11_window d3d11_window;
struct d3d11_window
{
    IDXGISwapChain1 *Swapchain;
    ID3D11Texture2D *Framebuffer;
    ID3D11RenderTargetView *FramebufferRTV;

    vec2_i32 LastResolution;
    
    d3d11_window *Next;
    d3d11_window *Prev;
};

typedef struct d3d11_renderer_state d3d11_renderer_state;
struct d3d11_renderer_state
{
    ID3D11Device *BaseDevice;
    ID3D11DeviceContext *BaseDeviceContext;

    ID3D11Device1 *Device;
    ID3D11DeviceContext1 *DeviceContext;
    
    IDXGIDevice1 *DXGIDevice;
    IDXGIAdapter *DXGIAdapter;
    IDXGIFactory2 *DXGIFactory;

    ID3D11RasterizerState1 *MainRasterizer;
    ID3D11BlendState *MainBlendState;
    ID3D11SamplerState *Samplers[RND_TEX2D_SAMPLER_KIND_COUNT];
    ID3D11DepthStencilState *NoOpDepthStencil;
    ID3D11VertexShader *VShaders[RND_VS_KIND_COUNT];
    ID3D11InputLayout *InputLayouts[RND_VS_KIND_COUNT];
    ID3D11PixelShader *PShaders[RND_PS_KIND_COUNT];
    ID3D11Buffer *UniformBuffers[RND_UNIFORM_KIND_COUNT];

    U64 UIInstanceBufferSize;
    ID3D11Buffer *UIInstanceBuffer;
    
    d3d11_window *FirstFreeWindow;
    d3d11_window *LastFreeWindow;
    
    d3d11_window *FirstActiveWindow;
    d3d11_window *LastActiveWindow;

    d3d11_tex2d *FirstFreeTexture;
    d3d11_tex2d *LastFreeTexture;
    
    rnd_handle BackupTexture;
    
    arena *Arena;
};

global_variable d3d11_renderer_state *DD11RendererState = 0;

void RndStartUp()
{
    arena *Arena = ArenaAlloc(Megabytes(4));
    DD11RendererState = PushStruct(Arena, d3d11_renderer_state);
    DD11RendererState->Arena = Arena;

    HRESULT Error = 0;

    UINT CreationFlags = 0;
#if PX_DEBUG
    CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0};
    D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
    Error = D3D11CreateDevice(0, DriverType, 0, CreationFlags, FeatureLevels, ArrayCount(FeatureLevels), D3D11_SDK_VERSION,
                              &DD11RendererState->BaseDevice, 0, &DD11RendererState->BaseDeviceContext);
    if(FAILED(Error) && DriverType == D3D_DRIVER_TYPE_HARDWARE)
    {
        Error = D3D11CreateDevice(0, D3D_DRIVER_TYPE_WARP, 0, CreationFlags, FeatureLevels, ArrayCount(FeatureLevels), D3D11_SDK_VERSION,
                                  &DD11RendererState->BaseDevice, 0, &DD11RendererState->BaseDeviceContext);
    }

    if(FAILED(Error))
    {
        FatalError("Failed to create the D3D11 device!\n");
    }

#if PX_DEBUG
    ID3D11InfoQueue *Info = 0;
    ID3D11Device_QueryInterface(DD11RendererState->BaseDevice, &IID_ID3D11InfoQueue, (void **)&Info);
    if(SUCCEEDED(Error))
    {
        Error = ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        Error = ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        ID3D11InfoQueue_Release(Info);
    }
#endif

    Error = ID3D11Device_QueryInterface(DD11RendererState->BaseDevice, &IID_ID3D11Device1, (void **)&DD11RendererState->Device);
    Error = ID3D11DeviceContext_QueryInterface(DD11RendererState->BaseDeviceContext, &IID_ID3D11DeviceContext1,
                                               (void **)&DD11RendererState->DeviceContext);

    Error = ID3D11Device1_QueryInterface(DD11RendererState->Device, &IID_IDXGIDevice1, (void **)&DD11RendererState->DXGIDevice);
    Error = IDXGIDevice_GetAdapter(DD11RendererState->DXGIDevice, &DD11RendererState->DXGIAdapter);
    Error = IDXGIAdapter_GetParent(DD11RendererState->DXGIAdapter, &IID_IDXGIFactory2, (void **)&DD11RendererState->DXGIFactory);

    {
        D3D11_RASTERIZER_DESC1 Desc = {0};
        Desc.FillMode = D3D11_FILL_SOLID;
        Desc.CullMode = D3D11_CULL_BACK;
        Desc.ScissorEnable = 1;
        Error = ID3D11Device1_CreateRasterizerState1(DD11RendererState->Device, &Desc, &DD11RendererState->MainRasterizer);
    }

    {
        D3D11_BLEND_DESC Desc = {0};
        Desc.RenderTarget[0].BlendEnable = TRUE;
        Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        Desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        Error = ID3D11Device1_CreateBlendState(DD11RendererState->Device, &Desc, &DD11RendererState->MainBlendState);
    }

    // Create texture samplers
    {
        D3D11_SAMPLER_DESC Desc = {0};
        Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        Error = ID3D11Device1_CreateSamplerState(DD11RendererState->Device, &Desc,
                                                 &DD11RendererState->Samplers[RND_TEX2D_SAMPLER_KIND_NEAREST]);
    }
    {
        D3D11_SAMPLER_DESC Desc = {0};
        Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        Error = ID3D11Device1_CreateSamplerState(DD11RendererState->Device, &Desc,
                                                 &DD11RendererState->Samplers[RND_TEX2D_SAMPLER_KIND_LINEAR]);
    }

    // Create depth stencil states
    {
        D3D11_DEPTH_STENCIL_DESC Desc = {0};
        Desc.DepthEnable = FALSE;
        Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        Desc.DepthFunc = D3D11_COMPARISON_LESS;
        Desc.StencilEnable = FALSE;
        Error = ID3D11Device1_CreateDepthStencilState(DD11RendererState->Device, &Desc, &DD11RendererState->NoOpDepthStencil);
    }

    // Create vertex shaders
    for(U32 Kind = 0; Kind < RND_VS_KIND_COUNT; ++Kind)
    {
        ID3D11VertexShader *VShader = 0;

        D3D11_INPUT_ELEMENT_DESC *LayoutDesc = 0;
        U64 LayoutDescCount = 0;
        U8 *Buffer = 0;
        U64 BufferSize = 0;
        switch(Kind)
        {
            case RND_VS_KIND_RECT:
            {
                Buffer = (U8 *)DD11VShaderBufferKindRect;
                BufferSize = sizeof(DD11VShaderBufferKindRect);
                LayoutDesc = DD11RectInputElementDesc;
                LayoutDescCount = ArrayCount(DD11RectInputElementDesc);
            }break;

            case RND_VS_KIND_CHECKERBOARD:
            {
                Buffer = (U8 *)DD11VShaderBufferKindCheckerboard;
                BufferSize = sizeof(DD11VShaderBufferKindCheckerboard);
            }break;
        }

        Error = ID3D11Device1_CreateVertexShader(DD11RendererState->Device, Buffer, BufferSize, 0, &VShader);

        if(LayoutDescCount)
        {
            ID3D11InputLayout *InputLayout = 0;
            Error = ID3D11Device1_CreateInputLayout(DD11RendererState->Device, LayoutDesc, (UINT)LayoutDescCount, Buffer, BufferSize,
                                                    &InputLayout);
 
            DD11RendererState->InputLayouts[Kind] = InputLayout;
        }
        
        DD11RendererState->VShaders[Kind] = VShader;
    }

    // Create pixel shaders
    for(U32 Kind = 0; Kind < RND_PS_KIND_COUNT; ++Kind)
    {
        ID3D11PixelShader *PShader = 0;

        U8 *Buffer = 0;
        U64 BufferSize = 0;
        switch(Kind)
        {
            case RND_PS_KIND_RECT:
            {
                Buffer = (U8 *)DD11PShaderBufferKindRect;
                BufferSize = sizeof(DD11PShaderBufferKindRect);
            }break;

            case RND_PS_KIND_CHECKERBOARD:
            {
                Buffer = (U8 *)DD11PShaderBufferKindCheckerboard;
                BufferSize = sizeof(DD11PShaderBufferKindCheckerboard);
            }break; 
        }

        Error = ID3D11Device_CreatePixelShader(DD11RendererState->Device, Buffer, BufferSize, 0, &PShader);

        DD11RendererState->PShaders[Kind] = PShader;
    }

    // Create uniform buffers
    for(U32 Kind = 0; Kind < RND_UNIFORM_KIND_COUNT; ++Kind)
    {
        ID3D11Buffer *Buffer = 0;

        U64 ByteWidth = 0;
        switch(Kind)
        {
            case RND_UNIFORM_KIND_RECT:
            {
                ByteWidth = sizeof(rnd_uniform_rect);
            }break;

            case RND_UNIFORM_KIND_CHECKERBOARD:
            {
                ByteWidth = sizeof(rnd_uniform_checkerboard);
            }break;
        }

        ByteWidth += 15;
        ByteWidth -= ByteWidth % 16;
        
        D3D11_BUFFER_DESC Desc = {0};
        Desc.ByteWidth = (UINT)ByteWidth;
        Desc.Usage = D3D11_USAGE_DYNAMIC;
        Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ID3D11Device_CreateBuffer(DD11RendererState->Device, &Desc, 0, &Buffer);
        DD11RendererState->UniformBuffers[Kind] = Buffer;
    }

    // Create backup texture
    {
        U8 BackupTextureData[] =
        {
            255,   0,   0, 255,
              0, 255,   0, 255,
              0,   0, 255, 255,
            255, 255, 255, 255,
        };
        DD11RendererState->BackupTexture = RndTex2DAlloc(RND_TEX2D_KIND_LONG_LIVED, Vec2I32(2, 2), RND_TEX2D_FORMAT_RGBA8,
                                                         BackupTextureData);
    }
}

void RndWindowBeginFrame(os_handle OSWindow, rnd_handle RndWindow)
{
    d3d11_window *Window = (d3d11_window *)RndWindow;

    ID3D11DeviceContext1 *DeviceContext = DD11RendererState->DeviceContext;

    rng2_f32 ClientRect = OSClientRectFromWindow(OSWindow);
    vec2_i32 Resolution = {(I32)(ClientRect.Right - ClientRect.Left), (I32)(ClientRect.Bottom - ClientRect.Top)};

    if((Window->LastResolution.X != Resolution.X || Window->LastResolution.Y != Resolution.Y) && (Resolution.X && Resolution.Y))
    {
        Window->LastResolution = Resolution;

        ID3D11RenderTargetView_Release(Window->FramebufferRTV);
        ID3D11Texture2D_Release(Window->Framebuffer);
        IDXGISwapChain1_ResizeBuffers(Window->Swapchain, 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        IDXGISwapChain1_GetBuffer(Window->Swapchain, 0, &IID_ID3D11Texture2D, (void **)&Window->Framebuffer);
        ID3D11Device1_CreateRenderTargetView(DD11RendererState->Device, (ID3D11Resource *)Window->Framebuffer, 0, &Window->FramebufferRTV);
    }
    
    F32 ClearColor[4] = {0.1f, 0.1f, 0.0f, 0.0f};
    ID3D11DeviceContext1_ClearRenderTargetView(DeviceContext, Window->FramebufferRTV, ClearColor);
}

void RndWindowEndFrame(os_handle OSWindow, rnd_handle RndWindow)
{
    d3d11_window *Window = (d3d11_window *)RndWindow;

    ID3D11DeviceContext1 *DeviceContext = DD11RendererState->DeviceContext;
    
    IDXGISwapChain1_Present(Window->Swapchain, 1, 0);
    ID3D11DeviceContext1_ClearState(DeviceContext);
}

void RndWindowSubmit(os_handle OSWindow, rnd_handle RndWindow, rnd_pass_list Passes)
{
    win32_window *Win32Window = (win32_window *)OSWindow;
    
    d3d11_window *Window = (d3d11_window *)RndWindow;
    ID3D11DeviceContext1 *DeviceContext = DD11RendererState->DeviceContext;

    for(rnd_pass *Pass = Passes.First; Pass; Pass = Pass->Next)
    {
        switch(Pass->Kind)
        {
            case RND_PASS_KIND_UI:
            {
                rnd_pass_params_ui *UIParams = Pass->UIParams;
                rnd_batch_group_2d_list *Rects = &UIParams->Rects;

                D3D11_VIEWPORT Viewport = {0.0f, 0.0f, (F32)Window->LastResolution.X, (F32)Window->LastResolution.Y, 0.0f, 1.0f};
                ID3D11DeviceContext1_RSSetViewports(DeviceContext, 1, &Viewport);
                ID3D11DeviceContext1_RSSetState(DeviceContext, (ID3D11RasterizerState *)DD11RendererState->MainRasterizer);
                
                for(rnd_batch_group_2d *BatchGroup = Rects->First; BatchGroup; BatchGroup = BatchGroup->Next)
                {
                    rnd_batch_group_2d_params *BatchParams = &BatchGroup->Params;
    
                    //ID3D11SamplerState *Sampler = DD11RendererState->Samplers[RND_TEX2D_SAMPLER_KIND_NEAREST];
                    ID3D11SamplerState *Sampler = DD11RendererState->Samplers[BatchParams->TexSamplerKind];
                    ID3D11VertexShader *VShader = DD11RendererState->VShaders[RND_VS_KIND_RECT];
                    ID3D11InputLayout *InputLayout = DD11RendererState->InputLayouts[RND_VS_KIND_RECT];
                    ID3D11PixelShader *PShader = DD11RendererState->PShaders[RND_PS_KIND_RECT];
                    ID3D11Buffer *UniformBuffer = DD11RendererState->UniformBuffers[RND_UNIFORM_KIND_RECT];

                    // Resize instance buffer if it is too small
                    if(DD11RendererState->UIInstanceBuffer == 0 || DD11RendererState->UIInstanceBufferSize < BatchGroup->Batches.ByteCount)
                    {
                        const U64 BufferPageSize = Kilobytes(64);
                        U64 AllocSize = (BufferPageSize + BatchGroup->Batches.ByteCount - 1);
                        AllocSize -= AllocSize % BufferPageSize;
                        if(DD11RendererState->UIInstanceBuffer) ID3D11Buffer_Release(DD11RendererState->UIInstanceBuffer);
                        DD11RendererState->UIInstanceBufferSize = AllocSize;

                        D3D11_BUFFER_DESC Desc = {0};
                        Desc.Usage = D3D11_USAGE_DYNAMIC;
                        Desc.ByteWidth = (UINT)DD11RendererState->UIInstanceBufferSize;
                        Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                        Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                        Desc.MiscFlags = 0;

                        HRESULT Error = ID3D11Device1_CreateBuffer(DD11RendererState->Device, &Desc, 0,
                                                                   &DD11RendererState->UIInstanceBuffer);
                    }

                    // Upload instance data to buffer
                    {
                        D3D11_MAPPED_SUBRESOURCE Subresource = {0};
                        ID3D11DeviceContext1_Map(DeviceContext, (ID3D11Resource *)DD11RendererState->UIInstanceBuffer,
                                                 0, D3D11_MAP_WRITE_DISCARD, 0, &Subresource);
                        U8 *Destination = (U8 *)Subresource.pData;
                        U64 Offset = 0;
                        for(rnd_batch *Batch = BatchGroup->Batches.First; Batch; Batch = Batch->Next)
                        {
                            MemoryCopy(Destination + Offset, Batch->Bytes, Batch->ByteCount);
                            Offset += Batch->ByteCount;
                        }
                        ID3D11DeviceContext1_Unmap(DeviceContext, (ID3D11Resource *)DD11RendererState->UIInstanceBuffer, 0);
                    }
                    // Grab the texture

                    rnd_handle TextureHandle = BatchParams->Texture;
                    if(TextureHandle == 0)
                    {
                        TextureHandle = DD11RendererState->BackupTexture;
                    }
                    d3d11_tex2d *Texture = (d3d11_tex2d *)TextureHandle;

                    F32 TextureSampleChannelMap[4][4] =
                    {
                        {1.0f, 0.0f, 0.0f, 0.0f},
                        {0.0f, 1.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 1.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f, 1.0f},
                    };
                    switch(Texture->Format)
                    {
                        case RND_TEX2D_FORMAT_A8:
                        {
                            MemoryZero(TextureSampleChannelMap, sizeof(TextureSampleChannelMap));
                            TextureSampleChannelMap[3][0] = 255.0f;
                            TextureSampleChannelMap[3][1] = 255.0f;
                            TextureSampleChannelMap[3][2] = 255.0f;
                            TextureSampleChannelMap[3][3] = 1.0f;
                        }break;
                    }
                    
                    // Upload uniform data to buffer
                    {
                        rnd_uniform_rect Uniform = {0};
                        Uniform.Resolution = Vec2F32((F32)Window->LastResolution.X, (F32)Window->LastResolution.Y);
                        Uniform.TexSize = Vec2F32((F32)Texture->Size.X, (F32)Texture->Size.Y);
                        MemoryCopy(Uniform.TextureSampleChannelMap, TextureSampleChannelMap, sizeof(TextureSampleChannelMap));

                        Uniform.Transform[0] = Vec4F32(BatchParams->Transform.V[0][0], BatchParams->Transform.V[1][0],
                                                       BatchParams->Transform.V[2][0], 0.0f);
                        Uniform.Transform[1] = Vec4F32(BatchParams->Transform.V[0][1], BatchParams->Transform.V[1][1],
                                                       BatchParams->Transform.V[2][1], 0.0f);
                        Uniform.Transform[2] = Vec4F32(BatchParams->Transform.V[0][2], BatchParams->Transform.V[1][2],
                                                       BatchParams->Transform.V[2][2], 0.0f);
                        vec2_f32 Transform2x2Col0 = Vec2F32(Uniform.Transform[0].X, Uniform.Transform[1].X);
                        vec2_f32 Transform2x2Col1 = Vec2F32(Uniform.Transform[0].Y, Uniform.Transform[1].Y);
                        Uniform.TransformScale.X = Length2F32(Transform2x2Col0);
                        Uniform.TransformScale.Y = Length2F32(Transform2x2Col1);

                        D3D11_MAPPED_SUBRESOURCE Subresource = {0};
                        ID3D11DeviceContext1_Map(DeviceContext, (ID3D11Resource *)UniformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
                                                 &Subresource);
                        MemoryCopy(Subresource.pData, &Uniform, sizeof(rnd_uniform_rect));
                        ID3D11DeviceContext1_Unmap(DeviceContext, (ID3D11Resource *)UniformBuffer, 0);
                    }
                    
                    ID3D11DeviceContext1_OMSetRenderTargets(DeviceContext, 1, &Window->FramebufferRTV, 0);
                    ID3D11DeviceContext1_OMSetDepthStencilState(DeviceContext, DD11RendererState->NoOpDepthStencil, 0);
                    ID3D11DeviceContext1_OMSetBlendState(DeviceContext, DD11RendererState->MainBlendState, 0, 0xffffffff);

                    UINT Stride = (UINT)BatchGroup->Batches.BytesPerInst;
                    UINT Offset = 0;
                    ID3D11DeviceContext1_IASetPrimitiveTopology(DeviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                    ID3D11DeviceContext1_IASetInputLayout(DeviceContext, InputLayout);
                    ID3D11DeviceContext1_IASetVertexBuffers(DeviceContext, 0, 1, &DD11RendererState->UIInstanceBuffer, &Stride, &Offset);

                    ID3D11DeviceContext1_VSSetShader(DeviceContext, VShader, 0, 0);
                    ID3D11DeviceContext1_VSSetConstantBuffers(DeviceContext, 0, 1, &UniformBuffer);
                    ID3D11DeviceContext1_PSSetShader(DeviceContext, PShader, 0, 0);
                    ID3D11DeviceContext1_PSSetConstantBuffers(DeviceContext, 0, 1, &UniformBuffer);
                    ID3D11DeviceContext1_PSSetShaderResources(DeviceContext, 0, 1, &Texture->View);
                    ID3D11DeviceContext1_PSSetSamplers(DeviceContext, 0, 1, &Sampler);

                    // Set the scissor rect
                    {
                        D3D11_RECT Rect = {0};
                        rng2_f32 Clip = BatchParams->Clip;
                        if(Clip.Left == 0 && Clip.Right == 0 && Clip.Top == 0 && Clip.Bottom == 0)
                        {
                            Rect.left = 0;
                            Rect.right = (LONG)Window->LastResolution.X;
                            Rect.top = 0;
                            Rect.bottom = (LONG)Window->LastResolution.Y;
                        }
                        else if(Clip.Left > Clip.Right || Clip.Top > Clip.Bottom)
                        {
                            Rect.left = 0;
                            Rect.right = 0;
                            Rect.top = 0;
                            Rect.bottom = 0;
                        }
                        else
                        {
                            Rect.left = (LONG)Clip.Left;
                            Rect.right = (LONG)Clip.Right;
                            Rect.top = (LONG)Clip.Top;
                            Rect.bottom = (LONG)Clip.Bottom;
                        }
                        ID3D11DeviceContext1_RSSetScissorRects(DeviceContext, 1, &Rect);
                    }

                    UINT InstanceCount = (UINT)(BatchGroup->Batches.ByteCount / BatchGroup->Batches.BytesPerInst);
                    ID3D11DeviceContext1_DrawInstanced(DeviceContext, 4, InstanceCount, 0, 0);
                }
            }break;

            case RND_PASS_KIND_CHECKERBOARD:
            {
                rnd_pass_params_checkerboard *CheckerboardParams = Pass->CheckerboardParams;
                
                ID3D11VertexShader *VShader = DD11RendererState->VShaders[RND_VS_KIND_CHECKERBOARD];
                ID3D11PixelShader *PShader = DD11RendererState->PShaders[RND_PS_KIND_CHECKERBOARD];
                ID3D11Buffer *UniformBuffer = DD11RendererState->UniformBuffers[RND_UNIFORM_KIND_CHECKERBOARD];

                // Setup output merger
                ID3D11DeviceContext1_OMSetRenderTargets(DeviceContext, 1, &Window->FramebufferRTV, 0);
                ID3D11DeviceContext1_OMSetDepthStencilState(DeviceContext, DD11RendererState->NoOpDepthStencil, 0);
                ID3D11DeviceContext1_OMSetBlendState(DeviceContext, DD11RendererState->MainBlendState, 0, 0xffffffff);

                // Setup viewport
                D3D11_VIEWPORT Viewport = {0.0f, 0.0f, (F32)Window->LastResolution.X, (F32)Window->LastResolution.Y, 0.0f, 1.0f};
                ID3D11DeviceContext1_RSSetViewports(DeviceContext, 1, &Viewport);
                ID3D11DeviceContext1_RSSetState(DeviceContext, (ID3D11RasterizerState *)DD11RendererState->MainRasterizer);

                // Setup input assembly
                ID3D11DeviceContext1_IASetPrimitiveTopology(DeviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                ID3D11DeviceContext1_IASetInputLayout(DeviceContext, 0);

                // Upload uniform data to buffer
                {
                    rnd_uniform_checkerboard Uniform = {0};
                    Uniform.Resolution = Vec2F32((F32)Window->LastResolution.X, (F32)Window->LastResolution.Y);
                    Uniform.Position = CheckerboardParams->Position;
                    Uniform.Offset = Vec2F32(Trunc(CheckerboardParams->Offset.X), Trunc(CheckerboardParams->Offset.Y));
                    Uniform.Zoom = CheckerboardParams->Zoom;
                    Uniform.Turns = RadiansFromTurnsF32(-CheckerboardParams->Turns);
                    Uniform.Size = CheckerboardParams->Size;
                    Uniform.Transform[0] = Vec4F32(CheckerboardParams->Transform.V[0][0], CheckerboardParams->Transform.V[1][0],
                                                   CheckerboardParams->Transform.V[2][0], 0.0f);
                    Uniform.Transform[1] = Vec4F32(CheckerboardParams->Transform.V[0][1], CheckerboardParams->Transform.V[1][1],
                                                   CheckerboardParams->Transform.V[2][1], 0.0f);
                    Uniform.Transform[2] = Vec4F32(CheckerboardParams->Transform.V[0][2], CheckerboardParams->Transform.V[1][2],
                                                   CheckerboardParams->Transform.V[2][2], 0.0f);
                    Uniform.Color1 = CheckerboardParams->Color1;
                    Uniform.Color2 = CheckerboardParams->Color2;
                    Uniform.Background = CheckerboardParams->Background;
                    
                    D3D11_MAPPED_SUBRESOURCE Subresource = {0};
                    ID3D11DeviceContext1_Map(DeviceContext, (ID3D11Resource *)UniformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
                                             &Subresource);
                    MemoryCopy(Subresource.pData, &Uniform, sizeof(rnd_uniform_checkerboard));
                    ID3D11DeviceContext1_Unmap(DeviceContext, (ID3D11Resource *)UniformBuffer, 0);
                }
                
                // Setup shaders
                ID3D11DeviceContext1_VSSetShader(DeviceContext, VShader, 0, 0);
                ID3D11DeviceContext1_VSSetConstantBuffers(DeviceContext, 0, 1, &UniformBuffer);
                ID3D11DeviceContext1_PSSetShader(DeviceContext, PShader, 0, 0);
                ID3D11DeviceContext1_PSSetConstantBuffers(DeviceContext, 0, 1, &UniformBuffer);

                // Setup the scissor rect
                {
                    D3D11_RECT Rect = {0};
                    rng2_f32 Clip = CheckerboardParams->Clip;
                    if(Clip.Left == 0 && Clip.Right == 0 && Clip.Top == 0 && Clip.Bottom == 0)
                    {
                        Rect.left = 0;
                        Rect.right = (LONG)Window->LastResolution.X;
                        Rect.top = 0;
                        Rect.bottom = (LONG)Window->LastResolution.Y;
                    }
                    else if(Clip.Left > Clip.Right || Clip.Top > Clip.Bottom)
                    {
                        Rect.left = 0;
                        Rect.right = 0;
                        Rect.top = 0;
                        Rect.bottom = 0;
                    }
                    else
                    {
                        Rect.left = (LONG)Clip.Left;
                        Rect.right = (LONG)Clip.Right;
                        Rect.top = (LONG)Clip.Top;
                        Rect.bottom = (LONG)Clip.Bottom;
                    }
                    ID3D11DeviceContext1_RSSetScissorRects(DeviceContext, 1, &Rect);
                }

                ID3D11DeviceContext1_Draw(DeviceContext, 4, 0);
            }break;
        }
    }
}

void RndUpdateTex2DResource(rnd_handle Handle, void *Texels, U64 DataSize)
{
    d3d11_tex2d *Texture = (d3d11_tex2d *)Handle;
    Assert(Texture->Kind == RND_TEX2D_KIND_TEMPORARY && "This update function is only for RND_TEX2D_KIND_TEMPORARY.");

    D3D11_MAPPED_SUBRESOURCE Subresource = {0};
    ID3D11DeviceContext1_Map(DD11RendererState->DeviceContext, (ID3D11Resource *)Texture->Texture, 0,
                             D3D11_MAP_WRITE_DISCARD, 0, &Subresource);

    U8 *Source = (U8 *)Texels;
    U8 *Destination = (U8 *)Subresource.pData;
    for(I32 Row = 0; Row < Texture->Size.Y; ++Row)
    {
        MemoryCopy(Destination, Source, Texture->Size.X * 4);
        Destination += Subresource.RowPitch;
        Source += Texture->Size.X * 4;
    }
    
    ID3D11DeviceContext1_Unmap(DD11RendererState->DeviceContext, (ID3D11Resource *)Texture->Texture, 0);
}

void RndFillTex2DRegion(rnd_handle Handle, rng2_i32 Subrect, void *Texels)
{
    d3d11_tex2d *Texture = (d3d11_tex2d *)Handle;
    Assert(Texture->Kind == RND_TEX2D_KIND_LONG_LIVED && "This update function is only for RND_TEX2D_KIND_LONG_LIVED.");
    U64 BytesPerPixel = Texture->PixelBytes;
    vec2_i32 Dimensions = Dim2I32(Subrect);

    D3D11_BOX DstBox =
    {
        (UINT)Subrect.Left, (UINT)Subrect.Top, 0,
        (UINT)Subrect.Right, (UINT)Subrect.Bottom, 1,
    };
    ID3D11DeviceContext_UpdateSubresource(DD11RendererState->DeviceContext, (ID3D11Resource *)Texture->Texture, 0, &DstBox, Texels,
                                          Dimensions.X * (UINT)BytesPerPixel, 0);
}

void RndTex2DRelease(rnd_handle TextureHandle)
{
    d3d11_tex2d *Texture = (d3d11_tex2d *)TextureHandle;
    if(Texture->View) ID3D11ShaderResourceView_Release(Texture->View);
    if(Texture->Texture) ID3D11Texture2D_Release(Texture->Texture);
    DLLPushFront(DD11RendererState->FirstFreeTexture, DD11RendererState->LastFreeTexture, Texture);
}

rnd_handle RndTex2DAlloc(rnd_tex2d_kind Kind, vec2_i32 Size, rnd_tex2d_format Format, void *Texels)
{
    d3d11_tex2d *Texture = DD11RendererState->FirstFreeTexture;
    if(Texture)
    {
        DLLRemove(DD11RendererState->FirstFreeTexture, DD11RendererState->LastFreeTexture, Texture);
        MemoryZero(Texture, sizeof(d3d11_tex2d));
    }
    else
        Texture = PushStruct(DD11RendererState->Arena, d3d11_tex2d);

    Texture->Kind = Kind;
    Texture->Size = Size;
    Texture->Format = Format;
    
    D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
    UINT CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if(Kind == RND_TEX2D_KIND_TEMPORARY)
        Usage = D3D11_USAGE_DYNAMIC;

    U64 PixelBytes = 4;
    DXGI_FORMAT DXGIFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    switch(Format)
    {
        case RND_TEX2D_FORMAT_R8:     DXGIFormat = DXGI_FORMAT_R8_UNORM;           PixelBytes = 1;  break;
        case RND_TEX2D_FORMAT_A8:     DXGIFormat = DXGI_FORMAT_A8_UNORM;           PixelBytes = 1;  break;
        case RND_TEX2D_FORMAT_RG8:    DXGIFormat = DXGI_FORMAT_R8G8_UNORM;         PixelBytes = 2;  break;
        case RND_TEX2D_FORMAT_RGBA8:  DXGIFormat = DXGI_FORMAT_R8G8B8A8_UNORM;     PixelBytes = 4;  break;
        case RND_TEX2D_FORMAT_BGRA8:  DXGIFormat = DXGI_FORMAT_B8G8R8A8_UNORM;     PixelBytes = 4;  break;
        case RND_TEX2D_FORMAT_R16:    DXGIFormat = DXGI_FORMAT_R16_UNORM;          PixelBytes = 2;  break;
        case RND_TEX2D_FORMAT_RGBA16: DXGIFormat = DXGI_FORMAT_R16G16B16A16_UNORM; PixelBytes = 8;  break;
        case RND_TEX2D_FORMAT_R32:    DXGIFormat = DXGI_FORMAT_R32_FLOAT;          PixelBytes = 4;  break;
        case RND_TEX2D_FORMAT_RG32:   DXGIFormat = DXGI_FORMAT_R32G32_FLOAT;       PixelBytes = 8;  break;
        case RND_TEX2D_FORMAT_RGBA32: DXGIFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; PixelBytes = 16; break;
    }

    Texture->PixelBytes = PixelBytes;
    
    D3D11_SUBRESOURCE_DATA InitialData = {0};
    D3D11_SUBRESOURCE_DATA *InitialDataPtr = 0;
    if(Texels)
    {
        InitialData.pSysMem = Texels;
        InitialData.SysMemPitch = (UINT)PixelBytes * Size.X;
        InitialDataPtr = &InitialData;
    }

    D3D11_TEXTURE2D_DESC TextureDesc = {0};
    TextureDesc.Width = Size.X;
    TextureDesc.Height = Size.Y;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGIFormat;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = Usage;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = CPUAccessFlags;
    ID3D11Device1_CreateTexture2D(DD11RendererState->Device, &TextureDesc, InitialDataPtr, &Texture->Texture);

    ID3D11Device1_CreateShaderResourceView(DD11RendererState->Device, (ID3D11Resource *)Texture->Texture, 0, &Texture->View);
    
    return (rnd_handle)Texture;
}

rnd_handle RndWindowEquip(os_handle OSWindow)
{
    d3d11_window *Window = 0;
    if(DD11RendererState->FirstFreeWindow)
    {
        Window = DD11RendererState->FirstFreeWindow;
        DLLRemove(DD11RendererState->FirstFreeWindow, DD11RendererState->LastFreeWindow, Window);
        MemoryZero(Window, sizeof(d3d11_window));
    }
    else
        Window = PushStruct(DD11RendererState->Arena, d3d11_window);
    
    DLLPushFront(DD11RendererState->FirstActiveWindow, DD11RendererState->LastActiveWindow, Window);
    
    win32_window *Win32Window = (win32_window *)OSWindow;
    HWND Handle = Win32Window->Handle;
    
    DXGI_SWAP_CHAIN_DESC1 SwapchainDesc = {0};
    SwapchainDesc.Width = 0;
    SwapchainDesc.Height = 0;
    SwapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapchainDesc.Stereo = FALSE;
    SwapchainDesc.SampleDesc.Count = 1;
    SwapchainDesc.SampleDesc.Quality = 0;
    SwapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapchainDesc.BufferCount = 2;
    SwapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    SwapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    SwapchainDesc.Flags = 0;

    HRESULT Error = IDXGIFactory2_CreateSwapChainForHwnd(DD11RendererState->DXGIFactory, (IUnknown *)DD11RendererState->Device, Handle,
                                                         &SwapchainDesc, 0, 0, &Window->Swapchain);
    if(FAILED(Error))
    {
        FatalError("Failed to create a D3D11 swap chain for the window!\n");
    }

    IDXGISwapChain1_GetBuffer(Window->Swapchain, 0, &IID_ID3D11Texture2D, (void **)&Window->Framebuffer);
    ID3D11Device1_CreateRenderTargetView(DD11RendererState->Device, (ID3D11Resource *)Window->Framebuffer, 0, &Window->FramebufferRTV);

    return (rnd_handle)Window;
}

void RndWindowUnequip(rnd_handle RndWindow)
{
    // TODO: Release resources
    d3d11_window *Window = (d3d11_window *)RndWindow;

    IDXGISwapChain1_Release(Window->Swapchain);
    ID3D11Texture2D_Release(Window->Framebuffer);
    ID3D11RenderTargetView_Release(Window->FramebufferRTV);
    
    DLLPushFront(DD11RendererState->FirstFreeWindow, DD11RendererState->LastFreeWindow, Window);
    DLLRemove(DD11RendererState->FirstActiveWindow, DD11RendererState->LastActiveWindow, Window);
}
