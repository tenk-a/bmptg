if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recol256
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col002              -b1
call :CONV col002-xdpx2        -b1 -xdp:1
call :CONV col002-xdpx4        -b1 -xdp:2
call :CONV col002-xdpx8        -b1 -xdp:3
call :CONV col002-xdh          -b1 -xdh
call :CONV col002-xdhx2        -b1 -xdh:1
call :CONV col002-xdhx4        -b1 -xdh:2
call :CONV col002-xdhx8        -b1 -xdh:3
call :CONV col002-xde          -b1 -xde
call :CONV col002-xdex2        -b1 -xde:1
call :CONV col002-xdex4        -b1 -xde:2
call :CONV col002-xdex8        -b1 -xde:3

call :CONV col002-xdp09        -b1 -xdh09:0
call :CONV col002-xdp09x2      -b1 -xdp09:1
call :CONV col002-xdp09x4      -b1 -xdp09:2
call :CONV col002-xdp09x8      -b1 -xdp09:3
call :CONV col002-xdh09        -b1 -xdh09:0
call :CONV col002-xdh09x2      -b1 -xdh09:1
call :CONV col002-xdh09x4      -b1 -xdh09:2
call :CONV col002-xdh09x8      -b1 -xdh09:3
call :CONV col002-xde09        -b1 -xde09:0
call :CONV col002-xde09x2      -b1 -xde09:1
call :CONV col002-xde09x4      -b1 -xde09:2
call :CONV col002-xde09x8      -b1 -xde09:3


call :CONV col004              -b2
call :CONV col004-xdpx2        -b2 -xdp:1
call :CONV col004-xdpx4        -b2 -xdp:2
call :CONV col004-xdpx8        -b2 -xdp:3
call :CONV col004-xdh          -b2 -xdh
call :CONV col004-xdhx2        -b2 -xdh:1
call :CONV col004-xdhx4        -b2 -xdh:2
call :CONV col004-xdhx8        -b2 -xdh:3
call :CONV col004-xde          -b2 -xde
call :CONV col004-xdex2        -b2 -xde:1
call :CONV col004-xdex4        -b2 -xde:2
call :CONV col004-xdex8        -b2 -xde:3

call :CONV col004-xdp09        -b2 -xdh09:0
call :CONV col004-xdp09x2      -b2 -xdp09:1
call :CONV col004-xdp09x4      -b2 -xdp09:2
call :CONV col004-xdp09x8      -b2 -xdp09:3
call :CONV col004-xdh09        -b2 -xdh09:0
call :CONV col004-xdh09x2      -b2 -xdh09:1
call :CONV col004-xdh09x4      -b2 -xdh09:2
call :CONV col004-xdh09x8      -b2 -xdh09:3
call :CONV col004-xde09        -b2 -xde09:0
call :CONV col004-xde09x2      -b2 -xde09:1
call :CONV col004-xde09x4      -b2 -xde09:2
call :CONV col004-xde09x8      -b2 -xde09:3


call :CONV col016mcyuv         -b4         -cp3
call :CONV col016mcyuv-xdpx2   -b4 -xdp:1  -cp3
call :CONV col016mcyuv-xdpx4   -b4 -xdp:2  -cp3
call :CONV col016mcyuv-xdpx8   -b4 -xdp:3  -cp3
call :CONV col016mcyuv-xdh     -b4 -xdh    -cp3
call :CONV col016mcyuv-xdhx2   -b4 -xdh:1  -cp3
call :CONV col016mcyuv-xdhx4   -b4 -xdh:2  -cp3
call :CONV col016mcyuv-xdhx8   -b4 -xdh:3  -cp3
call :CONV col016mcyuv-xde     -b4 -xde    -cp3
call :CONV col016mcyuv-xdex2   -b4 -xde:1  -cp3
call :CONV col016mcyuv-xdex4   -b4 -xde:2  -cp3
call :CONV col016mcyuv-xdex8   -b4 -xde:3  -cp3

call :CONV col016mcyuv-xdp09   -b4 -xdh09:0 -cp3
call :CONV col016mcyuv-xdp09x2 -b4 -xdp09:1 -cp3
call :CONV col016mcyuv-xdp09x4 -b4 -xdp09:2 -cp3
call :CONV col016mcyuv-xdp09x8 -b4 -xdp09:3 -cp3
call :CONV col016mcyuv-xdh09   -b4 -xdh09:0 -cp3
call :CONV col016mcyuv-xdh09x2 -b4 -xdh09:1 -cp3
call :CONV col016mcyuv-xdh09x4 -b4 -xdh09:2 -cp3
call :CONV col016mcyuv-xdh09x8 -b4 -xdh09:3 -cp3
call :CONV col016mcyuv-xde09   -b4 -xde09:0 -cp3
call :CONV col016mcyuv-xde09x2 -b4 -xde09:1 -cp3
call :CONV col016mcyuv-xde09x4 -b4 -xde09:2 -cp3
call :CONV col016mcyuv-xde09x8 -b4 -xde09:3 -cp3

