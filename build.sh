#! /bin/bash

gcc -c alsa.c
gcc -c espeak-sample.c

gcc alsa.o espeak-sample.o /usr/lib/libespeak-ng.so -lasound  -o main
