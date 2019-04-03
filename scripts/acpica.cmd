@ECHO OFF
SETLOCAL

SET ACPICA_TAG=R02_15_19
SET ACPICA_GIT=https://github.com/acpica/acpica.git

IF NOT EXIST "scripts\acpica.cmd" (
    ECHO Error: does not look like a root of Dancy source tree 1>&2
    EXIT /B 1
)

MKDIR external > NUL 2>&1
MKDIR include\acpica > NUL 2>&1
MKDIR include\acpica\platform > NUL 2>&1
MKDIR kernel\acpica > NUL 2>&1

IF NOT EXIST "external\acpica" (
    git.exe clone %ACPICA_GIT% external\acpica
    git.exe -C external\acpica checkout %ACPICA_TAG%
    RMDIR /S /Q external\acpica\.git
    RMDIR /S /Q external\acpica\generate
    RMDIR /S /Q external\acpica\tests
    DEL external\acpica\.gitignore
    DEL external\acpica\Makefile
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
    COPY /Y "%ACPICA_SOURCE%" %1 > NUL 2>&1
)

ENDLOCAL