call :CONV col016mcrgb         -b4         -cp4
call :CONV col016mcrgb-xdpx2   -b4 -xdp:1  -cp4
call :CONV col016mcrgb-xdpx4   -b4 -xdp:2  -cp4
call :CONV col016mcrgb-xdpx8   -b4 -xdp:3  -cp4
call :CONV col016mcrgb-xdh     -b4 -xdh    -cp4
call :CONV col016mcrgb-xdhx2   -b4 -xdh:1  -cp4
call :CONV col016mcrgb-xdhx4   -b4 -xdh:2  -cp4
call :CONV col016mcrgb-xdhx8   -b4 -xdh:3  -cp4
call :CONV col016mcrgb-xde     -b4 -xde    -cp4
call :CONV col016mcrgb-xdex2   -b4 -xde:1  -cp4
call :CONV col016mcrgb-xdex4   -b4 -xde:2  -cp4
call :CONV col016mcrgb-xdex8   -b4 -xde:3  -cp4

call :CONV col016mcrgb-xdp09   -b4 -xdh09:0 -cp4
call :CONV col016mcrgb-xdp09x2 -b4 -xdp09:1 -cp4
call :CONV col016mcrgb-xdp09x4 -b4 -xdp09:2 -cp4
call :CONV col016mcrgb-xdp09x8 -b4 -xdp09:3 -cp4
call :CONV col016mcrgb-xdh09   -b4 -xdh09:0 -cp4
call :CONV col016mcrgb-xdh09x2 -b4 -xdh09:1 -cp4
call :CONV col016mcrgb-xdh09x4 -b4 -xdh09:2 -cp4
call :CONV col016mcrgb-xdh09x8 -b4 -xdh09:3 -cp4
call :CONV col016mcrgb-xde09   -b4 -xde09:0 -cp4
call :CONV col016mcrgb-xde09x2 -b4 -xde09:1 -cp4
call :CONV col016mcrgb-xde09x4 -b4 -xde09:2 -cp4
call :CONV col016mcrgb-xde09x8 -b4 -xde09:3 -cp4

call :CONV col016hist          -b4         -cp5
call :CONV col016hist-xdpx2    -b4 -xdp:1  -cp5
call :CONV col016hist-xdpx4    -b4 -xdp:2  -cp5
call :CONV col016hist-xdpx8    -b4 -xdp:3  -cp5
call :CONV col016hist-xdh      -b4 -xdh    -cp5
call :CONV col016hist-xdhx2    -b4 -xdh:1  -cp5
call :CONV col016hist-xdhx4    -b4 -xdh:2  -cp5
call :CONV col016hist-xdhx8    -b4 -xdh:3  -cp5
call :CONV col016hist-xde      -b4 -xde    -cp5
call :CONV col016hist-xdex2    -b4 -xde:1  -cp5
call :CONV col016hist-xdex4    -b4 -xde:2  -cp5
call :CONV col016hist-xdex8    -b4 -xde:3  -cp5

call :CONV col016hist-xdp09    -b4 -xdh09:0 -cp5
call :CONV col016hist-xdp09x2  -b4 -xdp09:1 -cp5
call :CONV col016hist-xdp09x4  -b4 -xdp09:2 -cp5
call :CONV col016hist-xdp09x8  -b4 -xdp09:3 -cp5
call :CONV col016hist-xdh09    -b4 -xdh09:0 -cp5
call :CONV col016hist-xdh09x2  -b4 -xdh09:1 -cp5
call :CONV col016hist-xdh09x4  -b4 -xdh09:2 -cp5
call :CONV col016hist-xdh09x8  -b4 -xdh09:3 -cp5
call :CONV col016hist-xde09    -b4 -xde09:0 -cp5
call :CONV col016hist-xde09x2  -b4 -xde09:1 -cp5
call :CONV col016hist-xde09x4  -b4 -xde09:2 -cp5
call :CONV col016hist-xde09x8  -b4 -xde09:3 -cp5

call :CONV col256mcyuv         -b8         -cp3
call :CONV col256mcyuv-xdpx2   -b8 -xdp:1  -cp3
call :CONV col256mcyuv-xdpx4   -b8 -xdp:2  -cp3
call :CONV col256mcyuv-xdpx8   -b8 -xdp:3  -cp3
call :CONV col256mcyuv-xdh     -b8 -xdh    -cp3
call :CONV col256mcyuv-xdhx2   -b8 -xdh:1  -cp3
call :CONV col256mcyuv-xdhx4   -b8 -xdh:2  -cp3
call :CONV col256mcyuv-xdhx8   -b8 -xdh:3  -cp3
call :CONV col256mcyuv-xde     -b8 -xde    -cp3
call :CONV col256mcyuv-xdex2   -b8 -xde:1  -cp3
call :CONV col256mcyuv-xdex4   -b8 -xde:2  -cp3
call :CONV col256mcyuv-xdex8   -b8 -xde:3  -cp3

