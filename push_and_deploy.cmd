
git push

@ if %ERRORLEVEL% neq 0 (
	@rem this makes sure that all non local changes have been merged
	@rem ensuring everything is at latest
	@echo ERROR: Unable to push. Aborting deploy 
	@goto:eof
)

set version_counter_abspath=\\ubisoft.org\projects\piciteam\version_counters\version_counter_gameteamhandmade.txt

set /P previous_version=< %version_counter_abspath%

set /A version=previous_version+1

echo %version% > %version_counter_abspath%


set SOLUTION_ROOT_DIR_ABSPATH=%~dp0

set build_dir_abspath=d:\build\gameteamhandmade\%version%

md c:\empty
robocopy /mir c:\empty %build_dir_abspath%


if %USERNAME% == rfey (
	git clone --depth 1 file:///C/Users/%USERNAME%/git/gitlab-ncsa.ubisoft.org/piciteam/summer_jam_bb_c_handmade %build_dir_abspath%
) else (
	git clone --recursive %SOLUTION_ROOT_DIR_ABSPATH% %build_dir_abspath%
)

pushd %build_dir_abspath%

call %build_dir_abspath%\build.bat

echo on

call %~dp0.\get_upi.cmd

%UPI_EXE% get signtool 1
set signtool_exe=%userprofile%\upi\p\signtool\1\signtool.exe

set exe_abspath=%build_dir_abspath%\bin\main.exe

if exist %exe_abspath% (
	%signtool_exe% sign /f \\firebird.piteam.ubisoft.org\piteam_secrets_share\piteam.pfx /p Piteam2019 /t http://timestamp.digicert.com /fd SHA256 %exe_abspath%

	%UPI_EXE% deploy gameteamhandmade %version% %build_dir_abspath%
	
	%UPI_EXE% publish_package shared gameteamhandmade %version% %build_dir_abspath%
	
	%UPI_EXE% sign_version gameteamhandmade %version% piciteam \\ubisoft.org\projects\piciteam\secrets\pici_upi_signing.private_key
	
)
popd
