REM Make pmdrcc.exe
@echo off
cl /c /nologo .\pmdrcc\pmdrcc.c
link /nologo /out:.\pmdrcc.exe .\*.obj
del .\*.obj
copy .\pmdrcc.exe c:\winnt
