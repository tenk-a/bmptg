cmake_minimum_required(VERSION 3.18)

project(bmptg)

set(ROOT_DIR       "${CMAKE_SOURCE_DIR}/../..")
set(THIRDPARTY_DIR "${ROOT_DIR}/thirdparty")
set(SRC_DIR        "${CMAKE_SOURCE_DIR}/..")
set(PNG_DIR        "${THIRDPARTY_DIR}/libpng")
set(ZLIB_DIR       "${THIRDPARTY_DIR}/zlib")

set(LIBJPEG_TURBO_INSTALL_DIR "${CMAKE_BINARY_DIR}/libjpeg-turbo-install")
set(JPEG_LIB ${LIBJPEG_TURBO_INSTALL_DIR}/lib/libturbojpeg.a)

set(BMPTG_SRCS
    ${SRC_DIR}/bmptg/bmptg.cpp
    ${SRC_DIR}/bmptg/ConvOne.cpp
    ${SRC_DIR}/bmptg/ConvOne.hpp
    ${SRC_DIR}/bmptg/BmImgLoad.h
    ${SRC_DIR}/bmptg/BmImgLoad.cpp
    ${SRC_DIR}/bmptg/BmImgSave.h
    ${SRC_DIR}/bmptg/BmImgSave.cpp
    ${SRC_DIR}/bmptg/gen.h
    ${SRC_DIR}/bmptg/gen.cpp
    ${SRC_DIR}/bmptg/gen_clmp.c
    ${SRC_DIR}/bmptg/gen_clm8.c
    ${SRC_DIR}/bmptg/stdafx.h
    ${SRC_DIR}/bmptg/ExArgv_conf.h
)

set(IMGFMT_SRCS
    ${SRC_DIR}/ImgFmt/beta.c
    ${SRC_DIR}/ImgFmt/beta.h
    ${SRC_DIR}/ImgFmt/tga_read.c
    ${SRC_DIR}/ImgFmt/tga_read.h
    ${SRC_DIR}/ImgFmt/tga_wrt.c
    ${SRC_DIR}/ImgFmt/tga_wrt.h
    ${SRC_DIR}/ImgFmt/bmp_read.c
    ${SRC_DIR}/ImgFmt/bmp_read.h
    ${SRC_DIR}/ImgFmt/bmp_wrt.c
    ${SRC_DIR}/ImgFmt/bmp_wrt.h
    ${SRC_DIR}/ImgFmt/PngEncoder.hpp
    ${SRC_DIR}/ImgFmt/PngEncoder.cpp
    ${SRC_DIR}/ImgFmt/PngDecoder.hpp
    ${SRC_DIR}/ImgFmt/PngDecoder.cpp
    ${SRC_DIR}/ImgFmt/JpgEncoder.hpp
    ${SRC_DIR}/ImgFmt/JpgEncoder.cpp
    ${SRC_DIR}/ImgFmt/JpgDecoder.hpp
    ${SRC_DIR}/ImgFmt/JpgDecoder.cpp
)

set(PROC_SRCS
    ${SRC_DIR}/Proc/pix8.c
    ${SRC_DIR}/Proc/pix32.c
    ${SRC_DIR}/Proc/pix32_kyuv.c
    ${SRC_DIR}/Proc/pix32_resize.h
    ${SRC_DIR}/Proc/pix32_resizeAveragingI.c
    ${SRC_DIR}/Proc/pix32_resizeAveragingI.h
    ${SRC_DIR}/Proc/pix32_resizeBicubic.c
    ${SRC_DIR}/Proc/pix32_resizeBicubic.h
    ${SRC_DIR}/Proc/pix32_resizeBilinear.c
    ${SRC_DIR}/Proc/pix32_resizeBilinear.h
    ${SRC_DIR}/Proc/pix32_resizeSpline36.c
    ${SRC_DIR}/Proc/pix32_resizeSpline36.h
    ${SRC_DIR}/Proc/pix32_resizeLanczos3.c
    ${SRC_DIR}/Proc/pix32_resizeLanczos3.h
    ${SRC_DIR}/Proc/pix32_resizeBilinear.h
    ${SRC_DIR}/Proc/pix32_resizeBilinearAlpha.cpp
    ${SRC_DIR}/Proc/pix32_resizeNearestNeighbor.c
    ${SRC_DIR}/Proc/pix32_resizeNearestNeighbor.h
    ${SRC_DIR}/Proc/pix32_rotate.h
    ${SRC_DIR}/Proc/pix32_rotateBilinear.c
    ${SRC_DIR}/Proc/pix32_rotateBicubic.c
    ${SRC_DIR}/Proc/pix32_rotateSpline36.c
    ${SRC_DIR}/Proc/pix32_rotateBilinearAlpha.c
)

set(MISC_SRCS
    ${SRC_DIR}/misc/def.h
    ${SRC_DIR}/misc/subr.h
    ${SRC_DIR}/misc/subr.c
    ${SRC_DIR}/misc/StrExpr.c
    ${SRC_DIR}/misc/ExArgv.h
    ${SRC_DIR}/misc/ExArgv.c
)

