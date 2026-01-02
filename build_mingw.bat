@echo off
REM MinGW Build Script for Neon Swarm Game (64-bit)

echo Building with MinGW 64-bit...

if not exist "build" mkdir build

REM Download raylib if needed
if not exist "raylib-5.5" (
    echo Downloading raylib 5.5...
    powershell -Command "(New-Object System.Net.ServicePointManager).SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor [System.Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz' -OutFile 'raylib.tar.gz'"
    
    if exist "raylib.tar.gz" (
        echo Extracting raylib...
        powershell -Command "tar -xzf 'raylib.tar.gz'"
        del raylib.tar.gz
    ) else (
        echo Failed to download raylib
        exit /b 1
    )
)

REM Build raylib
if not exist "build\libraylib.a" (
    echo Building raylib from source, please wait...
    cd raylib-5.5\src
    mingw32-make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_43
    cd ..\..
    
    if exist "raylib-5.5\src\libraylib.a" (
        copy raylib-5.5\src\libraylib.a build\
        echo raylib built successfully
    ) else (
        echo Failed to build raylib
        exit /b 1
    )
)

REM Compile the game
echo Compiling game...
g++ -std=c++17 -O2 -I raylib-5.5/src src/main.cpp src/menu.cpp src/settings.cpp src/game.cpp src/bullets.cpp src/enemy.cpp src/background.cpp -L build -o build/NeonSwarmGame.exe -l raylib -l opengl32 -l winmm -l gdi32 -l user32 -l shell32

if exist "build\NeonSwarmGame.exe" (
    echo.
    echo Build successful!
    echo Run: build\NeonSwarmGame.exe
) else (
    echo Build failed
    exit /b 1
)

if exist "build\NeonSwarmGame.exe" (
    echo.
    echo Build successful!
    echo Run: build\NeonSwarmGame.exe
) else (
    echo.
    echo Build failed
    exit /b 1
)
