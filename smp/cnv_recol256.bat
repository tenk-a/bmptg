if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recol256
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col004              -b2
call :CONV col004-xde          -b2 -xde
call :CONV col004-xdex2        -b2 -xde:1
call :CONV col004-xdex4        -b2 -xde:2
call :CONV col004-xdex8        -b2 -xde:3
call :CONV col004-xdpx2        -b2 -xdp:1
call :CONV col004-xdpx4        -b2 -xdp:2
call :CONV col004-xdpx8        -b2 -xdp:3
call :CONV col004-xdo09x2      -b2 -xdo09:1
call :CONV col004-xdo09x4      -b2 -xdo09:2
call :CONV col004-xdo09x8      -b2 -xdo09:3


call :CONV col016mcyuv         -b4 -cp3
call :CONV col016mcyuv-xde     -b4 -cp3 -xde
call :CONV col016mcyuv-xdex2   -b4 -cp3 -xde:1
call :CONV col016mcyuv-xdex4   -b4 -cp3 -xde:2
call :CONV col016mcyuv-xdex8   -b4 -cp3 -xde:3
call :CONV col016mcyuv-xdpx8   -b4 -cp3 -xdp:3
call :CONV col016mcyuv-xdo09x2 -b4 -cp3 -xd9:1
call :CONV col016mcyuv-xdo09x4 -b4 -cp3 -xd9:2
call :CONV col016mcyuv-xdo09x8 -b4 -cp3 -xd9:3

call :CONV col256jp-xde       -b8 -cp1 -xde
call :CONV col256win-xde      -b8 -cp2 -xde
call :CONV col256xterm-xde    -b8 -cp7 -xde

call :CONV col064fix           -b6 -cp1
call :CONV col064fix-xdex8     -b6 -cp1 -xde:3
call :CONV col064fix-xdpx8     -b6 -cp1 -xdp:3
call :CONV col064fix-xdo09x8   -b6 -cp1 -xd9:3

call :CONV col064sp            -b8 -cp8
call :CONV col064sp-xde        -b8 -cp8 -xde
call :CONV col064sp-xdpx8      -b8 -cp8 -xdp:3

call :CONV col128fix           -b7 -cp1
call :CONV col128fix-xde       -b7 -cp1 -xde9
call :CONV col128fix-xde09x8   -b7 -cp1 -xde9:3
call :CONV col128fix-xdp09x8   -b7 -cp1 -xdp9:3
call :CONV col128fix-xdo09x8   -b7 -cp1 -xd9:3

call :CONV col256jp            -b8 -cp1
call :CONV col256jp-xde        -b8 -cp1 -xde
call :CONV col256jp-xdex2      -b8 -cp1 -xde:1
call :CONV col256jp-xdex4      -b8 -cp1 -xde:2
call :CONV col256jp-xdpx2      -b8 -cp1 -xdp:1
call :CONV col256jp-xdpx4      -b8 -cp1 -xdp:2
call :CONV col256jp-xdo09x2    -b8 -cp1 -xd9:1
call :CONV col256jp-xdo09x4    -b8 -cp1 -xd9:2
call :CONV col256jp-xdo09x8    -b8 -cp1 -xd9:3
call :CONV col256jp-xdo12x8    -b8 -cp1 -xd12:3
call :CONV col256jp-xdo15x8    -b8 -cp1 -xd15:3

call :CONV col256win           -b8 -cp2
call :CONV col256win-xde       -b8 -cp2 -xde
call :CONV col256win-xdex2     -b8 -cp2 -xde:1
call :CONV col256win-xdpx2     -b8 -cp2 -xdp:1
call :CONV col256win-xdo09x2   -b8 -cp2 -xd9:1
call :CONV col256win-xdo09x4   -b8 -cp2 -xd9:2
call :CONV col256win-xdo09x8   -b8 -cp2 -xd9:3
call :CONV col256win-xdo12x8   -b8 -cp2 -xd12:3
call :CONV col256win-xdo15x8   -b8 -cp2 -xd15:3

call :CONV col256xterm        -b8 -cp7
call :CONV col256xterm-xde    -b8 -cp7 -xde
call :CONV col256xterm-xdex2  -b8 -cp7 -xde:1
call :CONV col256xterm-xdpx2  -b8 -cp7 -xdp:1
call :CONV col256xterm-xd09x2 -b8 -cp7 -xd9:1
call :CONV col256xterm-xd09x4 -b8 -cp7 -xd9:2
call :CONV col256xterm-xd09x8 -b8 -cp7 -xd9:3
call :CONV col256xterm-xd12x4 -b8 -cp7 -xd12:2
call :CONV col256xterm-xd12x8 -b8 -cp7 -xd12:3
call :CONV col256xterm-xd15x8 -b8 -cp7 -xd15:3

call :CONV col216xterm         -b8 -cp7         -cn216
call :CONV col216xterm-xde     -b8 -cp7 -xde    -cn216
call :CONV col216xterm-xdex2   -b8 -cp7 -xde:1  -cn216
call :CONV col216xterm-xdpx2   -b8 -cp7 -xdp:1  -cn216
call :CONV col216xterm-xdo09x2 -b8 -cp7 -xd9:1  -cn216
call :CONV col216xterm-xdo09x4 -b8 -cp7 -xd9:2  -cn216
call :CONV col216xterm-xdo09x8 -b8 -cp7 -xd9:3  -cn216
call :CONV col216xterm-xdo12x4 -b8 -cp7 -xd12:2 -cn216
call :CONV col216xterm-xdo12x8 -b8 -cp7 -xd12:3 -cn216
call :CONV col216xterm-xdo15x8 -b8 -cp7 -xd15:3 -cn216


