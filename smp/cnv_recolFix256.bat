if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolFix256
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col256jp            -b8 -cp1
call :CONV col256jp-xdpx2      -b8 -cp1 -xdp:1
call :CONV col256jp-xdpx4      -b8 -cp1 -xdp:2
call :CONV col256jp-xdpx8      -b8 -cp1 -xdp:3

call :CONV col256jp-xdh        -b8 -cp1 -xdh:0
call :CONV col256jp-xdhx2      -b8 -cp1 -xdh:1
call :CONV col256jp-xdhx4      -b8 -cp1 -xdh:2
call :CONV col256jp-xdhx8      -b8 -cp1 -xdh:3

call :CONV col256jp-xde        -b8 -cp1 -xde:0
call :CONV col256jp-xdex2      -b8 -cp1 -xde:1
call :CONV col256jp-xdex4      -b8 -cp1 -xde:2
call :CONV col256jp-xdex8      -b8 -cp1 -xde:3

call :CONV col256jp-xdp12      -b8 -cp1 -xdp12
call :CONV col256jp-xdp12x2    -b8 -cp1 -xdp12:1
call :CONV col256jp-xdp12x4    -b8 -cp1 -xdp12:2
call :CONV col256jp-xdp12x8    -b8 -cp1 -xdp12:3

call :CONV col256jp-xdh12      -b8 -cp1 -xdh12:0
call :CONV col256jp-xdh12x2    -b8 -cp1 -xdh12:1
call :CONV col256jp-xdh12x4    -b8 -cp1 -xdh12:2
call :CONV col256jp-xdh12x8    -b8 -cp1 -xdh12:3

call :CONV col256jp-xde12      -b8 -cp1 -xde12:0
call :CONV col256jp-xde12x2    -b8 -cp1 -xde12:1
call :CONV col256jp-xde12x4    -b8 -cp1 -xde12:2
call :CONV col256jp-xde12x8    -b8 -cp1 -xde12:3


call :CONV col256win            -b8 -cp2
::call :CONV col256win-xdpx2      -b8 -cp2 -xdp:1
call :CONV col256win-xdpx4      -b8 -cp2 -xdp:2
::call :CONV col256win-xdpx8      -b8 -cp2 -xdp:3

call :CONV col256win-xdh        -b8 -cp2 -xdh:0
::call :CONV col256win-xdhx2      -b8 -cp2 -xdh:1
call :CONV col256win-xdhx4      -b8 -cp2 -xdh:2
::call :CONV col256win-xdhx8      -b8 -cp2 -xdh:3

call :CONV col256win-xde        -b8 -cp2 -xde:0
::call :CONV col256win-xdex2      -b8 -cp2 -xde:1
call :CONV col256win-xdex4      -b8 -cp2 -xde:2
::call :CONV col256win-xdex8      -b8 -cp2 -xde:3

call :CONV col256win-xdp15      -b8 -cp2 -xdp15
::call :CONV col256win-xdp15x2    -b8 -cp2 -xdp15:1
call :CONV col256win-xdp15x4    -b8 -cp2 -xdp15:2
::call :CONV col256win-xdp15x8    -b8 -cp2 -xdp15:3

call :CONV col256win-xdh15      -b8 -cp2 -xdh15:0
::call :CONV col256win-xdh15x2    -b8 -cp2 -xdh15:1
call :CONV col256win-xdh15x4    -b8 -cp2 -xdh15:2
::call :CONV col256win-xdh15x8    -b8 -cp2 -xdh15:3

call :CONV col256win-xde15      -b8 -cp2 -xde15:0
::call :CONV col256win-xde15x2    -b8 -cp2 -xde15:1
call :CONV col256win-xde15x4    -b8 -cp2 -xde15:2
::call :CONV col256win-xde15x8    -b8 -cp2 -xde15:3


call :CONV col256xterm          -b8 -cp7
::call :CONV col256xterm-xdpx2    -b8 -cp7 -xdp:1
call :CONV col256xterm-xdpx4    -b8 -cp7 -xdp:2
::call :CONV col256xterm-xdpx8    -b8 -cp7 -xdp:3

call :CONV col256xterm-xdh      -b8 -cp7 -xdh:0
::call :CONV col256xterm-xdhx2    -b8 -cp7 -xdh:1
call :CONV col256xterm-xdhx4    -b8 -cp7 -xdh:2
::call :CONV col256xterm-xdhx8    -b8 -cp7 -xdh:3

call :CONV col256xterm-xde      -b8 -cp7 -xde:0
::call :CONV col256xterm-xdex2    -b8 -cp7 -xde:1
call :CONV col256xterm-xdex4    -b8 -cp7 -xde:2
::call :CONV col256xterm-xdex8    -b8 -cp7 -xde:3

