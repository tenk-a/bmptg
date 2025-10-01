if "%BmpTgExe%"=="" set BmpTgExe=..\bin\exe64\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolDig
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col004jp2            -b2 -cp1

call :CONV col004jp2-xdp-x      -b2 -cp1 -xdpx:0
call :CONV col004jp2-xdpx2-x    -b2 -cp1 -xdpx:1
call :CONV col004jp2-xdpx4-x    -b2 -cp1 -xdpx:2
call :CONV col004jp2-xdpx8-x    -b2 -cp1 -xdpx:3

call :CONV col004jp2-xde-x      -b2 -cp1 -xdex:0
call :CONV col004jp2-xdex2-x    -b2 -cp1 -xdex:1
call :CONV col004jp2-xdex4-x    -b2 -cp1 -xdex:2
call :CONV col004jp2-xdex8-x    -b2 -cp1 -xdex:3

call :CONV col004jp2-xdp-y      -b2 -cp1 -xdpy:0
call :CONV col004jp2-xdpx2-y    -b2 -cp1 -xdpy:1
call :CONV col004jp2-xdpx4-y    -b2 -cp1 -xdpy:2
call :CONV col004jp2-xdpx8-y    -b2 -cp1 -xdpy:3

call :CONV col004jp2-xde-y      -b2 -cp1 -xdey:0
call :CONV col004jp2-xdex2-y    -b2 -cp1 -xdey:1
call :CONV col004jp2-xdex4-y    -b2 -cp1 -xdey:2
call :CONV col004jp2-xdex8-y    -b2 -cp1 -xdey:3

call :CONV col004jp2-xdp-xy      -b2 -cp1 -xdpxy:0
call :CONV col004jp2-xdpx2-xy    -b2 -cp1 -xdpxy:1
call :CONV col004jp2-xdpx4-xy    -b2 -cp1 -xdpxy:2
call :CONV col004jp2-xdpx8-xy    -b2 -cp1 -xdpxy:3

call :CONV col004jp2-xde-xy      -b2 -cp1 -xdexy:0
call :CONV col004jp2-xdex2-xy    -b2 -cp1 -xdexy:1
call :CONV col004jp2-xdex4-xy    -b2 -cp1 -xdexy:2
call :CONV col004jp2-xdex8-xy    -b2 -cp1 -xdexy:3

call :CONV col008win3           -b3 -cp2

call :CONV col008jp3            -b3 -cp1

call :CONV col008jp3-xde        -b3 -cp1 -xde:0
call :CONV col008jp3-xdex2      -b3 -cp1 -xde:1
call :CONV col008jp3-xdex4      -b3 -cp1 -xde:2
call :CONV col008jp3-xdex8      -b3 -cp1 -xde:3

call :CONV col008jp3-xdp        -b3 -cp1 -xdp:0
call :CONV col008jp3-xdpx2      -b3 -cp1 -xdp:1
call :CONV col008jp3-xdpx4      -b3 -cp1 -xdp:2
call :CONV col008jp3-xdpx8      -b3 -cp1 -xdp:3

rem 09,12,15 shitei sitemo chigai ga nakatta.
::call :CONV col008jp3-xde09      -b3 -cp1 -xde09:0
::call :CONV col008jp3-xde09x2    -b3 -cp1 -xde09:1
::call :CONV col008jp3-xde09x4    -b3 -cp1 -xde09:2
::call :CONV col008jp3-xde09x8    -b3 -cp1 -xde09:3
::call :CONV col008jp3-xde12      -b3 -cp1 -xde12:0
::call :CONV col008jp3-xde12x2    -b3 -cp1 -xde12:1
::call :CONV col008jp3-xde12x4    -b3 -cp1 -xde12:2
::call :CONV col008jp3-xde12x8    -b3 -cp1 -xde12:3
::call :CONV col008jp3-xde15      -b3 -cp1 -xde15:0
::call :CONV col008jp3-xde15x2    -b3 -cp1 -xde15:1
::call :CONV col008jp3-xde15x4    -b3 -cp1 -xde15:2
::call :CONV col008jp3-xde15x8    -b3 -cp1 -xde15:3
::call :CONV col008jp3-xdp09      -b3 -cp1 -xdp09:0
::call :CONV col008jp3-xdp09x2    -b3 -cp1 -xdp09:1
::call :CONV col008jp3-xdp09x4    -b3 -cp1 -xdp09:2
::call :CONV col008jp3-xdp09x8    -b3 -cp1 -xdp09:3
::call :CONV col008jp3-xdp12      -b3 -cp1 -xdp12:0
::call :CONV col008jp3-xdp12x2    -b3 -cp1 -xdp12:1
::call :CONV col008jp3-xdp12x4    -b3 -cp1 -xdp12:2
::call :CONV col008jp3-xdp12x8    -b3 -cp1 -xdp12:3
::call :CONV col008jp3-xdp15      -b3 -cp1 -xdp15:0
::call :CONV col008jp3-xdp15x2    -b3 -cp1 -xdp15:1
::call :CONV col008jp3-xdp15x4    -b3 -cp1 -xdp15:2
::call :CONV col008jp3-xdp15x8    -b3 -cp1 -xdp15:3

call :CONV col008jp3m2          -b3 -cp1         -cpm2
call :CONV col008jp3m2-xdp      -b3 -cp1 -xdp:0  -cpm2
call :CONV col008jp3m2-xde      -b3 -cp1 -xde:0  -cpm2
call :CONV col008jp3m2-xde09x8  -b3 -cp1 -xde9:3 -cpm2
call :CONV col008jp3m2-xdp09x8  -b3 -cp1 -xdp9:3 -cpm2

call :CONV col016jp4             -b4 -cp1
call :CONV col016win4            -b4 -cp2
call :CONV col016win4-xdp        -b4 -cp2 -xdp:0
call :CONV col016win4-xdpx2      -b4 -cp2 -xdp:1
call :CONV col016win4-xdpx4      -b4 -cp2 -xdp:2
call :CONV col016win4-xdpx8      -b4 -cp2 -xdp:3

call :CONV col016win4-xde        -b4 -cp2 -xde:0
call :CONV col016win4-xdex2      -b4 -cp2 -xde:1
call :CONV col016win4-xdex4      -b4 -cp2 -xde:2
call :CONV col016win4-xdex8      -b4 -cp2 -xde:3

call :CONV col004jp2            -b2 -cp1
call :CONV col004jp2-xdp        -b2 -cp1 -xdp:0
call :CONV col004jp2-xdpx2      -b2 -cp1 -xdp:1
call :CONV col004jp2-xdpx4      -b2 -cp1 -xdp:2
call :CONV col004jp2-xdpx8      -b2 -cp1 -xdp:3

call :CONV col004jp2-xde        -b2 -cp1 -xde:0
call :CONV col004jp2-xdex2      -b2 -cp1 -xde:1
call :CONV col004jp2-xdex4      -b2 -cp1 -xde:2
call :CONV col004jp2-xdex8      -b2 -cp1 -xde:3

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% :bmp -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
