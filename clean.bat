rem call delbak.bat .\
rem call delobj.bat .\

attrib /S -R -S -H *.suo
del /S *.bak *.obj *.o *.ncb *.pdb *.idb *.ilk *.tds *.tmp *.suo err.txt *.ilc *.ild *.ilf *.ils *.map
