# bmptg のコンパイル (for gnu make)
# 実質 vc 専用. mingw,dm-cもコンパイル通るかも.(他はすでにコンパイルが通らなくなった)

ERRFILE	=err.txt

ifndef TGTNAME
TGTNAME	=bmptg
endif

ifndef IMGFMT_DIR
IMGFMT_DIR=../ImgFmt/
MISC_DIR=../misc/
PROC_DIR=../Proc/
endif

JPG_DIR=$(IMGFMT_DIR)mozjpeg/
PNG_DIR=$(IMGFMT_DIR)libpng/
ZLIB_DIR=$(IMGFMT_DIR)zlib/

LIBS	=
BMPTG_SRCS=\
	bmptg.cpp 				\
	ConvOne.cpp				\
	BmImgLoad.cpp				\
	BmImgSave.cpp				\
	gen.cpp   				\
	gen_clmp.c				\
	gen_clm8.c				\
	$(PROC_DIR)pix32.c 			\
	$(PROC_DIR)pix8.c  			\
	$(PROC_DIR)pix32_kyuv.c			\
	$(PROC_DIR)pix32_resizeAveragingI.c	\
	$(PROC_DIR)pix32_resizeBicubic.c	\
	$(PROC_DIR)pix32_resizeBilinear.c	\
	$(PROC_DIR)pix32_resizeSpline36.c	\
	$(PROC_DIR)pix32_resizeLanczos3.c	\
	$(PROC_DIR)pix32_resizeBilinearAlpha.c	\
	$(PROC_DIR)pix32_resizeNearestNeighbor.c	\
	$(PROC_DIR)pix32_rotateBilinear.c	\
	$(PROC_DIR)pix32_rotateBicubic.c	\
	$(PROC_DIR)pix32_rotateSpline36.c	\
	$(PROC_DIR)pix32_rotateBilinearAlpha.c	\
	$(MISC_DIR)StrExpr.c			\
	$(MISC_DIR)subr.c 			\
	$(IMGFMT_DIR)beta.c  			\
	$(IMGFMT_DIR)tga_read.c			\
	$(IMGFMT_DIR)tga_wrt.c			\
	$(IMGFMT_DIR)bmp_read.c			\
	$(IMGFMT_DIR)bmp_wrt.c			\
	$(ADD_SRCS)

JPGSRCS=\
	$(IMGFMT_DIR)JpgDecoder.cpp 	\
	$(IMGFMT_DIR)JpgEncoder.cpp 	\

PNGSRCS=\
	$(IMGFMT_DIR)PngDecoder.cpp 	\
	$(IMGFMT_DIR)PngEncoder.cpp 	\
	\
	$(PNG_DIR)png.c			\
	$(PNG_DIR)pngerror.c		\
	$(PNG_DIR)pngget.c		\
	$(PNG_DIR)pngmem.c		\
	$(PNG_DIR)pngpread.c		\
	$(PNG_DIR)pngread.c		\
	$(PNG_DIR)pngrio.c		\
	$(PNG_DIR)pngrtran.c		\
	$(PNG_DIR)pngrutil.c		\
	$(PNG_DIR)pngset.c		\
	$(PNG_DIR)pngtrans.c		\
	$(PNG_DIR)pngwio.c		\
	$(PNG_DIR)pngwrite.c		\
	$(PNG_DIR)pngwtran.c		\
	$(PNG_DIR)pngwutil.c		\

ZLIBSRCS=\
	$(ZLIB_DIR)adler32.c		\
	$(ZLIB_DIR)compress.c		\
	$(ZLIB_DIR)crc32.c		\
	$(ZLIB_DIR)deflate.c		\
	$(ZLIB_DIR)infback.c		\
	$(ZLIB_DIR)inffast.c		\
	$(ZLIB_DIR)inflate.c		\
	$(ZLIB_DIR)inftrees.c		\
	$(ZLIB_DIR)trees.c		\
	$(ZLIB_DIR)uncompr.c		\
	$(ZLIB_DIR)zutil.c		\




#-----------------------------------------------------------------------------

ifeq ($(COMPILER), gcc)		# GNU C/C++
CFLAGS	=	-O3 -Wall -DUSE_JPG -DUSE_PNG -I"$(ZLIB_DIR)" -I"$(MISC_DIR)" -I"$(PROC_DIR)" -I"$(IMGFMT_DIR)" $(ADD_CFLAGS)
C_OPT_O =	-o 
CC	=	gcc -c
LINK	=	g++
LINK_OPT_O =	-o 
ERR	=2>>$(ERRFILE)

