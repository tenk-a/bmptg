!include x64.nsh
Name "libjpeg-turbo SDK for Visual C++"
OutFile "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}libjpeg-turbo-1.4.80-vc.exe"
InstallDir c:\libjpeg-turbo

SetCompressor bzip2

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "libjpeg-turbo SDK for Visual C++ (required)"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif
	SectionIn RO
!ifdef GCC
	IfFileExists $SYSDIR/libturbojpeg.dll exists 0
!else
	IfFileExists $SYSDIR/turbojpeg.dll exists 0
!endif
	goto notexists
	exists:
!ifdef GCC
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ is already installed.  Please uninstall it first."
!else
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ or the TurboJPEG SDK is already installed.  Please uninstall it first."
!endif
	quit

	notexists:
	SetOutPath $SYSDIR
!ifdef GCC
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\libturbojpeg.dll"
!else
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}turbojpeg.dll"
!endif
	SetOutPath $INSTDIR\bin
!ifdef GCC
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\libturbojpeg.dll"
!else
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}turbojpeg.dll"
!endif
!ifdef GCC
	File "/oname=libjpeg-62.dll" "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\libjpeg-*.dll"
!else
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\${BUILDDIR}jpeg62.dll"
!endif
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\${BUILDDIR}cjpeg.exe"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\${BUILDDIR}djpeg.exe"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\${BUILDDIR}jpegtran.exe"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}tjbench.exe"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}rdjpgcom.exe"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}wrjpgcom.exe"
	SetOutPath $INSTDIR\lib
!ifdef GCC
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\libturbojpeg.dll.a"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\libturbojpeg.a"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\libjpeg.dll.a"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\libjpeg.a"
!else
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}turbojpeg.lib"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}turbojpeg-static.lib"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\sharedlib\${BUILDDIR}jpeg.lib"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\${BUILDDIR}jpeg-static.lib"
!endif
!ifdef JAVA
	SetOutPath $INSTDIR\classes
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\java\${BUILDDIR}turbojpeg.jar"
!endif
	SetOutPath $INSTDIR\include
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\jconfig.h"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\jerror.h"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\jmorecfg.h"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\jpeglib.h"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\turbojpeg.h"
	SetOutPath $INSTDIR\doc
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\README"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\README-turbo.txt"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\example.c"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\libjpeg.txt"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\structure.txt"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\usage.txt"
	File "D:/apart/proj/ImgEx/ImgFmt/turbojpeg\wizard.txt"

	WriteRegStr HKLM "SOFTWARE\libjpeg-turbo 1.4.80" "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.4.80" "DisplayName" "libjpeg-turbo SDK v1.4.80 for Visual C++"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.4.80" "UninstallString" '"$INSTDIR\uninstall_1.4.80.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.4.80" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.4.80" "NoRepair" 1
	WriteUninstaller "uninstall_1.4.80.exe"
SectionEnd

Section "Uninstall"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif

	SetShellVarContext all

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.4.80"
	DeleteRegKey HKLM "SOFTWARE\libjpeg-turbo 1.4.80"

!ifdef GCC
	Delete $INSTDIR\bin\libjpeg-62.dll
	Delete $INSTDIR\bin\libturbojpeg.dll
	Delete $SYSDIR\libturbojpeg.dll
	Delete $INSTDIR\lib\libturbojpeg.dll.a"
	Delete $INSTDIR\lib\libturbojpeg.a"
	Delete $INSTDIR\lib\libjpeg.dll.a"
	Delete $INSTDIR\lib\libjpeg.a"
!else
	Delete $INSTDIR\bin\jpeg62.dll
	Delete $INSTDIR\bin\turbojpeg.dll
	Delete $SYSDIR\turbojpeg.dll
	Delete $INSTDIR\lib\jpeg.lib
	Delete $INSTDIR\lib\jpeg-static.lib
	Delete $INSTDIR\lib\turbojpeg.lib
	Delete $INSTDIR\lib\turbojpeg-static.lib
!endif
!ifdef JAVA
	Delete $INSTDIR\classes\turbojpeg.jar
!endif
	Delete $INSTDIR\bin\cjpeg.exe
	Delete $INSTDIR\bin\djpeg.exe
	Delete $INSTDIR\bin\jpegtran.exe
	Delete $INSTDIR\bin\tjbench.exe
	Delete $INSTDIR\bin\rdjpgcom.exe
	Delete $INSTDIR\bin\wrjpgcom.exe
	Delete $INSTDIR\include\jconfig.h"
	Delete $INSTDIR\include\jerror.h"
	Delete $INSTDIR\include\jmorecfg.h"
	Delete $INSTDIR\include\jpeglib.h"
	Delete $INSTDIR\include\turbojpeg.h"
	Delete $INSTDIR\uninstall_1.4.80.exe
	Delete $INSTDIR\doc\README
	Delete $INSTDIR\doc\README-turbo.txt
	Delete $INSTDIR\doc\example.c
	Delete $INSTDIR\doc\libjpeg.txt
	Delete $INSTDIR\doc\structure.txt
	Delete $INSTDIR\doc\usage.txt
	Delete $INSTDIR\doc\wizard.txt

	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\doc"
!ifdef JAVA
	RMDir "$INSTDIR\classes"
!endif
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"

SectionEnd
