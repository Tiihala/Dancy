@ECHO OFF

IF NOT EXIST "scripts\clean.cmd" (
    ECHO Error: does not look like a root of Dancy source tree 1>&2
    EXIT /B 1
)

RMDIR /S /Q arctic\bin32 > NUL 2>&1
RMDIR /S /Q arctic\bin64 > NUL 2>&1
RMDIR /S /Q arctic\o32 > NUL 2>&1
RMDIR /S /Q arctic\o64 > NUL 2>&1
RMDIR /S /Q bin > NUL 2>&1
RMDIR /S /Q efi > NUL 2>&1
RMDIR /S /Q include\acpica > NUL 2>&1
RMDIR /S /Q kernel\acpica > NUL 2>&1
RMDIR /S /Q o32 > NUL 2>&1
RMDIR /S /Q o64 > NUL 2>&1
RMDIR /S /Q release > NUL 2>&1
RMDIR /S /Q system > NUL 2>&1

DEL arctic\bin32.img > NUL 2>&1
DEL arctic\bin64.img > NUL 2>&1
DEL arctic\root.img > NUL 2>&1

DEL scripts\dancy.mk > NUL 2>&1
DEL LOADER.512 > NUL 2>&1
DEL LOADER.AT > NUL 2>&1

FOR /R %%i IN (*.obj) DO DEL "%%i"
