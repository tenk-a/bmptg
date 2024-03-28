md tmp_t1
abx -x -s smp\*.tga =smp_decreaseColorT1 smp\$c tmp_t1\$c
md tmp_t2
abx -x -s smp\*.tga =smp_decreaseColorT2 smp\$c tmp_t2\$c
md tmp_t3
abx -x -s smp\*.tga =smp_decreaseColorT3 smp\$c tmp_t3\$c
md tmp_mc1
abx -x -s smp\*.tga =smp_decreaseColorMC1 smp\$c tmp_mc1\$c
md tmp_mc2
abx -x -s smp\*.tga =smp_decreaseColorMC2 smp\$c tmp_mc2\$c
md tmp_win256
abx -x -s smp\*.tga =smp_decreaseColorWin256 smp\$c tmp_win256\$c
md tmp_iwi256
abx -x -s smp\*.tga =smp_decreaseColorIfWithin256 smp\$c tmp_iwi256\$c