call :CONV col256xterm-xdp15    -b8 -cp7 -xdp15
::call :CONV col256xterm-xdp15x2  -b8 -cp7 -xdp15:1
call :CONV col256xterm-xdp15x4  -b8 -cp7 -xdp15:2
::call :CONV col256xterm-xdp15x8  -b8 -cp7 -xdp15:3

call :CONV col256xterm-xdh15    -b8 -cp7 -xdh15:0
::call :CONV col256xterm-xdh15x2  -b8 -cp7 -xdh15:1
call :CONV col256xterm-xdh15x4  -b8 -cp7 -xdh15:2
::call :CONV col256xterm-xdh15x8  -b8 -cp7 -xdh15:3

call :CONV col256xterm-xde15    -b8 -cp7 -xde15:0
::call :CONV col256xterm-xde15x2  -b8 -cp7 -xde15:1
call :CONV col256xterm-xde15x4  -b8 -cp7 -xde15:2
::call :CONV col256xterm-xde15x8  -b8 -cp7 -xde15:3

call :CONV col216xterm          -b8 -cp7 -cn216
call :CONV col216xterm-xdpx4    -b8 -cp7 -cn216 -xdp:2
call :CONV col216xterm-xdh      -b8 -cp7 -cn216 -xdh:0
call :CONV col216xterm-xdhx4    -b8 -cp7 -cn216 -xdh:2
call :CONV col216xterm-xde      -b8 -cp7 -cn216 -xde:0
call :CONV col216xterm-xdex4    -b8 -cp7 -cn216 -xde:2


call :CONV col128fix           -b7 -cp1
call :CONV col128fix-xde       -b7 -cp1 -xde9
call :CONV col128fix-xde09x8   -b7 -cp1 -xde9:3
call :CONV col128fix-xdp09x8   -b7 -cp1 -xdp9:3
call :CONV col128fix-xdh09x8   -b7 -cp1 -xdh9:3
call :CONV col128fix-xde12x8   -b7 -cp1 -xde12:3
call :CONV col128fix-xdp12x8   -b7 -cp1 -xdp12:3
call :CONV col128fix-xdh12x8   -b7 -cp1 -xdh12:3


call :CONV col064fix           -b6 -cp1
::call :CONV col064fix-xdpx2     -b6 -cp1 -xdp:1
::call :CONV col064fix-xdpx4     -b6 -cp1 -xdp:2
call :CONV col064fix-xdpx8     -b6 -cp1 -xdp:3

call :CONV col064fix-xdh       -b6 -cp1 -xdh:0
::call :CONV col064fix-xdhx2     -b6 -cp1 -xdh:1
::call :CONV col064fix-xdhx4     -b6 -cp1 -xdh:2
call :CONV col064fix-xdhx8     -b6 -cp1 -xdh:3

call :CONV col064fix-xde       -b6 -cp1 -xde:0
::call :CONV col064fix-xdex2     -b6 -cp1 -xde:1
::call :CONV col064fix-xdex4     -b6 -cp1 -xde:2
call :CONV col064fix-xdex8     -b6 -cp1 -xde:3

call :CONV col064fix-xdp12     -b6 -cp1 -xdp12
::call :CONV col064fix-xdp12x2   -b6 -cp1 -xdp12:1
::call :CONV col064fix-xdp12x4   -b6 -cp1 -xdp12:2
call :CONV col064fix-xdp12x8   -b6 -cp1 -xdp12:3

call :CONV col064fix-xdh12     -b6 -cp1 -xdh12:0
::call :CONV col064fix-xdh12x2   -b6 -cp1 -xdh12:1
::call :CONV col064fix-xdh12x4   -b6 -cp1 -xdh12:2
call :CONV col064fix-xdh12x8   -b6 -cp1 -xdh12:3

call :CONV col064fix-xde12     -b6 -cp1 -xde12:0
::call :CONV col064fix-xde12x2   -b6 -cp1 -xde12:1
::call :CONV col064fix-xde12x4   -b6 -cp1 -xde12:2
call :CONV col064fix-xde12x8   -b6 -cp1 -xde12:3


call :CONV col256rgb5c40         -b8 -cp8
call :CONV col256rgb5c40-xdpx2   -b8 -cp8 -xdp:2
call :CONV col256rgb5c40-xdh     -b8 -cp8 -xdh
call :CONV col256rgb5c40-xdhx2   -b8 -cp8 -xdh:2
call :CONV col256rgb5c40-xde     -b8 -cp8 -xde
call :CONV col256rgb5c40-xdex2   -b8 -cp8 -xde:2

call :CONV col256rgb5c40-xdp12x2 -b8 -cp8 -xdp12:2
call :CONV col256rgb5c40-xdh12   -b8 -cp8 -xdh12
call :CONV col256rgb5c40-xdh12x2 -b8 -cp8 -xdh12:2
call :CONV col256rgb5c40-xde12   -b8 -cp8 -xde12
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
