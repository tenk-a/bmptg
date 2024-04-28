if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolMono
set CMN_OPTS=:bmp -s%SRCDIR% -if
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col002              -b1
call :CONV col002-xde          -b1 -xde
call :CONV col002-xdex2        -b1 -xde:1
call :CONV col002-xdex4        -b1 -xde:2
call :CONV col002-xdex8        -b1 -xde:3
call :CONV col002-xdpx2        -b1 -xdp:1
call :CONV col002-xdpx4        -b1 -xdp:2
call :CONV col002-xdpx8        -b1 -xdp:3
call :CONV col002-xdo09x2      -b1 -xdo09:1
call :CONV col002-xdo09x4      -b1 -xdo09:2
call :CONV col002-xdo09x8      -b1 -xdo09:3

call :CONV mono002         -cg -b1
call :CONV mono002-xde     -cg -b1 -xde
call :CONV mono002-xdex2   -cg -b1 -xde:1
call :CONV mono002-xdex4   -cg -b1 -xde:2
call :CONV mono002-xdex8   -cg -b1 -xde:3
call :CONV mono002-xdpx2   -cg -b1 -xdp:1
call :CONV mono002-xdpx4   -cg -b1 -xdp:2
call :CONV mono002-xdpx8   -cg -b1 -xdp:3
call :CONV mono002-xdo09x2 -cg -b1 -xdo09:1
call :CONV mono002-xdo09x4 -cg -b1 -xdo09:2
call :CONV mono002-xdo09x8 -cg -b1 -xdo09:3

call :CONV mono004         -cg -b2
call :CONV mono004-xde     -cg -b2 -xde
call :CONV mono004-xdex2   -cg -b2 -xde:1
call :CONV mono004-xdex4   -cg -b2 -xde:2
call :CONV mono004-xdex8   -cg -b2 -xde:3
call :CONV mono004-xdpx2   -cg -b2 -xdp:1
call :CONV mono004-xdpx4   -cg -b2 -xdp:2
call :CONV mono004-xdpx8   -cg -b2 -xdp:3
call :CONV mono004-xdo09x2 -cg -b2 -xdo09:1
call :CONV mono004-xdo09x4 -cg -b2 -xdo09:2
call :CONV mono004-xdo09x8 -cg -b2 -xdo09:3

goto END

call :CONV mono008         -cg -b3
call :CONV mono008-xde     -cg -b3 -xde
call :CONV mono008-xdex2   -cg -b3 -xde:1
call :CONV mono008-xdex4   -cg -b3 -xde:2
call :CONV mono008-xdex8   -cg -b3 -xde:3
call :CONV mono008-xdpx2   -cg -b3 -xdp:1
call :CONV mono008-xdpx4   -cg -b3 -xdp:2
call :CONV mono008-xdpx8   -cg -b3 -xdp:3
call :CONV mono008-xdo09x2 -cg -b3 -xdo09:1
call :CONV mono008-xdo09x4 -cg -b3 -xdo09:2
call :CONV mono008-xdo09x8 -cg -b3 -xdo09:3

call :CONV mono016         -cg -b4
call :CONV mono016-xde     -cg -b4 -xde
call :CONV mono016-xdex2   -cg -b4 -xde:1
call :CONV mono016-xdex4   -cg -b4 -xde:2
call :CONV mono016-xdex8   -cg -b4 -xde:3
call :CONV mono016-xdpx2   -cg -b4 -xdp:1
call :CONV mono016-xdpx4   -cg -b4 -xdp:2
call :CONV mono016-xdpx8   -cg -b4 -xdp:3
call :CONV mono016-xdo09x2 -cg -b4 -xdo09:1
call :CONV mono016-xdo09x4 -cg -b4 -xdo09:2
call :CONV mono016-xdo09x8 -cg -b4 -xdo09:3

call :CONV mono032         -cg -b5
call :CONV mono032-xde     -cg -b5 -xde
call :CONV mono032-xdex2   -cg -b5 -xde:1
call :CONV mono032-xdex4   -cg -b5 -xde:2
call :CONV mono032-xdex8   -cg -b5 -xde:3
call :CONV mono032-xdpx2   -cg -b5 -xdp:1
call :CONV mono032-xdpx4   -cg -b5 -xdp:2
call :CONV mono032-xdpx8   -cg -b5 -xdp:3
call :CONV mono032-xdo09x2 -cg -b5 -xdo09:1
call :CONV mono032-xdo09x4 -cg -b5 -xdo09:2
call :CONV mono032-xdo09x8 -cg -b5 -xdo09:3

call :CONV mono064         -cg -b6
call :CONV mono064-xde     -cg -b6 -xde
call :CONV mono064-xdex2   -cg -b6 -xde:1
call :CONV mono064-xdex4   -cg -b6 -xde:2
call :CONV mono064-xdex8   -cg -b6 -xde:3
call :CONV mono064-xdpx2   -cg -b6 -xdp:1
call :CONV mono064-xdpx4   -cg -b6 -xdp:2
call :CONV mono064-xdpx8   -cg -b6 -xdp:3
call :CONV mono064-xdo09x2 -cg -b6 -xdo09:1
call :CONV mono064-xdo09x4 -cg -b6 -xdo09:2
call :CONV mono064-xdo09x8 -cg -b6 -xdo09:3

call :CONV mono016-cp5     -cg -b4 -cp5

