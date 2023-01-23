@echo off
set start=%time%

if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
)

IF NOT EXIST bin mkdir bin
pushd bin

cl -Od -W4 -wd4505 -wd4100 -wd4201 -nologo /DSLOW /DINTERNAL -FC -Zi ..\src\main.cpp /link -incremental:no -opt:ref user32.lib winmm.lib Gdi32.lib opengl32.lib

del game_*.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp
cl -Od -W4 -wd4505 -wd4100 -wd4201 -nologo /DSLOW /DINTERNAL -FC -Zi ..\src\game.cpp -LD /link -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:UpdateGamePlay -EXPORT:UpdateGameAudio -EXPORT:RenderGameplay
del lock.tmp

cl -Od -W4 -wd4505 -wd4100 -wd4201 -nologo /DSLOW /DINTERNAL -FC -Zi ..\src\bot.cpp -LD /link -incremental:no -opt:ref -PDB:bot.pdb -EXPORT:UpdateBot /OUT:player2.dll

del /q *.exp
del /q *.lib

popd

set end=%time%
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%start%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
for /f %options% %%a in ("%end%") do set end_h=%%a&set /a end_m=100%%b %% 100&set /a end_s=100%%c %% 100&set /a end_ms=100%%d %% 100

set /a hours=%end_h%-%start_h%
set /a mins=%end_m%-%start_m%
set /a secs=%end_s%-%start_s%
set /a ms=%end_ms%-%start_ms%
if %ms% lss 0 set /a secs = %secs% - 1 & set /a ms = 100%ms%
if %secs% lss 0 set /a mins = %mins% - 1 & set /a secs = 60%secs%
if %mins% lss 0 set /a hours = %hours% - 1 & set /a mins = 60%mins%
if %hours% lss 0 set /a hours = 24%hours%
if 1%ms% lss 100 set ms=0%ms%

:: Mission accomplished
set /a totalsecs = %hours%*3600 + %mins%*60 + %secs%
echo Build Duration : (%totalsecs%.%ms%s)

pushd bin

popd