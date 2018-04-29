@echo off

set EXT=%~dp0external

call :RequestAdminElevation "%~dpf0" %* || goto:eof

echo Installing dependencies into '%EXT%'
if not exist "%EXT%" md "%EXT%"

REM ------------------ imgui

set SRC="https://github.com/ocornut/imgui/archive/v1.53.zip"
set ZIP=%EXT%\imgui-1.53.zip
set DST=%EXT%\.

if not exist "%EXT%\imgui" (
echo Downloading dear IMGUI from %SRC%...
powershell -Command "Start-BitsTransfer '%SRC%' '%ZIP%'"
powershell -Command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%ZIP%', '%DST%'); }"
rename "%EXT%\imgui-1.53" imgui

mkdir "%EXT%\gl3w\include"
xcopy "%EXT%\imgui\examples\libs\gl3w" "%EXT%\gl3w\include" /E

del "%ZIP%"
) else echo imgui detected. skipping.

REM ------------------ glfw3

set SRC=https://github.com/glfw/glfw/archive/3.2.1.zip
set ZIP=%EXT%\glfw-3.2.1.zip
set DST=%EXT%\.

if not exist "%EXT%\glfw3" (
echo Downloading glfw3-3.2.1 from %SRC%...
powershell -Command "Start-BitsTransfer '%SRC%' '%ZIP%'"
powershell -Command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%ZIP%', '%DST%'); }"
rename "%EXT%\glfw-3.2.1" glfw3
del "%ZIP%"
) else echo glfw3 detected. skipping.

REM ------------------ cmake

set SRC=https://cmake.org/files/v3.7/cmake-3.7.2-win64-x64.zip
set ZIP=%EXT%\cmake-3.7.2-win64-x64.zip
set DST=%EXT%\.

if not exist "%EXT%\cmake" (
echo Downloading cmake-3.7.2 from %SRC%...
powershell -Command "Start-BitsTransfer '%SRC%' '%ZIP%'"
powershell -Command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%ZIP%', '%DST%'); }"
rename "%EXT%\cmake-3.7.2-win64-x64" cmake
del "%ZIP%"
) else echo cmake detected. skipping.

REM ----------------------- soil

set SRC=http://www.lonesock.net/files/soil.zip
set ZIP=%EXT%\soil.zip
set DST=%EXT%\.

if not exist "%EXT%\soil" (
echo Downloading soil from %SRC%...
powershell -Command "Start-BitsTransfer '%SRC%' '%ZIP%'"
powershell -Command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%ZIP%', '%DST%'); }"
rename "%EXT%\Simple OpenGL Image Library" soil
del "%ZIP%"
del /f "%EXT%\soil\src\test_SOIL.cpp"
) else echo soil detected. skipping.

REM ------------------ TinyObjLoader

set SRC="https://raw.githubusercontent.com/syoyo/tinyobjloader/v1.0.6/tiny_obj_loader.h"
set FILE=%EXT%\tiny_obj_loader.h
set DST=%EXT%\tinyobjloader

if not exist "%EXT%\tinyobjloader" (
echo Downloading tinyobjloader from %SRC%...
powershell -Command "Start-BitsTransfer '%SRC%' '%FILE%'"
mkdir "%DST%"
move "%FILE%" "%DST%"
) else echo tinyobjloader detected. skipping.

REM ------------------ GLM

set SRC="https://github.com/g-truc/glm/archive/0.9.9-a2.zip"
set ZIP=%EXT%\0.9.9-a2.zip
set DST=%EXT%\.

if not exist "%EXT%\GLM" (
echo Downloading GLM from %SRC%...
powershell -Command "Start-BitsTransfer '%SRC%' '%ZIP%'"
powershell -Command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%ZIP%', '%DST%'); }"
rename "%EXT%\glm-0.9.9-a2" GLM
if exist "%EXT%\GLM" (
	mkdir "%EXT%\GLM\include"
	xcopy "%EXT%\GLM\glm" "%EXT%\GLM\include\GLM\" /E
	rmdir "%EXT%\GLM\glm" /s /q
)

del "%ZIP%"
) else echo GLM detected. skipping.

@echo off

REM -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

set CMAKEFOLDER=bin

:: Generate and Build GLFW3

IF "%~1" == "" GOTO No1
	set generator=%1
	ECHO Using %generator%
