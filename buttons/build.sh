#!/bin/bash

CFLAGS="-I../src -Wall -std=c11 -pedantic `sdl2-config --libs` -lm -O0 -g"
gcc buttons.c renderer.c microui.c $CFLAGS

