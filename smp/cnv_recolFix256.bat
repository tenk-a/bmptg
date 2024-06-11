if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolFix256
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col256jp            -b8 -cp1

call :CONV col256jp-xdp09      -b8 -cp1 -xdp09:0
call :CONV col256jp-xdp09x2    -b8 -cp1 -xdp09:1
call :CONV col256jp-xdp09x4    -b8 -cp1 -xdp09:2
call :CONV col256jp-xdp09x8    -b8 -cp1 -xdp09:3

call :CONV col256jp-xde09      -b8 -cp1 -xde09:0
call :CONV col256jp-xde09x2    -b8 -cp1 -xde09:1
call :CONV col256jp-xde09x4    -b8 -cp1 -xde09:2
call :CONV col256jp-xde09x8    -b8 -cp1 -xde09:3

call :CONV col256jp-xdp12      -b8 -cp1 -xdp12:0
call :CONV col256jp-xdp12x2    -b8 -cp1 -xdp12:1
call :CONV col256jp-xdp12x4    -b8 -cp1 -xdp12:2
call :CONV col256jp-xdp12x8    -b8 -cp1 -xdp12:3

call :CONV col256jp-xde12      -b8 -cp1 -xde12:0
call :CONV col256jp-xde12x2    -b8 -cp1 -xde12:1
call :CONV col256jp-xde12x4    -b8 -cp1 -xde12:2
call :CONV col256jp-xde12x8    -b8 -cp1 -xde12:3

call :CONV col256jp-xdp15      -b8 -cp1 -xdp15:0
call :CONV col256jp-xdp15x2    -b8 -cp1 -xdp15:1
call :CONV col256jp-xdp15x4    -b8 -cp1 -xdp15:2
call :CONV col256jp-xdp15x8    -b8 -cp1 -xdp15:3

call :CONV col256jp-xde15      -b8 -cp1 -xde15:0
call :CONV col256jp-xde15x2    -b8 -cp1 -xde15:1
call :CONV col256jp-xde15x4    -b8 -cp1 -xde15:2
call :CONV col256jp-xde15x8    -b8 -cp1 -xde15:3


call :CONV col256win            -b8 -cp2

call :CONV col256win-xdp12      -b8 -cp2 -xdp12:0
call :CONV col256win-xdp12x2    -b8 -cp2 -xdp12:1
call :CONV col256win-xdp12x4    -b8 -cp2 -xdp12:2
call :CONV col256win-xdp12x8    -b8 -cp2 -xdp12:3

call :CONV col256win-xde12      -b8 -cp2 -xde12:0
call :CONV col256win-xde12x2    -b8 -cp2 -xde12:1
call :CONV col256win-xde12x4    -b8 -cp2 -xde12:2
call :CONV col256win-xde12x8    -b8 -cp2 -xde12:3

call :CONV col256win-xdp15      -b8 -cp2 -xdp15:0
call :CONV col256win-xdp15x2    -b8 -cp2 -xdp15:1
call :CONV col256win-xdp15x4    -b8 -cp2 -xdp15:2
call :CONV col256win-xdp15x8    -b8 -cp2 -xdp15:3

call :CONV col256win-xde15      -b8 -cp2 -xde15:0
call :CONV col256win-xde15x2    -b8 -cp2 -xde15:1
call :CONV col256win-xde15x4    -b8 -cp2 -xde15:2
call :CONV col256win-xde15x8    -b8 -cp2 -xde15:3


call :CONV col256xterm          -b8 -cp7

call :CONV col256xterm-xdp12    -b8 -cp7 -xdp12:0
call :CONV col256xterm-xdp12x2  -b8 -cp7 -xdp12:1
call :CONV col256xterm-xdp12x4  -b8 -cp7 -xdp12:2
call :CONV col256xterm-xdp12x8  -b8 -cp7 -xdp12:3

call :CONV col256xterm-xde12    -b8 -cp7 -xde12:0
call :CONV col256xterm-xde12x2  -b8 -cp7 -xde12:1
call :CONV col256xterm-xde12x4  -b8 -cp7 -xde12:2
call :CONV col256xterm-xde12x8  -b8 -cp7 -xde12:3

