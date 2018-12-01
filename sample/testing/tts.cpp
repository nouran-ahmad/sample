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
//#include <opencv2/stitching.hpp>
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
	espeak_Initialize(output, Buflength, "/usr/local/share/espeak-ng-data",
			Options);
	espeak_SetParameter(espeakVOICETYPE, 1, 0);
	espeak_SetParameter(espeakRATE, 120, 0);
	espeak_SetVoiceByName("mb-ar2");
	espeak_SetSynthCallback(synthesisCallback);
}

void setupRaspicam(){
	Camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 ); //gray
	Camera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	Camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	if ( !Camera.open() ) {
		printf("Error opening camera");
		exit(1);
	}
	
	sleep(3);    
}


Mat deskew(double angle, Mat img){
  
  std::vector<cv::Point> points;
  cv::Mat_<uchar>::iterator it = img.begin<uchar>();
  cv::Mat_<uchar>::iterator end = img.end<uchar>();
  for (; it != end; ++it)
    if (*it)
      points.push_back(it.pos());
 
  RotatedRect box = minAreaRect(cv::Mat(points));
  
  Mat rot_mat = getRotationMatrix2D(box.center, angle, 1);
  Mat rotated;
  warpAffine(img, rotated, rot_mat, img.size(), INTER_CUBIC);
  
  Size box_size = box.size;
  if (box.angle < -45.)
    std::swap(box_size.width, box_size.height);
  Mat cropped;
  getRectSubPix(rotated, box_size, box.center, cropped);
   
  bitwise_not(cropped, cropped);
 
  return cropped;
  }

double compute_skew(Mat img) {
   
   cv::Size size = img.size();
   bitwise_not(img, img);
   
   //Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 3));
   //cv::erode(img, img, element);
   std::vector<cv::Point> points;
   Mat_<uchar>::iterator it = img.begin<uchar>();
   Mat_<uchar>::iterator end = img.end<uchar>();
   for (; it != end; ++it)
     if (*it)
       points.push_back(it.pos());
   
   RotatedRect box = cv::minAreaRect(cv::Mat(points));
   double angle = box.angle;
   if (angle < -45.)
     angle += 90.;
     
   printf("skew angle in degrees: %f \n", angle  );
   return angle;
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

Mat imageProcessing(Mat src){
	// convert to binary ADAPTIVE_THRESH_GAUSSIAN_C or ADAPTIVE_THRESH_MEAN_C
	adaptiveThreshold(src, src, 255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,15,5);
	bitwise_not(src, src);
	//threshold(src, src, 150, 255, cv::THRESH_BINARY);
    //return src;
    
	//double degrees = compute_skew(src);
	//if((0.00 < degrees < 1.00 )|| (0.00 > degrees > -1.00))
		return src;
	//Mat result= deskew(-15.00, src);
    //return result;	
	}

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

Pix* matToPix(Mat src){
	Pix *image = pixCreate(src.size().width, src.size().height, 8);

	for(int i=0; i<src.rows; i++) 
		for(int j=0; j<src.cols; j++) 
			pixSetPixel(image, j,i, (l_uint32) src.at<uchar>(i,j));
	return image;
	}


//Mat combineImages(Mat image1, Mat image2, Mat image3){
	//vector<Mat> imgs;
    //printf("imgs push\n");
	//imgs.push_back(image3);
	//imgs.push_back(image2);
	//imgs.push_back(image1);
	//Mat pano;
	//Stitcher::Mode mode = Stitcher::PANORAMA;
    //printf("create stiticher\n");
    //Ptr<Stitcher> stitcher = Stitcher::create(mode, false);
    //printf("before stitiching\n");
    //Stitcher::Status status = stitcher->stitch(imgs, pano);
      //if (status != Stitcher::OK)
    //{
        //cout << "Can't stitch images, error code = " << int(status) << endl;
    //}
    //return pano;
	//}
	
	string getResultText(char *text){
		string resultText;
		if(strlen(text) == 0)
		{
			printf("No text detected\n");
			resultText = "لا يوجد نص";
		}else {
			ofstream log;
			log.open("../img/ocrlog.txt");
			resultText = diacritizeText(string(text));
			log<<resultText.c_str();
			log.close();
		}
	return resultText;
	}
	
	void imageToSpeech(Mat img){
	printf("===> image To speech \n");
	auto start = std::chrono::system_clock::now();
	api->SetImage((uchar*)img.data, img.size().width, img.size().height, 
	img.channels(), img.step1());
	char *text = api->GetUTF8Text();
	auto ocrEnd = std::chrono::system_clock::now();
	chrono::duration<double> diff = ocrEnd - start;
	cout<<"response time is "<< diff.count()<< "s\n";
	
	string resultText = string(text);
	//string resultText = getResultText(text);
		
	espeak_POSITION_TYPE positionType = POS_WORD;
	unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO;
	espeak_Synth(resultText.c_str(), resultText.size()+1, position, 
	positionType, endPosition, flags,	NULL, userData);
	espeak_Synchronize();
	
	auto ttsEnd = std::chrono::system_clock::now();
	std::chrono::duration<double> totalDuration = ttsEnd - start;
	cout<<"total response time is "<< totalDuration.count()<< "s\n";
	
	delete[] text;
}

	
int main(int argc, char* argv[]) {

	setupPulseAudio();
	wiringPiSetup();
	//setupRaspicam();
	setupTesseract();
	setupEspeak();
	
	int pinNumber = 16;
	int led = 0;
	pinMode(pinNumber, INPUT);
	//while(true){
			//while(int status = digitalRead(pinNumber) != 0){
			//status = digitalRead(pinNumber);
			//printf("====> Waiting for input = %d\n", status);
			//sleep(1);
		//}
		//sleep(3);
		//Mat image = captureImage();
		//imwrite(argv[1], image);
		printf("====> image saved\n");
		Mat image = imread(argv[1], cv::IMREAD_GRAYSCALE);
		//Mat enhancedImage = imageProcessing(image);
		//imwrite(argv[2], enhancedImage);
		imageToSpeech(image);
	//}
	freeApi();
    //Camera.release();
	return 0;
}



