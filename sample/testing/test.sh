#! /bin/bash

g++ -c ocrTest.cpp

g++ ocrTest.o -llept -ltesseract -lraspicam -lraspicam_cv \
-lopencv_core  -lopencv_imgcodecs -lopencv_objdetect  -o main