call :CONV col256mcyuv-xdp09   -b8 -xdh09:0 -cp3
call :CONV col256mcyuv-xdp09x2 -b8 -xdp09:1 -cp3
call :CONV col256mcyuv-xdp09x4 -b8 -xdp09:2 -cp3
call :CONV col256mcyuv-xdp09x8 -b8 -xdp09:3 -cp3
call :CONV col256mcyuv-xdh09   -b8 -xdh09:0 -cp3
call :CONV col256mcyuv-xdh09x2 -b8 -xdh09:1 -cp3
call :CONV col256mcyuv-xdh09x4 -b8 -xdh09:2 -cp3
call :CONV col256mcyuv-xdh09x8 -b8 -xdh09:3 -cp3
call :CONV col256mcyuv-xde09   -b8 -xde09:0 -cp3
call :CONV col256mcyuv-xde09x2 -b8 -xde09:1 -cp3
call :CONV col256mcyuv-xde09x4 -b8 -xde09:2 -cp3
call :CONV col256mcyuv-xde09x8 -b8 -xde09:3 -cp3

call :CONV col256mcrgb         -b8         -cp4
call :CONV col256mcrgb-xdpx2   -b8 -xdp:1  -cp4
call :CONV col256mcrgb-xdpx4   -b8 -xdp:2  -cp4
::call :CONV col256mcrgb-xdpx8   -b8 -xdp:3  -cp4
call :CONV col256mcrgb-xdh     -b8 -xdh    -cp4
call :CONV col256mcrgb-xdhx2   -b8 -xdh:1  -cp4
call :CONV col256mcrgb-xdhx4   -b8 -xdh:2  -cp4
::call :CONV col256mcrgb-xdhx8   -b8 -xdh:3  -cp4
call :CONV col256mcrgb-xde     -b8 -xde    -cp4
call :CONV col256mcrgb-xdex2   -b8 -xde:1  -cp4
call :CONV col256mcrgb-xdex4   -b8 -xde:2  -cp4
::call :CONV col256mcrgb-xdex8   -b8 -xde:3  -cp4

call :CONV col256mcrgb-xdp09   -b8 -xdh09:0 -cp4
call :CONV col256mcrgb-xdp09x2 -b8 -xdp09:1 -cp4
call :CONV col256mcrgb-xdp09x4 -b8 -xdp09:2 -cp4
::call :CONV col256mcrgb-xdp09x8 -b8 -xdp09:3 -cp4
call :CONV col256mcrgb-xdh09   -b8 -xdh09:0 -cp4
call :CONV col256mcrgb-xdh09x2 -b8 -xdh09:1 -cp4
call :CONV col256mcrgb-xdh09x4 -b8 -xdh09:2 -cp4
::call :CONV col256mcrgb-xdh09x8 -b8 -xdh09:3 -cp4
call :CONV col256mcrgb-xde09   -b8 -xde09:0 -cp4
call :CONV col256mcrgb-xde09x2 -b8 -xde09:1 -cp4
call :CONV col256mcrgb-xde09x4 -b8 -xde09:2 -cp4
::call :CONV col256mcrgb-xde09x8 -b8 -xde09:3 -cp4

call :CONV col256hist          -b8         -cp5
call :CONV col256hist-xdpx2    -b8 -xdp:1  -cp5
call :CONV col256hist-xdpx4    -b8 -xdp:2  -cp5
::call :CONV col256hist-xdpx8    -b8 -xdp:3  -cp5
call :CONV col256hist-xdh      -b8 -xdh    -cp5
call :CONV col256hist-xdhx2    -b8 -xdh:1  -cp5
call :CONV col256hist-xdhx4    -b8 -xdh:2  -cp5
::call :CONV col256hist-xdhx8    -b8 -xdh:3  -cp5
call :CONV col256hist-xde      -b8 -xde    -cp5
call :CONV col256hist-xdex2    -b8 -xde:1  -cp5
call :CONV col256hist-xdex4    -b8 -xde:2  -cp5
::call :CONV col256hist-xdex8    -b8 -xde:3  -cp5

call :CONV col256hist-xdp09    -b8 -xdh09:0 -cp5
call :CONV col256hist-xdp09x2  -b8 -xdp09:1 -cp5
call :CONV col256hist-xdp09x4  -b8 -xdp09:2 -cp5
::call :CONV col256hist-xdp09x8  -b8 -xdp09:3 -cp5
call :CONV col256hist-xdh09    -b8 -xdh09:0 -cp5
call :CONV col256hist-xdh09x2  -b8 -xdh09:1 -cp5
call :CONV col256hist-xdh09x4  -b8 -xdh09:2 -cp5
::call :CONV col256hist-xdh09x8  -b8 -xdh09:3 -cp5
call :CONV col256hist-xde09    -b8 -xde09:0 -cp5
call :CONV col256hist-xde09x2  -b8 -xde09:1 -cp5
call :CONV col256hist-xde09x4  -b8 -xde09:2 -cp5
::call :CONV col256hist-xde09x8  -b8 -xde09:3 -cp5

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
