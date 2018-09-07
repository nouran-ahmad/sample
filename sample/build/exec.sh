#! /bin/bash

g++ -c ../src/program.cpp

g++ program.o /usr/local/lib/libespeak-ng.so /usr/local/lib/libcurlpp.so.1.0.0 \
-lpulse-simple -lpulse -llept -ltesseract -lraspicam \
-lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_highgui -lcurl -o main

# find / | grep search
#./main ../img/arabic2.png
