#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/sample.h>
#include <iostream>
#include <fstream>
#include <raspicam/raspicam_cv.h>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <wiringPi.h> 
 
tesseract::TessBaseAPI *api;
raspicam::RaspiCam_Cv Camera;
 
int Buflength = 10000, Options = 0;
void* userData;
t_espeak_callback *SynthCallback;

pa_simple *pulseAudio = NULL;

using namespace std;
using namespace cv;

int synthesisCallback(short* waveData, int sampleCount, espeak_EVENT* event) {

	printf("Synthesis callback received: sampleCount: %d :: %d\r\n",
			sampleCount, event->type);
	if (waveData) {
		int error;
		int e = pa_simple_write(pulseAudio, (char*) waveData, sampleCount * 2, &error);
		int e2 = pa_simple_drain(pulseAudio, &error);
	}
}

void freeApi() {
	espeak_Terminate();
	api->End();
	if (pulseAudio)
		pa_simple_free(pulseAudio);
}

void setupTesseract() {
	api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with Arabic, with specifying tessdata path
	printf("Tesseract Init:\n");
	if (api->Init("/home/pi/Desktop/project/sample/dependencies/tesseract/tessdata","ara")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}
	// PSM_AUTO_OSD
	//PSM_AUTO_OSD 		PSM_SINGLE_LINE 
	//PSM_SPARSE_TEXT 	PSM_RAW_LINE PSM_AUTO
	api->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
}

void setupPulseAudio() {
	int error;
	const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16LE,
		.rate = 22050, //suitable for espeak
		.channels = 1
	};
	/* connect to server using new playback stream */
	if (!(pulseAudio = pa_simple_new(NULL, "name", PA_STREAM_PLAYBACK, NULL,
			"playback", &ss, NULL, NULL, &error))) {
		fprintf(stderr, ": connecting to pulse audio failed\n");
		exit(1);
	}
}

void setupEspeak() {
	espeak_AUDIO_OUTPUT output = AUDIO_OUTPUT_RETRIEVAL;
	espeak_Initialize(output, Buflength, "/usr/local/share/espeak-ng-data",	Options);
	espeak_SetParameter(espeakVOICETYPE, 1, 0);
	espeak_SetParameter(espeakRATE, 120, 0);
	espeak_SetVoiceByName("mb-ar2");
	espeak_SetSynthCallback(synthesisCallback);
}

void setupRaspicam(){
	Camera.set(CAP_PROP_FORMAT, CV_8UC1 ); //gray
	Camera.set(CAP_PROP_FRAME_WIDTH, 640);
	Camera.set(CAP_PROP_FRAME_HEIGHT, 480);
	if ( !Camera.open() ) {
		printf("Error opening camera");
		exit(1);
	}
	sleep(3);    
}

Mat captureImage(){
	printf("====> capturing image\n");
	if(!Camera.grab()) {
	    printf("Camera buffer grabbing failed!\n");
	}
	Mat image;
	Camera.retrieve(image);
	return image;
	}

//Mat imageProcessing(Mat src){
	//// convert to binary ADAPTIVE_THRESH_GAUSSIAN_C or ADAPTIVE_THRESH_MEAN_C
	//adaptiveThreshold(src, src, 255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,15,5);
	//bitwise_not(src, src);
	//return src;	
	//}

string diacritizeText(string text){
	std::ostringstream os;
    string textUrlEncoded = curlpp::escape(text); 
    string url = string("http://127.0.0.1:8080/ajaxGet?resultType=text&text=")
    +textUrlEncoded+"&action=TashkeelText&lastmark=0";
    curlpp::options::Url myUrl(url);
    curlpp::Easy myRequest;
    myRequest.setOpt(myUrl);
    myRequest.perform();  
    os << myRequest;
    string result = os.str();
    return result;
	}


string getResultText(char *text){
		string resultText;
		if(strlen(text) == 0)
		{
			printf("No text detected\n");
			resultText = "لا يوجد نص";
		}else {
			resultText = diacritizeText(string(text));
			ofstream log;
			log.open("../img/ocrlog.txt",std::ofstream::out | std::ofstream::app);
			log<<resultText.c_str();
			log.close();
		}
	return resultText;
	}
	
string ocr(Mat img){
	api->SetImage((uchar*)img.data, img.size().width, img.size().height, 
	img.channels(), img.step1());
	char *text = api->GetUTF8Text();
	string result = getResultText(text);
	delete[] text;
	return result;
}
	
void imageToSpeech(Mat img){
	printf("===> image To speech \n");
	string resultText = ocr(img);
		
	espeak_POSITION_TYPE positionType = POS_WORD;
	unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO;
	espeak_Synth(resultText.c_str(), resultText.size()+1, position, 
	positionType, endPosition, flags, NULL, userData);
	espeak_Synchronize();
}

int main(int argc, char* argv[]) {

	setupPulseAudio();
	wiringPiSetup();
	setupRaspicam();
	setupTesseract();
	setupEspeak();
	
	int pinNumber = 16;
	int led = 0;
	pinMode(pinNumber, INPUT);
	while(true){
			while(int status = digitalRead(pinNumber) != 0){
			status = digitalRead(pinNumber);
			printf("====> Waiting for input = %d\n", status);
			sleep(1);
		}
		sleep(2);
		Mat image = captureImage();
		imwrite(argv[1], image);
		printf("====> image saved\n");
		//Mat image = imread(argv[1], cv::IMREAD_GRAYSCALE);
		//Mat enhancedImage = imageProcessing(image);
		//imwrite(argv[2], enhancedImage);
		imageToSpeech(image);
	}
	freeApi();
    Camera.release();
	return 0;
}



