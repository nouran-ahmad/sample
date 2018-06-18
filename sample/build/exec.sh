#! /bin/bash

g++ -c ../src/alsa.cpp
g++ -c ../src/program.cpp

g++ alsa.o program.o /usr/lib/libespeak-ng.so -lpulse-simple -lpulse -lasound -llept -ltesseract -o main
#./main
