#! /bin/bash

g++ -c alsa.cpp
g++ -c program.cpp

g++ alsa.o program.o /usr/lib/libespeak-ng.so -lasound -llept -ltesseract -o main
#./main
