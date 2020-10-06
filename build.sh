#!/bin/bash

CFLAGS="-Wall -pedantic -std=c11 `sdl2-config --libs` -O0 -g -o sine_tuner"
gcc src/main.c $CFLAGS

