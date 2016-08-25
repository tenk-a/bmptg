rem tspiroägí£î≈ÇÃiftga.spiÇÃê∂ê¨.
@echo off
set FMT=tga
set NAME=if%FMT%
set SPIDIR=../spi
set FMTDIR=../ImgFmt
set COPT=-LD -MT -Ox -Ot -GL -GA -W4 -wd4244 -wd4127 -D"NDEBUG" -D"_WINDOWS" -D"USE_REGOPT" -I%SPIDIR% -I%FMTDIR%
set LNKOPT= advapi32.lib -link/def:%SPIDIR%/Spi.def
cl %COPT% -Fe%NAME%.spi %SPIDIR%/SpiMain.c SpiRead_%FMT%.c %FMTDIR%/%FMT%_read.c %SPIDIR%/RegOpt.c %LNKOPT%
del *.obj
del *.exp
del *.lib
