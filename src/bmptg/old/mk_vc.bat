rem mingw32-make.exe ‚Ö‚Ìpath‚ª’Ê‚Á‚Ä‚¢‚é‚±‚Æ
del err.txt
set ADD_CFLAGS=-DNDEBUG -wd4018 -wd4244 -wd4389 -wd4127 -wd4996 -wd4131 -wd4100
set ADD_SRCS=
set ADD_LIBS=
set TGTNAME=bmptg

if not exist ..\ImgFmt\libpng\pnglibconf.h copy ..\ImgFmt\libpng\scripts\pnglibconf.h.prebuilt ..\ImgFmt\libpng\pnglibconf.h
copy ..\ImgFmt\jpeg\jconfig.vc ..\ImgFmt\jpeg\jconfig.h

mingw32-make -f bmptg.mak COMPILER=%1

set ADD_CFLAGS=
set ADD_SRCS=
set ADD_LIBS=
type err.txt
