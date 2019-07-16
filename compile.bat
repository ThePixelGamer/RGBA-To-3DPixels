g++ -std=gnu++17 -I ./build/ -o build/test.exe *.cpp -O2 -Wall -Wextra -pedantic -lmingw32 -lgdi32 -lpng -lz && cd build && test.exe

@echo off

pause