SRCS	= $(BMPTG_SRCS) $(JPGSRCS) $(PNGSRCS) $(ZLIBSRCS)

else
ifeq ($(COMPILER), bcc)		# Borland C/C++
#CFLAGS	=	-Ox -v $(ADD_CFLAGS)
CFLAGS	=	-O2 -Oc -Oi -OS -Ov -x- -pr -DUSE_JPG -DUSE_PNG -I"$(ZLIB_DIR)" -I"$(MISC_DIR)" -I"$(PROC_DIR)" -I"$(IMGFMT_DIR)" $(ADD_CFLAGS)
C_OPT_O =	-o
CC	=	bcc32 -c
LINK	=	bcc32
LINK_OPT_O =	-e
ERR	=>>$(ERRFILE)

SRCS	= $(subst /,\, $(BMPTG_SRCS) $(JPGSRCS) $(PNGSRCS) $(ZLIBSRCS))

else
ifeq ($(COMPILER), dmc)		# Digital Mars C/C++
CFLAGS	=	-w -o -Bj -j0 -DUSE_JPG -DUSE_PNG -I"$(ZLIB_DIR)" -I"$(MISC_DIR)" -I"$(PROC_DIR)" -I"$(IMGFMT_DIR)" $(ADD_CFLAGS)
C_OPT_O =	-o
CC	=	dmc -c
LINK	=	dmc
LINK_OPT_O =	-o
ERR	=>>$(ERRFILE)

SRCS	= $(subst /,\, $(BMPTG_SRCS) $(JPGSRCS) $(PNGSRCS) $(ZLIBSRCS))

else
ifeq ($(COMPILER), wat)		# Watcom-C/C++
CFLAGS	=	-ot -w3 -k2000000 -DUSE_JPG -DUSE_PNG -I"$(STLP_INC_DIR)" -I"$(ZLIB_DIR)" -I"$(IMGFMT_DIR)" -I"$(MISC_DIR)" -I"$(PROC_DIR)" $(ADD_CFLAGS)
C_OPT_O =	-fo
CC	=	wcl386 -c
LINK	=	wcl386
LINK_OPT_O =	-fe
ERR	=>>$(ERRFILE)

SRCS	= $(subst /,\, $(BMPTG_SRCS) $(JPGSRCS) $(PNGSRCS) $(ZLIBSRCS))

else		# Visual-C/C++
CFLAGS	=	-DUSE_JPG -DUSE_MOZJPEG -DUSE_PNG -EHsc -nologo -W4 -D_CRT_SECURE_NO_WARNINGS \
		 -I"$(ZLIB_DIR)" -I"$(MISC_DIR)" -I"$(PROC_DIR)" -I"$(IMGFMT_DIR)" $(ADD_CFLAGS) \
		 -wd4018 -wd4244 -wd4389 -wd4127 -wd4996 -wd4131 -wd4100
C_OPT_O =	-Fo
CC	=	cl -c
#LINK	=	cl -link/OPT:REF -link/OPT:ICF
LINK	=	cl
LINK_OPT_O =	-Fe
ERR	=>>$(ERRFILE)

SRCS	= $(BMPTG_SRCS) $(JPGSRCS) $(PNGSRCS) $(ZLIBSRCS)

endif
endif
endif
endif


#-----------------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .obj .asm .c .cpp

.cpp.obj:
	$(CC) $(CFLAGS) $(C_OPT_O)$*.obj $*.cpp $(ERR)

.c.obj:
	$(CC) $(CFLAGS) $(C_OPT_O)$*.obj $*.c   $(ERR)

# vpath=$(OBJ_DIR)
OBJS1 = $(SRCS:.c=.obj)
OBJS  = $(OBJS1:.cpp=.obj)

TGTFILE=$(TGTNAME).exe

ALL: $(TGTFILE)

$(TGTFILE):	$(OBJS)
	$(LINK) $(CFLAGS) $(LINK_OPT_O)$(TGTFILE) $(OBJS) $(LIBS) $(ADD_LIBS) $(ERR)
#	echo $(CFLAGS) $(LINK_OPT_O)$(TGTFILE) $(OBJS) $(LIBS) $(ADD_LIBS) >res.tmp
#	$(LINK) @res.tmp $(ERR)
