rem ../ImgFmt/jpeg/jconfig.h �� mingw �Ή����ɃR�s�[�������Ă���̂Œ���.
rem vc �ɖ߂��ꍇ�� mk_vc.bat ���Ď��s�̂���.
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
