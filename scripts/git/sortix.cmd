@ECHO OFF
SETLOCAL

SET SORTIX_TAG=b99f6211411689645e5cdf21d99d5f308560e9c8
SET SORTIX_GIT=https://gitlab.com/sortix/sortix.git

IF NOT EXIST "scripts\git\sortix.cmd" EXIT /B 1

IF NOT EXIST "external\sortix" (
    mkdir "external" > NUL 2>&1
    git.exe clone %SORTIX_GIT% "external\sortix"
    git.exe -C "external\sortix" checkout %SORTIX_TAG%
)

ENDLOCAL