call :CONV col256hist          -b8 -cp5
call :CONV col256hist-xde      -b8 -cp5 -xde
call :CONV col256hist-xdex2    -b8 -cp5 -xde:1
call :CONV col256hist-xdpx2    -b8 -cp5 -xdp:1
call :CONV col256hist-xdo09x2  -b8 -cp5 -xd9:1
call :CONV col256hist-xdo09x4  -b8 -cp5 -xd9:2
call :CONV col256hist-xdo09x8  -b8 -cp5 -xd9:3
call :CONV col256hist-xdo12x4  -b8 -cp5 -xd12:2
call :CONV col256hist-xdo12x8  -b8 -cp5 -xd12:3
call :CONV col256hist-xdo15x2  -b8 -cp5 -xd15:1
call :CONV col256hist-xdo15x4  -b8 -cp5 -xd15:2
call :CONV col256hist-xdo15x8  -b8 -cp5 -xd15:3

call :CONV col256mcyuv         -b8 -cp3
call :CONV col256mcyuv-xde     -b8 -cp3 -xde
call :CONV col256mcyuv-xdex2   -b8 -cp3 -xde:1
call :CONV col256mcyuv-xdpx2   -b8 -cp3 -xdp:1
call :CONV col256mcyuv-xdo09x2 -b8 -cp3 -xd9:1
call :CONV col256mcyuv-xdo09x4 -b8 -cp3 -xd9:2
call :CONV col256mcyuv-xdo09x8 -b8 -cp3 -xd9:3
call :CONV col256mcyuv-xdo12x4 -b8 -cp3 -xd12:2
call :CONV col256mcyuv-xdo12x8 -b8 -cp3 -xd12:3
call :CONV col256mcyuv-xdo15x2 -b8 -cp3 -xd15:1
call :CONV col256mcyuv-xdo15x4 -b8 -cp3 -xd15:2
call :CONV col256mcyuv-xdo15x8 -b8 -cp3 -xd15:3

call :CONV col256mcrgb         -b8 -cp4
call :CONV col256mcrgb-xde     -b8 -cp4 -xde
call :CONV col256mcrgb-xdex2   -b8 -cp4 -xde:1
call :CONV col256mcrgb-xdpx2   -b8 -cp4 -xdp:1
call :CONV col256mcrgb-xdo09x2 -b8 -cp4 -xd9:1
call :CONV col256mcrgb-xdo09x4 -b8 -cp4 -xd9:2
call :CONV col256mcrgb-xdo09x8 -b8 -cp4 -xd9:3
call :CONV col256mcrgb-xdo12x4 -b8 -cp4 -xd12:2
call :CONV col256mcrgb-xdo12x8 -b8 -cp4 -xd12:3
call :CONV col256mcrgb-xdo15x2 -b8 -cp4 -xd15:1
call :CONV col256mcrgb-xdo15x4 -b8 -cp4 -xd15:2
call :CONV col256mcrgb-xdo15x8 -b8 -cp4 -xd15:3

call :CONV col256rgb5c40         -b8 -cp8
call :CONV col256rgb5c40-xde     -b8 -cp8 -xde
call :CONV col256rgb5c40-xdex2   -b8 -cp8 -xde:1
call :CONV col256rgb5c40-xdpx2   -b8 -cp8 -xdp:1
call :CONV col256rgb5c40-xdo09x2 -b8 -cp8 -xd9:1
call :CONV col256rgb5c40-xdo09x4 -b8 -cp8 -xd9:2
call :CONV col256rgb5c40-xdo09x8 -b8 -cp8 -xd9:3
call :CONV col256rgb5c40-xdo12x4 -b8 -cp8 -xd12:2
call :CONV col256rgb5c40-xdo12x8 -b8 -cp8 -xd12:3
call :CONV col256rgb5c40-xdo15x8 -b8 -cp8 -xd15:3

call :CONV col216rgb5c40         -b8 -cp8         -cn216
call :CONV col216rgb5c40-xde     -b8 -cp8 -xde    -cn216
call :CONV col216rgb5c40-xdex2   -b8 -cp8 -xde:1  -cn216
call :CONV col216rgb5c40-xdpx2   -b8 -cp8 -xdp:1  -cn216
call :CONV col216rgb5c40-xdo09x2 -b8 -cp8 -xd9:1  -cn216
call :CONV col216rgb5c40-xdo09x4 -b8 -cp8 -xd9:2  -cn216
call :CONV col216rgb5c40-xdo09x8 -b8 -cp8 -xd9:3  -cn216
call :CONV col216rgb5c40-xdo12x4 -b8 -cp8 -xd12:2 -cn216
call :CONV col216rgb5c40-xdo12x8 -b8 -cp8 -xd12:3 -cn216
call :CONV col216rgb5c40-xdo15x8 -b8 -cp8 -xd15:3 -cn216

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
