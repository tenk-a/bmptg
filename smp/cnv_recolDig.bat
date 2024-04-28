if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolDig
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col008jp3            -b3 -cp1

call :CONV col008win3           -b3 -cp2
call :CONV col016win4           -b4 -cp2

call :CONV col008jp3-xde        -b3 -cp1 -xde
call :CONV col008jp3-xdex2      -b3 -cp1 -xde:1
call :CONV col008jp3-xdex4      -b3 -cp1 -xde:2
call :CONV col008jp3-xdex8      -b3 -cp1 -xde:3
call :CONV col008jp3-xdpx2      -b3 -cp1 -xdp:1
call :CONV col008jp3-xdpx4      -b3 -cp1 -xdp:2
call :CONV col008jp3-xdpx8      -b3 -cp1 -xdp:3
call :CONV col008jp3-xdo09x2    -b3 -cp1 -xdo09:1
call :CONV col008jp3-xdo09x4    -b3 -cp1 -xdo09:2
call :CONV col008jp3-xdo09x8    -b3 -cp1 -xdo09:3
call :CONV col008jp3-xdo12x2    -b3 -cp1 -xdo12:1
call :CONV col008jp3-xdo12x4    -b3 -cp1 -xdo12:2
call :CONV col008jp3-xdo12x8    -b3 -cp1 -xdo12:3
call :CONV col008jp3-xdo15x2    -b3 -cp1 -xdo15:1
call :CONV col008jp3-xdo15x4    -b3 -cp1 -xdo15:2
call :CONV col008jp3-xdo15x8    -b3 -cp1 -xdo15:3

call :CONV m2col008jp3          -b3 -cp1         -cpm2
call :CONV m2col008jp3-xde      -b3 -cp1 -xde    -cpm2
call :CONV m2col008jp3-xdo09x8  -b3 -cp1 -xdo9:3 -cpm2
call :CONV m2col008jp3-xdpx8    -b3 -cp1 -xdp:3  -cpm2

call :CONV col016win4            -b4 -cp2
call :CONV col016win4-xde        -b4 -cp2 -xde
call :CONV col016win4-xdex2      -b4 -cp2 -xde:1
call :CONV col016win4-xdex4      -b4 -cp2 -xde:2
call :CONV col016win4-xdex8      -b4 -cp2 -xde:3
call :CONV col016win4-xdpx2      -b4 -cp2 -xdp:1
call :CONV col016win4-xdpx4      -b4 -cp2 -xdp:2
call :CONV col016win4-xdpx8      -b4 -cp2 -xdp:3
call :CONV col016win4-xdo09x2    -b4 -cp2 -xdo09:1
call :CONV col016win4-xdo09x4    -b4 -cp2 -xdo09:2
call :CONV col016win4-xdo09x8    -b4 -cp2 -xdo09:3
call :CONV col016win4-xdo12x2    -b4 -cp2 -xdo12:1
call :CONV col016win4-xdo12x4    -b4 -cp2 -xdo12:2
call :CONV col016win4-xdo12x8    -b4 -cp2 -xdo12:3
call :CONV col016win4-xdo15x2    -b4 -cp2 -xdo15:1
call :CONV col016win4-xdo15x4    -b4 -cp2 -xdo15:2
call :CONV col016win4-xdo15x8    -b4 -cp2 -xdo15:3

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% :bmp -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
