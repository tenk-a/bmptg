set CCOPTS=-TP -EHsc -Ox -I../ImgFmt/
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorT1.h"  -Fesmp_decreaseColorT1.exe  smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorT2.h"  -Fesmp_decreaseColorT2.exe  smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorT3.h"  -Fesmp_decreaseColorT3.exe  smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorMC1.h" -Fesmp_decreaseColorMC1.exe smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorMC2.h" -Fesmp_decreaseColorMC2.exe smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorWin256.h" -Fesmp_decreaseColorWin256.exe smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
cl %CCOPTS% -FI"../Proc_Test/DecreaseColorIfWithin256.h" -DDecreaseColor=DecreaseColorIfWithin256 -Fesmp_decreaseColorIfWithin256.exe smp_decreaseColor.cpp ../ImgFmt/tga_read.c ../ImgFmt/tga_wrt.c
