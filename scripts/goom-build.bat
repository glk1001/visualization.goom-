@ECHO OFF

SETLOCAL

SET SCRIPT_PATH=%~dp0
SET VIS_GOOM_ROOT_PATH=%SCRIPT_PATH%\..

IF "%~1" == "" (
  ECHO BUILD_TYPE must be specified.
  Exit /b 1
)
SET BUILD_TYPE=%1
IF "%~2" == "" (
  ECHO Suffix must be specified.
  Exit /b 1
)
SET SUFFIX=%2
SET BUILD_DIR=build-cl-%BUILD_TYPE%-%SUFFIX%

PUSHD %VIS_GOOM_ROOT_PATH%
IF ErrorLevel 1 Exit /b 1

cmake --build %BUILD_DIR% --config %BUILD_TYPE%

cmake --install %BUILD_DIR%

POPD
