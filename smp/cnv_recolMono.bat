if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolMono
set CMN_OPTS=:bmp -s%SRCDIR% -if
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV mono003jp3         -cg -b3 -cp1 -cn3
call :CONV mono003jp3-xdp     -cg -b3 -cp1 -cn3 -xdp:0
call :CONV mono003jp3-xdpx2   -cg -b3 -cp1 -cn3 -xdp:1
call :CONV mono003jp3-xdpx4   -cg -b3 -cp1 -cn3 -xdp:2
call :CONV mono003jp3-xdpx8   -cg -b3 -cp1 -cn3 -xdp:3
call :CONV mono003jp3-xde     -cg -b3 -cp1 -cn3 -xde
call :CONV mono003jp3-xdex2   -cg -b3 -cp1 -cn3 -xde:1
call :CONV mono003jp3-xdex4   -cg -b3 -cp1 -cn3 -xde:2
call :CONV mono003jp3-xdex8   -cg -b3 -cp1 -cn3 -xde:3

call :CONV mono004jp3         -cg -b3 -cp1 -cn4
call :CONV mono004jp3-xdp     -cg -b3 -cp1 -cn4 -xdp:0
call :CONV mono004jp3-xdpx2   -cg -b3 -cp1 -cn4 -xdp:1
call :CONV mono004jp3-xdpx4   -cg -b3 -cp1 -cn4 -xdp:2
call :CONV mono004jp3-xdpx8   -cg -b3 -cp1 -cn4 -xdp:3
call :CONV mono004jp3-xde     -cg -b3 -cp1 -cn4 -xde
call :CONV mono004jp3-xdex2   -cg -b3 -cp1 -cn4 -xde:1
call :CONV mono004jp3-xdex4   -cg -b3 -cp1 -cn4 -xde:2
call :CONV mono004jp3-xdex8   -cg -b3 -cp1 -cn4 -xde:3

call :CONV mono003win         -cg -b4 -cp2 -cn3
call :CONV mono003win-xdpx2   -cg -b4 -cp2 -cn3 -xdp:1
call :CONV mono003win-xdpx4   -cg -b4 -cp2 -cn3 -xdp:2
call :CONV mono003win-xdpx8   -cg -b4 -cp2 -cn3 -xdp:3
call :CONV mono003win-xde     -cg -b4 -cp2 -cn3 -xde
call :CONV mono003win-xdex2   -cg -b4 -cp2 -cn3 -xde:1
call :CONV mono003win-xdex4   -cg -b4 -cp2 -cn3 -xde:2
call :CONV mono003win-xdex8   -cg -b4 -cp2 -cn3 -xde:3

call :CONV mono004win         -cg -b4 -cp2 -cn4
call :CONV mono004win-xdpx2   -cg -b4 -cp2 -cn4 -xdp:1
call :CONV mono004win-xdpx4   -cg -b4 -cp2 -cn4 -xdp:2
call :CONV mono004win-xdpx8   -cg -b4 -cp2 -cn4 -xdp:3
call :CONV mono004win-xde     -cg -b4 -cp2 -cn4 -xde
call :CONV mono004win-xdex2   -cg -b4 -cp2 -cn4 -xde:1
call :CONV mono004win-xdex4   -cg -b4 -cp2 -cn4 -xde:2
call :CONV mono004win-xdex8   -cg -b4 -cp2 -cn4 -xde:3


call :CONV mono002          -cg -b1
call :CONV mono002-xdpx2    -cg -b1 -xdp:1
call :CONV mono002-xdpx4    -cg -b1 -xdp:2
call :CONV mono002-xdpx8    -cg -b1 -xdp:3
call :CONV mono002-xde      -cg -b1 -xde
call :CONV mono002-xdex2    -cg -b1 -xde:1
call :CONV mono002-xdex4    -cg -b1 -xde:2
call :CONV mono002-xdex8    -cg -b1 -xde:3