call :CONV col256xterm-xdp15    -b8 -cp7 -xdp15:0
call :CONV col256xterm-xdp15x2  -b8 -cp7 -xdp15:1
call :CONV col256xterm-xdp15x4  -b8 -cp7 -xdp15:2
call :CONV col256xterm-xdp15x8  -b8 -cp7 -xdp15:3

call :CONV col256xterm-xde15    -b8 -cp7 -xde15:0
call :CONV col256xterm-xde15x2  -b8 -cp7 -xde15:1
call :CONV col256xterm-xde15x4  -b8 -cp7 -xde15:2
call :CONV col256xterm-xde15x8  -b8 -cp7 -xde15:3

call :CONV col216xterm          -b8 -cp7 -cn216
call :CONV col216xterm-xdpx4    -b8 -cp7 -cn216 -xdp:2
call :CONV col216xterm-xde      -b8 -cp7 -cn216 -xde:0
call :CONV col216xterm-xdex4    -b8 -cp7 -cn216 -xde:2

call :CONV col064fix           -b6 -cp1
call :CONV col064fix-xdpx2     -b6 -cp1 -xdp:1
call :CONV col064fix-xdpx4     -b6 -cp1 -xdp:2
call :CONV col064fix-xdpx8     -b6 -cp1 -xdp:3

call :CONV col064fix-xde       -b6 -cp1 -xde:0
call :CONV col064fix-xdex2     -b6 -cp1 -xde:1
call :CONV col064fix-xdex4     -b6 -cp1 -xde:2
call :CONV col064fix-xdex8     -b6 -cp1 -xde:3

call :CONV col064fix-xdp12     -b6 -cp1 -xdp12:0
call :CONV col064fix-xdp12x2   -b6 -cp1 -xdp12:1
call :CONV col064fix-xdp12x4   -b6 -cp1 -xdp12:2
call :CONV col064fix-xdp12x8   -b6 -cp1 -xdp12:3

call :CONV col064fix-xde12     -b6 -cp1 -xde12:0
call :CONV col064fix-xde12x2   -b6 -cp1 -xde12:1
call :CONV col064fix-xde12x4   -b6 -cp1 -xde12:2
call :CONV col064fix-xde12x8   -b6 -cp1 -xde12:3

call :CONV col064xterm-xde12x4 -b8 -cp7 -cn64 -xde12:2
call :CONV col064win-xde12x4   -b8 -cp2 -cn64 -xde12:2

call :CONV col128fix           -b7 -cp8
call :CONV col128fix-xdp09     -b7 -cp8 -xdp9:0
call :CONV col128fix-xde09     -b7 -cp8 -xde9:0
call :CONV col128fix-xdp09x8   -b7 -cp8 -xdp9:3
call :CONV col128fix-xde09x8   -b7 -cp8 -xde9:3
call :CONV col128fix-xdp12x8   -b7 -cp8 -xdp12:3
call :CONV col128fix-xde12x8   -b7 -cp8 -xde12:3


call :CONV col256rgb5c40         -b8 -cp8
call :CONV col256rgb5c40-xdpx2   -b8 -cp8 -xdp:2
call :CONV col256rgb5c40-xde     -b8 -cp8 -xde:0
call :CONV col256rgb5c40-xdex2   -b8 -cp8 -xde:2

call :CONV col256rgb5c40-xdp12   -b8 -cp8 -xdp12:0
call :CONV col256rgb5c40-xdp12x2 -b8 -cp8 -xdp12:2
call :CONV col256rgb5c40-xde12   -b8 -cp8 -xde12:0
call :CONV col256rgb5c40-xde12x2 -b8 -cp8 -xde12:2

call :CONV col064sp            -b8 -cp8
call :CONV col064sp-xdpx8      -b8 -cp8 -xdp:3
call :CONV col064sp-xde        -b8 -cp8 -xde

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
