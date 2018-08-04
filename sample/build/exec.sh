#! /bin/bash

g++ -c ../src/program.cpp

g++ program.o /usr/local/lib/libespeak-ng.so \
-lpulse-simple -lpulse -llept -ltesseract -lraspicam \
-lopencv_core -lopencv_imgproc -lopencv_objdetect -o main

#./main ../img/arabic2.png
