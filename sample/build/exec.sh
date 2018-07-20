#! /bin/bash

g++ -c ../src/program.cpp

g++ program.o /usr/local/lib/libespeak-ng.so -lpulse-simple -lpulse -llept -ltesseract -lraspicam -o main
#./main ../img/arabic2.png
