@echo off

if not exist build mkdir build

pushd build

cl /nologo /O2 /c ..\src\harfbuzz.cc /DHB_LEAN=1 /DHB_MINI=1
lib /nologo /OUT:harfbuzz.lib harfbuzz.obj

popd