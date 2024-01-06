@echo off
set files=main.c gfx\gfx.c game\motion.c game\planner.c game\math\bezier.c game\math\mathutils.c
set libs=-l./lib/x64/SDL2 -l./lib/x64/SDL2main -l./lib/x64/SDL2_image
clang %files% -o auto.exe %libs%
auto figure.txt
