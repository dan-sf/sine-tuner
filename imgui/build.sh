#!/bin/bash

CFLAGS="-Wall -std=c11 -pedantic `sdl2-config --libs` -lm -O0 -g -o main"
gcc main.c $CFLAGS

