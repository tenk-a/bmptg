if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recoldig
set CMN_OPTS=:bmp -s%SRCDIR% -b8 -if -cpm0
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col008jp3            -b3 -cp1
call :CONV col016jp4            -b4 -cp1

call :CONV col008jp3-m2         -b3 -cp1        -cpm2
call :CONV col008jp3-xdo09x8-m2 -b3 -cp1 -xd9:3 -cpm2
call :CONV col008jp3-xdp09x8-m2 -b3 -cp1 -xdp9:3 -cpm2

call :CONV col008jp3-xdo06x8    -b3 -cp1 -xd6:3
call :CONV col008jp3-xdp06x8    -b3 -cp1 -xdp6:3
call :CONV col008win3-xd06x8-m2 -b3 -cp2 -xd6:3 -cpm2

call :CONV col008jp3-xde        -b3 -cp1 -xde
call :CONV col008jp3-xde2       -b3 -cp1 -xde2
call :CONV col008jp4-xde        -b4 -cp1 -xde
call :CONV col008jp4-xde2       -b4 -cp1 -xde2

call :CONV col016win4           -b4 -cp2
call :CONV col016win4-xdo09x8   -b4 -cp2 -xd9:3
call :CONV col016win4-xdp09x8   -b4 -cp2 -xdp9:3

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% :bmp -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
