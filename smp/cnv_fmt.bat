if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_fmt
set JPG_OPTS=:jpg -xq80
set CMN_OPTS=:png -cfn112288

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\to_png
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% :png -d%TGTDIR% -s%SRCDIR% *.png
%BmpTgExe% :png -d%TGTDIR% -s%SRCDIR% *.jpg

set TGTDIR=%DSTDIR%\to_jpg
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% :jpg -xq85 -d%TGTDIR% -s%SRCDIR% *.png
%BmpTgExe% :jpg -xq85 -d%TGTDIR% -s%SRCDIR% *.jpg

set TGTDIR=%DSTDIR%\to_bmp
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% :bmp -d%TGTDIR% -s%SRCDIR% *.png
%BmpTgExe% :bmp -d%TGTDIR% -s%SRCDIR% *.jpg

set TGTDIR=%DSTDIR%\to_tga
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% :tga -d%TGTDIR% -s%SRCDIR% *.png
%BmpTgExe% :tga -d%TGTDIR% -s%SRCDIR% *.jpg

set SRCDIR=%TGTDIR%
set TGTDIR=%DSTDIR%\to_png_from_tga
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% :png -d%TGTDIR% -s%SRCDIR% *.tga

set SRCDIR=%DSTDIR%\to_bmp
set TGTDIR=%DSTDIR%\to_jpg_from_bmp
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% :jpg -xq85 -d%TGTDIR% -s%SRCDIR% *.bmp

