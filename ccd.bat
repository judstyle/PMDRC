REM Make pmdrc.dll
@echo off
cl /c /GD /nologo .\pmdrc\prdmain.c
cl /c /GD /nologo .\pmdrc\prdanal.c
cl /c /GD /nologo .\pmdrc\prdout.c
cl /c /GD /nologo .\pmdrc\prdutil.c
cl /c /GD /nologo .\pmdrc\prdstat.c
link /DLL /nologo /export:pmdrc_dllmain /export:pmdrc_dllfreem /out:.\pmdrc.dll .\*.obj
rem link /DLL /nologo /out:.\pmdrc.dll .\*.obj
del .\*.obj
copy .\pmdrc.dll C:\WINNT\system32\
