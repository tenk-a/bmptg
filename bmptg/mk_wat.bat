rem open watcom 1.9 + stlport�p
rem mingw32-make.exe �ւ�path���ʂ��Ă��邱��
rem stlport �̃f�B���N�g����ݒ�. ����w�b�_�݂̂�ok.
rem set STLP_INC_DIR=c:\lib\stlport-5.2.1\stlport

del err.txt
set ADD_CFLAGS=-DNDEBUG -DPNG_RESTRICT=
set ADD_SRCS=
set ADD_LIBS=
rem set IMGFMT_DIR=..\ImgFmt\
rem set MISC_DIR=..\misc\
rem set PROC_DIR=..\Proc\
set TGTNAME=bmptg

if not exist ..\ImgFmt\libpng\pnglibconf.h copy ..\ImgFmt\libpng\scripts\pnglibconf.h.prebuilt ..\ImgFmt\libpng\pnglibconf.h
copy ..\ImgFmt\jpeg\jconfig.wat ..\ImgFmt\jpeg\jconfig.h

mingw32-make -f bmptg.mak COMPILER=wat

set ADD_CFLAGS=
set ADD_SRCS=
set ADD_LIBS=
type err.txt
