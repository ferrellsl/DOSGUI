@echo off
rem ---------------------------------------------------------------------------
rem  Builds the GUI library and the example program with Open Watcom.
rem
rem     build          16-bit, large model     ->  build16\gui.lib, demo.exe
rem     build 32       32-bit, DOS/4GW         ->  build32\gui.lib, demo.exe
rem
rem  Open Watcom is looked for in C:\WATCOM unless WATCOM is already set.
rem ---------------------------------------------------------------------------
setlocal
if "%WATCOM%"=="" set WATCOM=C:\WATCOM
set PATH=%WATCOM%\binnt64;%WATCOM%\binnt;%PATH%
set INCLUDE=%WATCOM%\h

set BITS=%1
if "%BITS%"=="" set BITS=16
set OUT=build%BITS%
if not exist %OUT% mkdir %OUT%

if "%BITS%"=="32" (
  set CC=wcc386 -q -bt=dos -os -w2
  set SYS=dos4g
  set STACK=65536
) else (
  set CC=wcc -q -ml -bt=dos -0 -os -w2
  set SYS=dos
  set STACK=20000
)

set MODULES=gfx gport dispfunc base window button menu fields scr_bar perc_bar help filedlg

for %%f in (%MODULES%) do (
  %CC% -fo=%OUT%\%%f.obj %%f.c
  if errorlevel 1 exit /b 1
)

if exist %OUT%\gui.lib del %OUT%\gui.lib
for %%f in (%MODULES%) do (
  wlib -q -b %OUT%\gui.lib +%OUT%\%%f.obj
  if errorlevel 1 exit /b 1
)

%CC% -fo=%OUT%\demo.obj demo.c
if errorlevel 1 exit /b 1

wlink option quiet system %SYS% option stack=%STACK% name %OUT%\demo.exe file %OUT%\demo.obj library %OUT%\gui.lib
if errorlevel 1 exit /b 1

rem  what the example needs beside it
copy /y MENUFILE %OUT% >nul
copy /y HELPFILE %OUT% >nul
if "%BITS%"=="32" copy /y %WATCOM%\binw\dos4gw.exe %OUT% >nul

echo built %OUT%\gui.lib and %OUT%\demo.exe
