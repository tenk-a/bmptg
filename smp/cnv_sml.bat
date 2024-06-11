if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_sml
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV smp_40-jp  -b8 -cp1 -xrs40
call :CONV smp_60-jp  -b8 -cp1 -xrs60

call :CONV smp_40-hist -b8 -cp5 -xrs40
call :CONV smp_60-hist -b8 -cp5 -xrs60

call :CONV smp_40-xterm -b8 -cp7 -xrs40
call :CONV smp_60-xterm -b8 -cp7 -xrs60


goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
