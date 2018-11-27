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
#include "rapidxml-1.13/rapidxml.hpp"
 
tesseract::TessBaseAPI *api;
void* userData;

using namespace std;
using namespace cv;
using namespace rapidxml;

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

	//setupTesseract();
	
	std::ifstream file("/home/pi/Desktop/project/apti-imgs/ArabicTransparent/xml/set1/Image_12_Arabic Transparent_0.xml");
	string str;
	string file_contents;
	while (std::getline(file, str))
	{
	  file_contents += str;
	  file_contents.push_back('\n');
	} 
	
	char *cstr = new char[file_contents.length() + 1];
	strcpy(cstr, file_contents.c_str());
	xml_document<> doc;    
	doc.parse<0>(cstr);
	
	xml_node<> *node = doc.first_node("wordImage");
	xml_node<> *content=node->first_node();
	xml_attribute<> *attr = content->first_attribute();
	string word=attr->value();
	cout<<"word is "<<word<<"\n";
	
	//for(int i=1;i<=3;i++){
		//Mat image = imread(argv[1], cv::IMREAD_GRAYSCALE);
		//ocr(image);
		//image.release();
	//}
	//freeApi();
    
	return 0;
}



