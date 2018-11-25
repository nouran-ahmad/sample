#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <fstream>
#include <raspicam/raspicam_cv.h>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <opencv2/core.hpp>
 
tesseract::TessBaseAPI *api;
void* userData;

using namespace std;
using namespace cv;

void freeApi() {
	api->End();
}

void setupTesseract() {
	api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with Arabic, with specifying tessdata path
	printf("Tesseract Init:\n");
	if (api->Init("/home/pi/Desktop/project/sample/dependencies/tesseract/tessdata","ara")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}
	//PSM_AUTO_OSD 		PSM_SINGLE_LINE 
	//PSM_SPARSE_TEXT 	PSM_RAW_LINE PSM_AUTO PSM_SINGLE_WORD
	api->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
}

	
void ocr(Mat img){
	auto start = std::chrono::system_clock::now();
	
	api->SetImage((uchar*)img.data, img.size().width, img.size().height, 
	img.channels(), img.step1());
	char *text = api->GetUTF8Text();
	
	auto ocrEnd = std::chrono::system_clock::now();
	chrono::duration<double> diff = ocrEnd - start;
	//string resultText = string(text);
	cout<<text<<" response time is "<< diff.count()<< " s\n";
	
	delete[] text;
}

	
int main(int argc, char* argv[]) {

	setupTesseract();
	//for(int i=1;i<=3;i++){
		Mat image = imread(argv[1], cv::IMREAD_GRAYSCALE);
		ocr(image);
		image.release();
	//}
	freeApi();
    
	return 0;
}



