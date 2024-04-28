if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recol256
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col002             -b1
call :CONV col004             -b2

call :CONV col002-xdp2        -b1 -xdp:2
call :CONV col002-xde         -b1 -xde
call :CONV col004-xde         -b2 -xde
call :CONV col004-xdp2        -b2 -xdp:2

call :CONV col08jp-xde        -b3 -cp1 -xde
call :CONV col016jp-xde       -b4 -cp1 -xde
call :CONV col016win-xde      -b4 -cp2 -xde

call :CONV col256jp-xde       -b8 -cp1 -xde
call :CONV col256win-xde      -b8 -cp2 -xde
call :CONV col256xterm-xde    -b8 -cp7 -xde
call :CONV col064sp-xde       -b8 -cp8 -xde

goto END

call :CONV col064fix          -b6 -cp1
call :CONV col064fix-xd09x8   -b6 -cp1 -xd9:3
call :CONV col128fix          -b7 -cp1
call :CONV col128fix-xd09x8   -b7 -cp1 -xd9:3

call :CONV col256jp           -b8 -cp1
call :CONV col256jp-xd09x2    -b8 -cp1 -xd9:1
call :CONV col256jp-xd09x4    -b8 -cp1 -xd9:2
call :CONV col256jp-xd09x8    -b8 -cp1 -xd9:3
call :CONV col256jp-xd12x8    -b8 -cp1 -xd12:3
call :CONV col256jp-xd15x8    -b8 -cp1 -xd15:3

call :CONV col256win          -b8 -cp2
call :CONV col256win-xd09x2   -b8 -cp2 -xd9:1
call :CONV col256win-xd09x4   -b8 -cp2 -xd9:2
call :CONV col256win-xd09x8   -b8 -cp2 -xd9:3
call :CONV col256win-xd12x8   -b8 -cp2 -xd12:3
call :CONV col256win-xd15x8   -b8 -cp2 -xd15:3

call :CONV col256xterm        -b8 -cp7
call :CONV col256xterm-xd09x2 -b8 -cp7 -xd9:1
call :CONV col256xterm-xd09x4 -b8 -cp7 -xd9:2
call :CONV col256xterm-xd09x8 -b8 -cp7 -xd9:3
call :CONV col256xterm-xd12x4 -b8 -cp7 -xd12:2
call :CONV col256xterm-xd12x8 -b8 -cp7 -xd12:3
call :CONV col256xterm-xd15x8 -b8 -cp7 -xd15:3

call :CONV col256hist         -b8 -cp5
call :CONV col256hist-xd09x2  -b8 -cp5 -xd9:1
call :CONV col256hist-xd09x4  -b8 -cp5 -xd9:2
call :CONV col256hist-xd09x8  -b8 -cp5 -xd9:3
call :CONV col256hist-xd12x4  -b8 -cp5 -xd12:2
call :CONV col256hist-xd12x8  -b8 -cp5 -xd12:3
call :CONV col256hist-xd15x2  -b8 -cp5 -xd15:1
call :CONV col256hist-xd15x4  -b8 -cp5 -xd15:2
call :CONV col256hist-xd15x8  -b8 -cp5 -xd15:3

call :CONV col016mcyuv        -b4 -cp3
call :CONV col016mcyuv-xd09x8 -b4 -cp3 -xd9:3

call :CONV col256mcyuv        -b8 -cp3
call :CONV col256mcyuv-xd09x2 -b8 -cp3 -xd9:1
call :CONV col256mcyuv-xd09x4 -b8 -cp3 -xd9:2
call :CONV col256mcyuv-xd09x8 -b8 -cp3 -xd9:3
call :CONV col256mcyuv-xd12x4 -b8 -cp3 -xd12:2
call :CONV col256mcyuv-xd12x8 -b8 -cp3 -xd12:3
call :CONV col256mcyuv-xd15x2 -b8 -cp3 -xd15:1
call :CONV col256mcyuv-xd15x4 -b8 -cp3 -xd15:2
call :CONV col256mcyuv-xd15x8 -b8 -cp3 -xd15:3

call :CONV col256mcrgb        -b8 -cp4
call :CONV col256mcrgb-xd09x2 -b8 -cp4 -xd9:1
call :CONV col256mcrgb-xd09x4 -b8 -cp4 -xd9:2
call :CONV col256mcrgb-xd09x8 -b8 -cp4 -xd9:3
call :CONV col256mcrgb-xd12x4 -b8 -cp4 -xd12:2
call :CONV col256mcrgb-xd12x8 -b8 -cp4 -xd12:3
call :CONV col256mcrgb-xd15x2 -b8 -cp4 -xd15:1
call :CONV col256mcrgb-xd15x4 -b8 -cp4 -xd15:2
call :CONV col256mcrgb-xd15x8 -b8 -cp4 -xd15:3

call :CONV col256rgb5c40        -b8 -cp8
call :CONV col256rgb5c40-xd09x2 -b8 -cp8 -xd9:1
call :CONV col256rgb5c40-xd09x4 -b8 -cp8 -xd9:2
call :CONV col256rgb5c40-xd09x8 -b8 -cp8 -xd9:3
call :CONV col256rgb5c40-xd12x4 -b8 -cp8 -xd12:2
call :CONV col256rgb5c40-xd12x8 -b8 -cp8 -xd12:3
call :CONV col256rgb5c40-xd15x8 -b8 -cp8 -xd15:3

call :CONV col216rgb5c40        -b8 -cp8         -cn216
call :CONV col216rgb5c40-xd09x2 -b8 -cp8 -xd9:1  -cn216
call :CONV col216rgb5c40-xd09x4 -b8 -cp8 -xd9:2  -cn216
call :CONV col216rgb5c40-xd09x8 -b8 -cp8 -xd9:3  -cn216
call :CONV col216rgb5c40-xd12x4 -b8 -cp8 -xd12:2 -cn216
call :CONV col216rgb5c40-xd12x8 -b8 -cp8 -xd12:3 -cn216
call :CONV col216rgb5c40-xd15x8 -b8 -cp8 -xd15:3 -cn216

call :CONV col216xterm        -b8 -cp7         -cn216
call :CONV col216xterm-xd09x2 -b8 -cp7 -xd9:1  -cn216
call :CONV col216xterm-xd09x4 -b8 -cp7 -xd9:2  -cn216
call :CONV col216xterm-xd09x8 -b8 -cp7 -xd9:3  -cn216
call :CONV col216xterm-xd12x4 -b8 -cp7 -xd12:2 -cn216
call :CONV col216xterm-xd12x8 -b8 -cp7 -xd12:3 -cn216
call :CONV col216xterm-xd15x8 -b8 -cp7 -xd15:3 -cn216

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
