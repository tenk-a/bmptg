rem mingw32-make.exe へのpathが通っていること
del err.txt
set ADD_CFLAGS=-DNDEBUG -Ox -Ot -D_RELEASE
set ADD_SRCS=
set ADD_LIBS=..\ImgFmt\turbojpeg\turbojpeg-static%1.lib

set TGTNAME=bmptg-tj%1

if not exist ..\ImgFmt\libpng\pnglibconf.h copy ..\ImgFmt\libpng\scripts\pnglibconf.h.prebuilt ..\ImgFmt\libpng\pnglibconf.h

mingw32-make -f bmptg-tj.mak COMPILER=cl

set ADD_CFLAGS=
set ADD_SRCS=
set ADD_LIBS=
type err.txt
