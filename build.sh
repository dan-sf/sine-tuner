#!/bin/bash

CFLAGS="-Wall -std=c11 -pedantic `sdl2-config --libs` -lm -O0 -g -o sine_tuner"
gcc main.c $CFLAGS

