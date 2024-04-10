@echo off
cd raylib
mingw32-make PLATFORM=PLATFORM_DESKTOP RAYLIB_RELEASE_PATH=../
cd ..
gcc code/main.cpp -o game.exe libraylib.a -I raylib -L. -lraylib -lopengl32 -lgdi32 -lwinmm
