#! /bin/bash

gcc -c alsa.c
gcc -c espeak-sample2.c

gcc alsa.o espeak-sample2.o /usr/lib/libespeak-ng.so -lasound  -o main
