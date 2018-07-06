#! /bin/bash

#g++ -c ../src/alsa.cpp
g++ -c ../src/program.cpp

g++ program.o /usr/local/lib/libespeak-ng.so -lpulse-simple -lpulse -llept -ltesseract -o main
#./main
