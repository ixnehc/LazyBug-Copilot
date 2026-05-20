@echo off
setlocal enabledelayedexpansion

:: Set source path and target directory
set "SOURCE_PATH=%~1"
set "TARGET_DIR=%~2"

:: Check if parameters are provided
if "%SOURCE_PATH%"=="" (
    echo Usage: %0 ^<source_path^> ^<target_directory^>
    echo Example: %0 "C:\path\to\file.txt" "D:\target\directory"
    echo Example: %0 "C:\path\to\folder" "D:\target\directory"
    pause
    exit /b 1
)

if "%TARGET_DIR%"=="" (
    echo Usage: %0 ^<source_path^> ^<target_directory^>
    echo Example: %0 "C:\path\to\file.txt" "D:\target\directory"
    echo Example: %0 "C:\path\to\folder" "D:\target\directory"
    pause
    exit /b 1
)

:: Check if the source path exists
if not exist "%SOURCE_PATH%" (
    echo Error: Source path does not exist: %SOURCE_PATH%
    pause
    exit /b 1
)

:: Check if the target directory exists
if not exist "%TARGET_DIR%" (
    echo Error: Target directory does not exist: %TARGET_DIR%
    pause
    exit /b 1
)

:: Get the source name (file or directory name)
for %%F in ("%SOURCE_PATH%") do set "SOURCENAME=%%~nxF"

:: Check if source is a file or directory
echo Source path: %SOURCE_PATH%
echo Target directory: %TARGET_DIR%
echo.

:: Counter
set /a COUNT=0

:: Check if source is a directory
if exist "%SOURCE_PATH%\*" (
    :: Source is a directory
    echo Source is a directory: %SOURCENAME%
    echo Finding and replacing directory contents...
    echo.
    
    :: Use a for loop to recursively find directories with the same name in the target directory
    for /r "%TARGET_DIR%" %%G in ("%SOURCENAME%") do (
        if exist "%%G\*" (
            echo Found directory: %%G
            echo Copying files from source to this directory...
            
            :: Copy all files from source directory to found directory
            xcopy /y /e /i "%SOURCE_PATH%\*" "%%G\" >nul 2>&1
            
            if !errorlevel! equ 0 (
                echo Successfully replaced directory contents: %%G
                set /a COUNT+=1
            ) else (
                echo Failed to replace directory contents: %%G
            )
            echo.
        )
    )
) else (
    :: Source is a file
    echo Source is a file: %SOURCENAME%
    echo Finding and replacing file...
    echo.
    
    :: Use a for loop to recursively find files with the same name in the target directory
    for /r "%TARGET_DIR%" %%G in ("%SOURCENAME%") do (
        if exist "%%G" (
            :: Check if it's a file (not a directory)
            if not exist "%%G\*" (
                echo Found file: %%G
                echo Replacing...
                copy /y "%SOURCE_PATH%" "%%G" >nul
                if !errorlevel! equ 0 (
                    echo Successfully replaced: %%G
                    set /a COUNT+=1
                ) else (
                    echo Failed to replace: %%G
                )
                echo.
            )
        )
    )
)

echo Operation completed!
echo Total items replaced: %COUNT%.