set(ZLIB_SRCS
    ${ZLIB_DIR}/adler32.c
    ${ZLIB_DIR}/compress.c
    ${ZLIB_DIR}/crc32.c
    ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/infback.c
    ${ZLIB_DIR}/inffast.c
    ${ZLIB_DIR}/inflate.c
    ${ZLIB_DIR}/inftrees.c
    ${ZLIB_DIR}/trees.c
    ${ZLIB_DIR}/uncompr.c
    ${ZLIB_DIR}/zutil.c
)

set(PNG_SRCS
    ${PNG_DIR}/png.c
    ${PNG_DIR}/pngerror.c
    ${PNG_DIR}/pngget.c
    ${PNG_DIR}/pngmem.c
    ${PNG_DIR}/pngpread.c
    ${PNG_DIR}/pngread.c
    ${PNG_DIR}/pngrio.c
    ${PNG_DIR}/pngrtran.c
    ${PNG_DIR}/pngrutil.c
    ${PNG_DIR}/pngset.c
    ${PNG_DIR}/pngtrans.c
    ${PNG_DIR}/pngwio.c
    ${PNG_DIR}/pngwrite.c
    ${PNG_DIR}/pngwtran.c
    ${PNG_DIR}/pngwutil.c
)

add_compile_options(-DUSE_JPG -DUSE_PNG -DEXARGV_USE_CONF_H)

if(NOT MSVC)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(WIN32)
  set(ADD_SRCS "${THIRDPARTY_DIR}/ms/ActiveCodePageUTF8.manifest")

  if(MSVC)
    add_compile_options(-DUNICODE -D_UNICODE)
    add_compile_options(-D_CRT_SECURE_NO_WARNINGS)
    set(JPEG_LIB ${LIBJPEG_TURBO_INSTALL_DIR}/lib/turbojpeg-static.lib)
    # set(ADD_LIBS kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib)
    set(ADD_LIBS kernel32.lib user32.lib shell32.lib advapi32.lib)

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    add_compile_options(-Zc:wchar_t -Zc:forScope)

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.0")
      set(CMAKE_CXX_STANDARD 17)
      set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()

    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "19.0.24215.1")
      add_compile_options(-utf-8)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.0.24215.1")
      add_compile_options(-utf-8 -Zc:rvalueCast)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.14.26428.1")
      add_compile_options(-Zc:__cplusplus)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.0")
      if(NOT EXISTS "${THIRDPARTY_DIR}/ccwrap")
        message(FATAL_ERROR "thirdparty/ccwrap directory is missing. For vc12 and earlier, run thirdparty/install_ccwap.bat")
      endif()
      add_compile_options(-FI${THIRDPARTY_DIR}/ccwrap/vc/ccwrap_header.h)
      set(ADD_INC_DIRS "${THIRDPARTY_DIR}/ccwrap/vc/" "${THIRDPARTY_DIR}/ccwrap/ccwrap/")
      # add_compile_options(-D_UNICODE)
    endif()
  #elseif(CMAKE_C_COMPILER_ID MATCHES "Watcom" OR CMAKE_CXX_COMPILER_ID MATCHES "Watcom")
  #  # message(STATUS "Using Watcom C/C++ Compiler!!!")
  else()
    # g++
    add_compile_options(-D_ANSI)
    add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8 -fwide-exec-charset=utf-32LE)
    # add_compile_options(-municode)
  endif()
elseif(APPLE)
  #add_compile_options(--std=c++20)
else()
  add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8 -fwide-exec-charset=utf-32LE)
endif()

include(ExternalProject)

#set_target_properties(${PROJECT_NAME} PROPERTIES
#  RUNTIME_OUTPUT_DIRECTORY ${ROOT_DIR}/bin
#)

add_executable(${PROJECT_NAME}
    ${BMPTG_SRCS}
    ${IMGFMT_SRCS}
    ${PROC_SRCS}
    ${MISC_SRCS}
    ${PNG_SRCS}
    ${ZLIB_SRCS}
    ${ADD_SRCS}
)

ExternalProject_Add(
    libjpeg-turbo
    SOURCE_DIR  "${THIRDPARTY_DIR}/libjpeg-turbo"
    BINARY_DIR  "${CMAKE_BINARY_DIR}/libjpeg-turbo-build"
    INSTALL_DIR "${LIBJPEG_TURBO_INSTALL_DIR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
#    BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --target turbojpeg-static
    INSTALL_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --target install --config $<CONFIG>
    STEP_TARGETS build
    EXCLUDE_FROM_ALL TRUE
)
add_dependencies(${PROJECT_NAME} libjpeg-turbo)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${ADD_INC_DIRS}
    ${SRC_DIR}/bmptg
    ${SRC_DIR}/ImgFmt
    ${SRC_DIR}/Proc
    ${SRC_DIR}/misc
    ${LIBJPEG_TURBO_INSTALL_DIR}/include
    ${THIRDPARTY_DIR}
    ${THIRDPARTY_DIR}/include
    ${THIRDPARTY_DIR}/libjpeg-turbo
    ${THIRDPARTY_DIR}/zlib
    ${THIRDPARTY_DIR}/libpng
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${JPEG_LIB}
    ${ADD_LIBS}
)
