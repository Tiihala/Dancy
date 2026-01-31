@ECHO OFF
SETLOCAL

SET ACPICA_TAG=R03_31_21
SET ACPICA_GIT=https://github.com/acpica/acpica.git

IF NOT EXIST "scripts\git\acpica.cmd" (
    ECHO Error: does not look like a root of Dancy source tree 1>&2
    EXIT /B 1
)

MKDIR external > NUL 2>&1
MKDIR common\include\acpica > NUL 2>&1
MKDIR common\include\acpica\platform > NUL 2>&1
MKDIR kernel\acpica > NUL 2>&1

IF NOT EXIST "external\acpica" (
    git.exe clone %ACPICA_GIT% external\acpica
    git.exe -C external\acpica checkout %ACPICA_TAG%
    bin\dy-patch.exe -p1 -i kernel\acpios\patches\acenv
    bin\dy-patch.exe -p1 -i kernel\acpios\patches\acenvex
    bin\dy-patch.exe -p1 -i kernel\acpios\patches\rsdump
)

FOR /F %%i IN ("%1") DO SET ACPICA_EXT=%%~xi
FOR /F %%i IN ("%1") DO SET ACPICA_FILE=%%~nxi

SET ACPICA_H=external\acpica\source\include
SET ACPICA_C=external\acpica\source\components

IF "%ACPICA_EXT%" == ".h" (
    FOR /F %%i IN ('DIR "%ACPICA_H%\*.h" /S /B ^| FIND "%ACPICA_FILE%"') DO (
        SET ACPICA_SOURCE=%%i
    )
)

IF "%ACPICA_EXT%" == ".c" (
    FOR /F %%i IN ('DIR "%ACPICA_C%\*.c" /S /B ^| FIND "%ACPICA_FILE%"') DO (
        SET ACPICA_SOURCE=%%i
    )
)

IF NOT "%ACPICA_SOURCE%" == "" (
    TYPE "%ACPICA_SOURCE%" > %1
)

ENDLOCAL