call :CONV mono004          -cg -b2
call :CONV mono004-xdpx2    -cg -b2 -xdp:1
call :CONV mono004-xdpx4    -cg -b2 -xdp:2
call :CONV mono004-xdpx8    -cg -b2 -xdp:3
call :CONV mono004-xde      -cg -b2 -xde
call :CONV mono004-xdex2    -cg -b2 -xde:1
call :CONV mono004-xdex4    -cg -b2 -xde:2
call :CONV mono004-xdex8    -cg -b2 -xde:3

call :CONV mono008          -cg -b3
call :CONV mono008-xdpx2    -cg -b3 -xdp:1
call :CONV mono008-xdpx4    -cg -b3 -xdp:2
call :CONV mono008-xdpx8    -cg -b3 -xdp:3
call :CONV mono008-xde      -cg -b3 -xde
call :CONV mono008-xdex2    -cg -b3 -xde:1
call :CONV mono008-xdex4    -cg -b3 -xde:2
call :CONV mono008-xdex8    -cg -b3 -xde:3

call :CONV mono016          -cg -b4
call :CONV mono016-xdpx2    -cg -b4 -xdp:1
call :CONV mono016-xdpx4    -cg -b4 -xdp:2
call :CONV mono016-xdpx8    -cg -b4 -xdp:3
call :CONV mono016-xde      -cg -b4 -xde
call :CONV mono016-xdex2    -cg -b4 -xde:1
call :CONV mono016-xdex4    -cg -b4 -xde:2
call :CONV mono016-xdex8    -cg -b4 -xde:3

call :CONV mono032          -cg -b5
call :CONV mono032-xdpx2    -cg -b5 -xdp:1
call :CONV mono032-xdpx4    -cg -b5 -xdp:2
call :CONV mono032-xdpx8    -cg -b5 -xdp:3
call :CONV mono032-xde      -cg -b5 -xde
call :CONV mono032-xdex2    -cg -b5 -xde:1
call :CONV mono032-xdex4    -cg -b5 -xde:2
call :CONV mono032-xdex8    -cg -b5 -xde:3

call :CONV mono064          -cg -b6
call :CONV mono064-xdpx2    -cg -b6 -xdp:1
call :CONV mono064-xdpx4    -cg -b6 -xdp:2
call :CONV mono064-xdpx8    -cg -b6 -xdp:3
call :CONV mono064-xde      -cg -b6 -xde
call :CONV mono064-xdex2    -cg -b6 -xde:1
call :CONV mono064-xdex4    -cg -b6 -xde:2
call :CONV mono064-xdex8    -cg -b6 -xde:3

call :CONV mono007xterm-xdpx8 -cg -b8 -cp8 -cn7 -xdp:3
call :CONV mono007win-xdpx8   -cg -b8 -cp2 -cn7 -xdp:3

call :CONV c1mono003jp4-xdpx8 -cg:0000ff -b4 -cp1 -cn3 -xdp:3
call :CONV c2mono003jp4-xdpx8 -cg:ff0000 -b4 -cp1 -cn3 -xdp:3
call :CONV c3mono003jp4-xdpx8 -cg:ff00ff -b4 -cp1 -cn3 -xdp:3
call :CONV c4mono003jp4-xdpx8 -cg:00ff00 -b4 -cp1 -cn3 -xdp:3
call :CONV c5mono003jp4-xdpx8 -cg:00ffff -b4 -cp1 -cn3 -xdp:3
call :CONV c6mono003jp4-xdpx8 -cg:ffff00 -b4 -cp1 -cn3 -xdp:3

call :CONV c4mono009win-xdex8 -cg:00ff00 -b8 -cp2 -cn9 -xde:3
call :CONV c2mono007win-xdex8 -cg:ff0000 -b8 -cp2 -cn7 -xde:3
call :CONV c6mono007win-xdex8 -cg:ffff00 -b8 -cp2 -cn7 -xde:3
call :CONV cHmono007win-xdex8 -cg:ffddaa -b8 -cp2 -cn7 -xde:3

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
