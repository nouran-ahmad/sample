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


void setupTesseract() {
	api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with Arabic, with specifying tessdata path
	printf("Tesseract Init:\n");
	if (api->Init("/home/pi/Desktop/project/sample/dependencies/tesseract/tessdata","ara")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	} 		
	//PSM_SINGLE_LINE 
	//PSM_SPARSE_TEXT 	PSM_RAW_LINE PSM_AUTO PSM_SINGLE_WORD
	api->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
}


void log (string expectedWord,string actual){
ofstream log;
	log.open("./ocrTestlog.txt");
	log<<expectedWord.c_str()<<"\n";
	log<<actual.c_str()<<"\n";
	log.close();
}
	
string ocr(Mat img){
	//auto start = std::chrono::system_clock::now();
	
	api->SetImage((uchar*)img.data, img.size().width, img.size().height, 
	img.channels(), img.step1());
	char *text = api->GetUTF8Text();
	
	//auto ocrEnd = std::chrono::system_clock::now();
	//chrono::duration<double> diff = ocrEnd - start;
	//cout<<text<<" response time is "<< diff.count()<< " s\n";
	
	return string(text);
}

string readFile(string fileName){
	std::ifstream file(fileName);
	string str;
	string file_contents;
	while (std::getline(file, str))
	{
	  file_contents += str;
	  file_contents.push_back('\n');
	} 
	return file_contents;
}

string getExpectedWord(string xmlfile){

	string file_contents = readFile(xmlfile);
	char *cstr = new char[file_contents.length() + 1];
	strcpy(cstr, file_contents.c_str());
	xml_document<> doc;    
	doc.parse<0>(cstr);
	
	xml_node<> *node = doc.first_node("wordImage");
	xml_node<> *content=node->first_node();
	xml_attribute<> *attr = content->first_attribute();
	string expectedWord = string(attr->value());
	return expectedWord;	
}

int calculateEditCost(string expectedWord, string actual){
	int cost=0;
	int expWordSize=expectedWord.size();
	if(actual.compare(expectedWord) != 0){
		
		int size = expWordSize;
		if(expWordSize > actual.size()){
			size = actual.size(); //assign the smaller size
			cost += expWordSize - actual.size();
		}else{
			//cost += actual.size() - expectedWord.size();
		}
		
		for (int i = 0; i < size; ++i){
			if(actual[i] != expectedWord[i])
				cost++;
		}
	}
	return cost;
}

void writeFile (string content){
ofstream log;
	log.open("./ocrTestlog2.txt",std::ofstream::out | std::ofstream::app);
	log<<content.c_str()<<"\n";
	log.close();
}

int main(int argc, char* argv[]) {
	auto startTime = std::chrono::system_clock::now();
	setupTesseract();
	string font = "Traditional Arabic";
	string size= argv[1];
	string set= argv[2];
	string basePath = "/home/pi/Desktop/project/apti-imgs/"+font+"/";
	string pathXml= basePath+size+"/xml/"+set+"/Image_"+size+"_"+font+"_";
	string pathImg= basePath+size+"/imgs/"+set+"/Image_"+size+"_"+font+"_";

	int end= atoi(argv[3]);
	
	unsigned int numberOfChars=0;
	unsigned int costSum=0;
	double wordError=0;
	double words=0;
	for(int i=0; i<end; i++){		
		string xmlfile = pathXml+to_string(i)+".xml";
		string imgfile = pathImg+to_string(i)+".png";
		
		cout<<"image "<<i<<"\n";	
		string expectedWord = getExpectedWord(xmlfile);
		Mat image = imread(imgfile, cv::IMREAD_GRAYSCALE);
		string actual = ocr(image);
		image.release();
		
		numberOfChars +=expectedWord.size();
		int cost= calculateEditCost(expectedWord, actual);
		if(cost > 1)
			wordError++;
		words++;
		costSum +=cost;
	}
	double totalchars = numberOfChars;
	double totalCost = costSum;
	
	double accuracy= (totalchars - totalCost)/ totalchars * 100;
	double wordAccuracy= (words - wordError)/ words * 100;
	
	string content= font+ " size: "+ size +"\n"+
	"word accuracy = "+ to_string(wordAccuracy) + "% \n"
	"accuracy = "+to_string(accuracy)+"% \n ================= \n";
	writeFile(content);
	
	cout<<"word accuracy= "<<wordAccuracy<<"\n";
	cout<<"accuracy= "<<accuracy<<"\n";
	api->End();
	
    auto ocrEnd = std::chrono::system_clock::now();
	chrono::duration<double> diff = ocrEnd - startTime;
	cout<<"response time is "<< diff.count()<< " s\n";
    
	return 0;
}