GOTO End1
:No1
	ECHO Using Visual Studio 15 2017 Win64
	set generator="Visual Studio 15 2017 Win64"
GOTO End1
:End1

set ROOTDIR=%~dp0

cd "%ROOTDIR%external\glfw3"

set CURDIR=%~dp0
set CMAKEPATH=%ROOTDIR%external\cmake\bin

if exist "%CMAKEFOLDER%" rmdir "%CMAKEFOLDER%" /s /q
mkdir "%CMAKEFOLDER%"

pushd "%CMAKEFOLDER%"

"%CMAKEPATH%\cmake" -G %generator% -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF ..
"%CMAKEPATH%\cmake" --build . --config Release
"%CMAKEPATH%\cmake" --build . --config Debug

popd

goto:eof

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:RequestAdminElevation FilePath %* || goto:eof
:: 
:: 
:: Func: opens an admin elevation prompt. If elevated, runs everything after the function call, with elevated rights.
:: Returns: -1 if elevation was requested
::           0 if elevation was successful
::           1 if an error occured
:: 
:: USAGE:
:: If function is copied to a batch file:
::     call :RequestAdminElevation "%~dpf0" %* || goto:eof
::
:: If called as an external library (from a separate batch file):
::     set "_DeleteOnExit=0" on Options
::     (call :RequestAdminElevation "%~dpf0" %* || goto:eof) && CD /D %CD%
::
:: If called from inside another CALL, you must set "_ThisFile=%~dpf0" at the beginning of the file
::     call :RequestAdminElevation "%_ThisFile%" %* || goto:eof
::
:: If you need to use the ! char in the arguments, the calling must be done like this, and afterwards you must use %args% to get the correct arguments:
::      set "args=%* "
::      call :RequestAdminElevation .....   use one of the above but replace the %* with %args:!={a)%
::      set "args=%args:{a)=!%" 
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
setlocal ENABLEDELAYEDEXPANSION & set "_FilePath=%~1"
  if NOT EXIST "!_FilePath!" (echo/Read RequestAdminElevation usage information)
  :: UAC.ShellExecute only works with 8.3 filename, so use %~s1
  set "_FN=_%~ns1" & echo/%TEMP%| findstr /C:"(" >nul && (echo/ERROR: %%TEMP%% path can not contain parenthesis &pause &endlocal &fc;: 2>nul & goto:eof)
  :: Remove parenthesis from the temp filename
  set _FN=%_FN:(=%
  set _vbspath="%temp:~%\%_FN:)=%.vbs" & set "_batpath=%temp:~%\%_FN:)=%.bat"

  :: Test if we gave admin rights
  fltmc >nul 2>&1 || goto :_getElevation

  :: Elevation successful
  (if exist %_vbspath% ( del %_vbspath% )) & (if exist %_batpath% ( del %_batpath% )) 
  :: Set ERRORLEVEL 0, set original folder and exit
  endlocal & CD /D "%~dp1" & ver >nul & goto:eof

  :_getElevation
  echo/Requesting elevation...
  :: Try to create %_vbspath% file. If failed, exit with ERRORLEVEL 1
  echo/Set UAC = CreateObject^("Shell.Application"^) > %_vbspath% || (echo/&echo/Unable to create %_vbspath% & endlocal &md; 2>nul &goto:eof) 
  echo/UAC.ShellExecute "%_batpath%", "", "", "runas", 1 >> %_vbspath% & echo/wscript.Quit(1)>> %_vbspath%
  :: Try to create %_batpath% file. If failed, exit with ERRORLEVEL 1
  echo/@%* > "%_batpath%" || (echo/&echo/Unable to create %_batpath% & endlocal &md; 2>nul &goto:eof)
  echo/@if %%errorlevel%%==9009 (echo/^&echo/Admin user could not read the batch file. If running from a mapped drive or UNC path, check if Admin user can read it.)^&echo/^& @if %%errorlevel%% NEQ 0 pause >> "%_batpath%"

  :: Run %_vbspath%, that calls %_batpath%, that calls the original file
  %_vbspath% && (echo/&echo/Failed to run VBscript %_vbspath% &endlocal &md; 2>nul & goto:eof)

  :: Vbscript has been run, exit with ERRORLEVEL -1
  echo/&echo/Elevation was requested on a new CMD window &endlocal &fc;: 2>nul & goto:eof
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::