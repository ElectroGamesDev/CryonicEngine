@echo off

echo Copying api files
robocopy "%~1Engine" "%~2api" /MIR /NFL /NDL /NJH /NJS /NC /NS
IF ERRORLEVEL 8 (
    echo ERROR: Failed to copy Engine to api
    EXIT /B 8
)

echo Copying resource files
robocopy "%~1Engine\Resources" "%~2resources" /MIR /NFL /NDL /NJH /NJS /NC /NS
IF ERRORLEVEL 8 (
    echo ERROR: Failed to copy Engine\Resources to resources
    EXIT /B 8
)

robocopy "%~1Editor\Resources" "%~2resources" /MIR /NFL /NDL /NJH /NJS /NC /NS
IF ERRORLEVEL 8 (
    echo ERROR: Failed to copy Editor\Resources to resources
    EXIT /B 8
)

echo Copying build files
robocopy "%~1Build" "%~2BuildPresets" /MIR /NFL /NDL /NJH /NJS /NC /NS
IF ERRORLEVEL 8 (
    echo ERROR: Failed to copy Build to BuildPresets
    EXIT /B 8
)

echo Copying build tools
robocopy "%~1BuildTools" "%~2tools" /MIR /NFL /NDL /NJH /NJS /NC /NS
IF ERRORLEVEL 8 (
    echo ERROR: Failed to copy BuildTools to tools
    EXIT /B 8
)

EXIT /B 0
