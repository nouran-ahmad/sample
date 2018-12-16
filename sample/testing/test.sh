#! /bin/bash

g++ -c ocrTest.cpp

g++ ocrTest.o -llept -ltesseract -lraspicam -lraspicam_cv \
-lopencv_core  -lopencv_imgcodecs -lopencv_objdetect  -o main

for i in 12 16 24
#for i in 12 14 16 18 24
do
#./main $i set1 100
./main $i set1 18892
ret=$?
	if [ $ret -ne 0 ]; then
       exit $ret
	fi
done
