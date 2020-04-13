rem tspiroägí£î≈ÇÃiftga.spiÇÃê∂ê¨.
set FMT=tga
set NAME=if%FMT%
set COPT=-Ox -d -tWD -DUSE_REGOPT -DNDEBUG -D_WINDOWS
bcc32 %COPT% -e%NAME%.spi ../spi/SpiMain.c SpiRead_%FMT%.c ../ImgFmt/%FMT%_read.c ../spi/regopt.c
del  *.tds
