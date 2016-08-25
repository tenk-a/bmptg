set COPT=-MT -Ox -Ot -GL -GA -W3 -wd4244 -wd4127 -wd4100 -D"NDEBUG" -D"_WINDOWS" -D"USE_REGOPT"
rc -d"NDEBUG" -l 0x411 tspiro.rc
cl %COPT% tspiro.c gr.c tspiro.res comdlg32.lib advapi32.lib user32.lib gdi32.lib shell32.lib -link/subsystem:windows
del *.obj
del *.ncb
del *.res
