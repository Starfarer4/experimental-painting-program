@echo off

REM Compile the D3D11 shaders
fxc /nologo /T vs_4_0 /O3 /WX /Fh code\renderer\d3d11\shaders\generated\vs_kind_rect.h /Vn DD11VShaderBufferKindRect /Qstrip_reflect /Qstrip_debug /Qstrip_priv code\renderer\d3d11\shaders\vs_kind_rect.hlsl
fxc /nologo /T ps_4_0 /O3 /WX /Fh code\renderer\d3d11\shaders\generated\ps_kind_rect.h /Vn DD11PShaderBufferKindRect /Qstrip_reflect /Qstrip_debug /Qstrip_priv code\renderer\d3d11\shaders\ps_kind_rect.hlsl

fxc /nologo /T vs_4_0 /O3 /WX /Fh code\renderer\d3d11\shaders\generated\vs_kind_checkerboard.h /Vn DD11VShaderBufferKindCheckerboard /Qstrip_reflect /Qstrip_debug /Qstrip_priv code\renderer\d3d11\shaders\vs_kind_checkerboard.hlsl
fxc /nologo /T ps_4_0 /O3 /WX /Fh code\renderer\d3d11\shaders\generated\ps_kind_checkerboard.h /Vn DD11PShaderBufferKindCheckerboard /Qstrip_reflect /Qstrip_debug /Qstrip_priv code\renderer\d3d11\shaders\ps_kind_checkerboard.hlsl

if not exist build mkdir build

set THIRD_PARTY_LINKS=..\third_party\freetype-2.13.2\objs\freetype.lib ..\third_party\harfbuzz-8.3.0\build\harfbuzz.lib
set THIRD_PARTY_INCLUDES=/I..\third_party\stb\ /I..\third_party\freetype-2.13.2\include\ /I..\third_party\harfbuzz-8.3.0\src\

set DEFINES=/DPX_PLATFORM_WINDOWS=1 /DPX_DEBUG=1
set LINKS=kernel32.lib user32.lib d3d11.lib dxguid.lib ole32.lib windowscodecs.lib gdi32.lib

pushd build

rc /nologo /fo resources.res ..\data\resources.rc

REM Build and run the code generator
if not exist codegen mkdir codegen
pushd codegen
cl /nologo /Zi /FC /I..\..\third_party\metadesk ..\..\code\codegen\codegen.c
codegen.exe ../../code/ui/ ../../code/base/ ../../code/qualia/ ../../code/renderer/ ../../code/os/
popd

cl /nologo /Zi /W3 /FC /I..\code\ %THIRD_PARTY_INCLUDES% ..\code\qualia\qualia.c resources.res %DEFINES% %LINKS% %THIRD_PARTY_LINKS%

popd
