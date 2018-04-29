@echo off

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

set CMAKEPATH="%ROOTDIR%external\cmake\bin"
set CMAKEFOLDER=bin

if exist %CMAKEFOLDER% rmdir %CMAKEFOLDER% /s /q
mkdir %CMAKEFOLDER%

pushd %CMAKEFOLDER%

%CMAKEPATH%\cmake -G %generator% -Wno-dev ..

popd

if NOT '%1' == 'NOPAUSE' EXIT 0