call :CONV mono004jp3         -cg -b3 -cp1 -cn4
call :CONV mono003jp3         -cg -b3 -cp1 -cn3
call :CONV mono003jp3-xde     -cg -b3 -cp1 -cn3 -xde
call :CONV mono003jp3-xdex2   -cg -b3 -cp1 -cn3 -xde:1
call :CONV mono003jp3-xdex4   -cg -b3 -cp1 -cn3 -xde:2
call :CONV mono003jp3-xdex8   -cg -b3 -cp1 -cn3 -xde:3
call :CONV mono003jp3-xdpx2   -cg -b3 -cp1 -cn3 -xdp:1
call :CONV mono003jp3-xdpx4   -cg -b3 -cp1 -cn3 -xdp:2
call :CONV mono003jp3-xdpx8   -cg -b3 -cp1 -cn3 -xdp:3
call :CONV mono003jp3-xdo09x2 -cg -b3 -cp1 -cn3 -xdo09:1
call :CONV mono003jp3-xdo09x4 -cg -b3 -cp1 -cn3 -xdo09:2
call :CONV mono003jp3-xdo09x8 -cg -b3 -cp1 -cn3 -xdo09:3

call :CONV mono003jp4         -cg -b4 -cp1 -cn3
call :CONV mono003jp4-xde     -cg -b4 -cp1 -cn3 -xde
call :CONV mono003jp4-xdex2   -cg -b4 -cp1 -cn3 -xde:1
call :CONV mono003jp4-xdex4   -cg -b4 -cp1 -cn3 -xde:2
call :CONV mono003jp4-xdex8   -cg -b4 -cp1 -cn3 -xde:3
call :CONV mono003jp4-xdpx2   -cg -b4 -cp1 -cn3 -xdp:1
call :CONV mono003jp4-xdpx4   -cg -b4 -cp1 -cn3 -xdp:2
call :CONV mono003jp4-xdpx8   -cg -b4 -cp1 -cn3 -xdp:3
call :CONV mono003jp4-xdo09x2 -cg -b4 -cp1 -cn3 -xdo09:1
call :CONV mono003jp4-xdo09x4 -cg -b4 -cp1 -cn3 -xdo09:2
call :CONV mono003jp4-xdo09x8 -cg -b4 -cp1 -cn3 -xdo09:3

call :CONV mono004jp4         -cg -b4 -cp1 -cn4
call :CONV mono004jp4-xde     -cg -b4 -cp1 -cn4 -xde
call :CONV mono004jp4-xdex2   -cg -b4 -cp1 -cn4 -xde:1
call :CONV mono004jp4-xdex4   -cg -b4 -cp1 -cn4 -xde:2
call :CONV mono004jp4-xdex8   -cg -b4 -cp1 -cn4 -xde:3
call :CONV mono004jp4-xdpx2   -cg -b4 -cp1 -cn4 -xdp:1
call :CONV mono004jp4-xdpx4   -cg -b4 -cp1 -cn4 -xdp:2
call :CONV mono004jp4-xdpx8   -cg -b4 -cp1 -cn4 -xdp:3
call :CONV mono004jp4-xdo09x2 -cg -b4 -cp1 -cn4 -xdo09:1
call :CONV mono004jp4-xdo09x4 -cg -b4 -cp1 -cn4 -xdo09:2
call :CONV mono004jp4-xdo09x8 -cg -b4 -cp1 -cn4 -xdo09:3

call :CONV c1mono003jp3-xde -cg:0000ff -b3 -cp1 -cn3 -xde
call :CONV c2mono003jp3-xde -cg:ff0000 -b3 -cp1 -cn3 -xde
call :CONV c3mono003jp3-xde -cg:ff00ff -b3 -cp1 -cn3 -xde
call :CONV c4mono003jp3-xde -cg:00ff00 -b3 -cp1 -cn3 -xde
call :CONV c5mono003jp3-xde -cg:00ffff -b3 -cp1 -cn3 -xde
call :CONV c6mono003jp3-xde -cg:ffff00 -b3 -cp1 -cn3 -xde

call :CONV c1mono003jp4-xde -cg:0000ff -b4 -cp1 -cn3 -xde
call :CONV c2mono003jp4-xde -cg:ff0000 -b4 -cp1 -cn3 -xde
call :CONV c3mono003jp4-xde -cg:ff00ff -b4 -cp1 -cn3 -xde
call :CONV c4mono003jp4-xde -cg:00ff00 -b4 -cp1 -cn3 -xde
call :CONV c5mono003jp4-xde -cg:00ffff -b4 -cp1 -cn3 -xde
call :CONV c6mono003jp4-xde -cg:ffff00 -b4 -cp1 -cn3 -xde

call :CONV c1mono004jp4-xde -cg:0000ff -b4 -cp1 -cn4 -xde
call :CONV c2mono004jp4-xde -cg:ff0000 -b4 -cp1 -cn4 -xde
call :CONV c3mono004jp4-xde -cg:ff00ff -b4 -cp1 -cn4 -xde
call :CONV c4mono004jp4-xde -cg:00ff00 -b4 -cp1 -cn4 -xde
call :CONV c5mono004jp4-xde -cg:00ffff -b4 -cp1 -cn4 -xde
call :CONV c6mono004jp4-xde -cg:ffff00 -b4 -cp1 -cn4 -xde

goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
