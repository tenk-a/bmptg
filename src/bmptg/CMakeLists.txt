cmake_minimum_required(VERSION 3.18)

project(bmptg)

set(ROOT_DIR       "${CMAKE_SOURCE_DIR}/../..")
set(THIRDPARTY_DIR "${ROOT_DIR}/thirdparty")
set(SRC_DIR        "${CMAKE_SOURCE_DIR}/..")
set(PNG_DIR        "${THIRDPARTY_DIR}/libpng")
set(ZLIB_DIR       "${THIRDPARTY_DIR}/zlib")

# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-DUSE_JPG -DUSE_PNG)

include(ExternalProject)

ExternalProject_Add(libjpeg-turbo
    SOURCE_DIR "${THIRDPARTY_DIR}/libjpeg-turbo"
    BINARY_DIR "${CMAKE_BINARY_DIR}/libjpeg-turbo-build"
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "${THIRDPARTY_DIR}/libjpeg-turbo"
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> -DCMAKE_BUILD_TYPE=Release
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

if(WIN32 AND MSVC)
  set(ADD_SRCS "${THIRDPARTY_DIR}/ms/ActiveCodePageUTF8.manifest")
  add_compile_options(-D_ANSI)

  if(MSVC)
    set(THIRDPARTY_LIB_DIR   ${THIRDPARTY_DIR}/lib/x64_Release)

    set(THIRDPARTY_JPEG_LIB_DIR ${THIRDPARTY_LIB_DIR}/libjpeg-turbo)
    set(THIRDPARTY_PNG_LIB_DIR  ${THIRDPARTY_LIB_DIR}/libpng)
    set(THIRDPARTY_ZLIB_LIB_DIR ${THIRDPARTY_LIB_DIR}/zlib)

    # set(JPEG_LIB ${THIRDPARTY_JPEG_LIB_DIR}/turbojpeg-static.lib)
#    set(JPEG_LIB ${CMAKE_BINARY_DIR}/libjpeg-turbo-build/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/turbojpeg-static.lib)
    set(JPEG_LIB ${CMAKE_BINARY_DIR}/libjpeg-turbo-build/Release/turbojpeg-static.lib)

    # set(PNG_LIB  ${THIRDPARTY_PNG_LIB_DIR}/libpng16_static.lib)
    # set(ZLIB_LIB ${THIRDPARTY_ZLIB_LIB_DIR}/zlibstatic.lib)

    # set(ADD_LIBS kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib)
    set(ADD_LIBS kernel32.lib user32.lib shell32.lib advapi32.lib)

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    add_compile_options(-Zc:wchar_t -Zc:forScope)

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
      set(ADD_INC_DIRS "${THIRDPARTY_DIR}/ccwrap/vc/" "${THIRDPARTY_DIR}/ccwrap/ccwrap/")
      add_compile_options(-FIccwrap_header.h)
      add_compile_options(-D_UNICODE)
    endif()
  endif()
  #if(CMAKE_C_COMPILER_ID MATCHES "Watcom" OR CMAKE_CXX_COMPILER_ID MATCHES "Watcom")
  #  # message(STATUS "Using Watcom C/C++ Compiler!!!")
  #  set(THIRDPARTY_LIB_DIR   ${THIRDPARTY_DIR}/lib/Release)
  #endif()
else()
# TODO:
#   if(APPLE)
#       #add_compile_options(--std=c++20)
#   else()
#       add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8 -fwide-exec-charset=utf-32LE)
#   endif()
#
#   set(THIRDPARTY_LIB_DIR  ${THIRDPARTY_DIR}/lib)
#
#   set(THIRDPARTY_JPEG_LIB_DIR ${THIRDPARTY_LIB_DIR}/libjpeg-turbo)
#   set(THIRDPARTY_PNG_LIB_DIR  ${THIRDPARTY_LIB_DIR}/libpng)
#   set(THIRDPARTY_ZLIB_LIB_DIR ${THIRDPARTY_LIB_DIR}/zlib)
#
#   set(JPEG_LIB ${THIRDPARTY_JPEG_LIB_DIR}/turbojpeg-static.a)
#   # set(PNG_LIB  ${THIRDPARTY_PNG_LIB_DIR}/libpng16_static.a)
#   # set(ZLIB_LIB ${THIRDPARTY_ZLIB_LIB_DIR}/zlibstatic.a)
endif()


set(BMPTG_SRCS
    ${SRC_DIR}/bmptg/stdafx.h
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
    ${SRC_DIR}/misc/def.h
    ${SRC_DIR}/misc/subr.h
    ${SRC_DIR}/misc/subr.c
    ${SRC_DIR}/misc/StrExpr.c
    ${SRC_DIR}/proc/pix32.c
    ${SRC_DIR}/proc/pix8.c
    ${SRC_DIR}/proc/pix32_kyuv.c
    ${SRC_DIR}/proc/pix32_resize.h
    ${SRC_DIR}/proc/pix32_resizeAveragingI.c
    ${SRC_DIR}/proc/pix32_resizeAveragingI.h
    ${SRC_DIR}/proc/pix32_resizeBicubic.c
    ${SRC_DIR}/proc/pix32_resizeBicubic.h
    ${SRC_DIR}/proc/pix32_resizeBilinear.c
    ${SRC_DIR}/proc/pix32_resizeBilinear.h
    ${SRC_DIR}/proc/pix32_resizeSpline36.c
    ${SRC_DIR}/proc/pix32_resizeSpline36.h
    ${SRC_DIR}/proc/pix32_resizeLanczos3.c
    ${SRC_DIR}/proc/pix32_resizeLanczos3.h
    ${SRC_DIR}/proc/pix32_resizeBilinear.h
    ${SRC_DIR}/proc/pix32_resizeBilinearAlpha.cpp
    ${SRC_DIR}/proc/pix32_resizeNearestNeighbor.c
    ${SRC_DIR}/proc/pix32_resizeNearestNeighbor.h
    ${SRC_DIR}/proc/pix32_rotate.h
    ${SRC_DIR}/proc/pix32_rotateBilinear.c
    ${SRC_DIR}/proc/pix32_rotateBicubic.c
    ${SRC_DIR}/proc/pix32_rotateSpline36.c
    ${SRC_DIR}/proc/pix32_rotateBilinearAlpha.c
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
    ${ADD_SRCS}
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


#set_target_properties(${PROJECT_NAME} PROPERTIES
#  RUNTIME_OUTPUT_DIRECTORY ${ROOT_DIR}/bin
#)

add_executable(${PROJECT_NAME}
    ${BMPTG_SRCS}
    ${PNG_SRCS}
    ${ZLIB_SRCS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}/bmptg
    ${SRC_DIR}/ImgFmt
    ${SRC_DIR}/proc
    ${SRC_DIR}/misc
    ${THIRDPARTY_DIR}
    ${THIRDPARTY_JPEG_LIB_DIR}
    ${THIRDPARTY_ZLIB_LIB_DIR}
    ${THIRDPARTY_PNG_LIB_DIR}
    ${THIRDPARTY_DIR}/libjpeg-turbo
    ${THIRDPARTY_DIR}/zlib
    ${THIRDPARTY_DIR}/libpng
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${JPEG_LIB}
#   ${PNG_LIB}
#   ${ZLIB_LIB}
    ${ADD_LIBS}
)
