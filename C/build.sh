# build -> snake.c
gcc -Os -s snake.c -o snake -lgdi32 -luser32 -mwindows "-Wl,--strip-all" -march=native -mtune=native