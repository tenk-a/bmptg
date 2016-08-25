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

JPG_DIR=$(IMGFMT_DIR)jpeg/
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
	$(PROC_DIR)pix32_resizeNearestNeighbor.c	\
	$(PROC_DIR)pix32_rotateBilinear.c	\
	$(PROC_DIR)pix32_rotateBicubic.c	\
	$(PROC_DIR)pix32_rotateSpline36.c	\
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
	\
	$(JPG_DIR)jaricom.c		\
	$(JPG_DIR)jcapimin.c		\
	$(JPG_DIR)jcapistd.c		\
	$(JPG_DIR)jcarith.c		\
	$(JPG_DIR)jccoefct.c		\
	$(JPG_DIR)jccolor.c		\
	$(JPG_DIR)jcdctmgr.c		\
	$(JPG_DIR)jchuff.c		\
	$(JPG_DIR)jcinit.c		\
	$(JPG_DIR)jcmainct.c		\
	$(JPG_DIR)jcmarker.c		\
	$(JPG_DIR)jcmaster.c		\
	$(JPG_DIR)jcomapi.c		\
	$(JPG_DIR)jcparam.c		\
	$(JPG_DIR)jcprepct.c		\
	$(JPG_DIR)jcsample.c		\
	$(JPG_DIR)jctrans.c		\
	$(JPG_DIR)jdapimin.c		\
	$(JPG_DIR)jdapistd.c		\
	$(JPG_DIR)jdarith.c		\
	$(JPG_DIR)jdatadst.c		\
	$(JPG_DIR)jdatasrc.c		\
	$(JPG_DIR)jdcoefct.c		\
	$(JPG_DIR)jdcolor.c		\
	$(JPG_DIR)jddctmgr.c		\
	$(JPG_DIR)jdhuff.c		\
	$(JPG_DIR)jdinput.c		\
	$(JPG_DIR)jdmainct.c		\
	$(JPG_DIR)jdmarker.c		\
	$(JPG_DIR)jdmaster.c		\
	$(JPG_DIR)jdmerge.c		\
	$(JPG_DIR)jdpostct.c		\
	$(JPG_DIR)jdsample.c		\
	$(JPG_DIR)jdtrans.c		\
	$(JPG_DIR)jerror.c		\
	$(JPG_DIR)jfdctflt.c		\
	$(JPG_DIR)jfdctfst.c		\
	$(JPG_DIR)jfdctint.c		\
	$(JPG_DIR)jidctflt.c		\
	$(JPG_DIR)jidctfst.c		\
	$(JPG_DIR)jidctint.c		\
	$(JPG_DIR)jmemmgr.c		\
	$(JPG_DIR)jmemnobs.c		\
	$(JPG_DIR)jquant1.c		\
	$(JPG_DIR)jquant2.c		\
	$(JPG_DIR)jutils.c		\

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
CFLAGS	=	-DUSE_JPG -DUSE_PNG -EHsc -nologo -Ox -Ot -W4 -D_CRT_SECURE_NO_WARNINGS \
		 -I"$(ZLIB_DIR)" -I"$(MISC_DIR)" -I"$(PROC_DIR)" -I"$(IMGFMT_DIR)" $(ADD_CFLAGS)
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
