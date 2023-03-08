@ECHO OFF

SETLOCAL

SET SCRIPT_PATH=%~dp0
SET VIS_GOOM_ROOT_PATH=%SCRIPT_PATH%\..
SET VIS_GOOM_PARENT=%VIS_GOOM_ROOT_PATH%\..

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

RMDIR /Q/S %BUILD_DIR%
MKDIR %BUILD_DIR%

CD %BUILD_DIR%

cmake -DADDONS_TO_BUILD=visualization.goom-pp ^
      -DADDON_SRC_PREFIX=%VIS_GOOM_PARENT% ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX=%VIS_GOOM_PARENT%\kodi\kodi-build\addons ^
      -DPACKAGE_ZIP=1 ^
      -DBUILD_ARGS_ext="-DENABLE_TESTING=1;-DUSE_MAGIC_ENUM=0" ^
      %VIS_GOOM_PARENT%\kodi\cmake\addons

POPD
