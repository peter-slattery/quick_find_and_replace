@echo off

set ProgramDevRoot=C:\programs-dev
set ProgramName=find_and_replace
set ProgramDevPath=%ProgramDevRoot%\%ProgramName%

pushd %ProgramDevPath%

IF NOT EXIST .\build\ mkdir .\build

set CommonCompilerFlags=-nologo -FC -WL -WX -W4 -Z7 -Oi -GR- -EHsc -EHa- -wd4127 -wd4702 -wd4101 -wd4505 -wd4100 -wd4189 -wd4244 -wd4201 -wd4996 -I%CommonLibs% -Od -MTd -fp:fast -fp:except- 
set CommonCompilerFlags=-DDEBUG=1 %CommonCompilerFlags%
set CommonLinkerFlags= -opt:ref -incremental:no user32.lib gdi32.lib opengl32.lib

pushd build

cl %CommonCompilerFlags% ..\src\win32_%ProgramName%.cpp  /link %CommonLinkerFlags%

popd
popd