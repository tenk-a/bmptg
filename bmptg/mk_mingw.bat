rem ../ImgFmt/jpeg/jconfig.h を mingw 対応物にコピーし直しているので注意.
rem vc に戻す場合は mk_vc.bat を再実行のこと.
del err.txt
set ADD_CFLAGS=-DNDEBUG
set ADD_SRCS=
set ADD_LIBS=
set TGTNAME=bmptg

if not exist ..\ImgFmt\libpng\pnglibconf.h copy ..\ImgFmt\libpng\scripts\pnglibconf.h.prebuilt ..\ImgFmt\libpng\pnglibconf.h
copy ..\ImgFmt\jpeg\jconfig.dj ..\ImgFmt\jpeg\jconfig.h

mingw32-make -f bmptg.mak COMPILER=gcc

set ADD_CFLAGS=
set ADD_SRCS=
set ADD_LIBS=
type err.txt
