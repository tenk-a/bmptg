cl -Ox -W3 -DNDEBUG -D_CRT_SECURE_NO_WARNINGS -I../ImgFmt/zlib smp_png2tga.cpp ../ImgFmt/tga_wrt.c ../ImgFmt/PngDecoder.cpp ../ImgFmt/libpng/libpng.lib ../ImgFmt/zlib/zlib.lib
