REM Make f2pcv.dll
@echo off
cl /c /GD /nologo .\f2pcv\fpdmain.c
cl /c /GD /nologo .\f2pcv\fpdanal.c
cl /c /GD /nologo .\f2pcv\fpdout.c
cl /c /GD /nologo .\f2pcv\fpdutil.c
cl /c /GD /nologo .\f2pcv\fpdstat.c
link /DLL /nologo /export:f2pcv_dllmain /export:f2pcv_dllfreem /out:.\f2pcv.dll .\*.obj
del .\*.obj
copy .\f2pcv.dll C:\WINNT\system32\
