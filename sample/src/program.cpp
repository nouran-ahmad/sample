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
#include <raspicam/raspicam.h>
#include <ctime>
#include <unistd.h>
#include <opencv2/opencv.hpp>


tesseract::TessBaseAPI *api;
raspicam::RaspiCam Camera; 

int Buflength = 10000, Options = 0;
void* userData;
t_espeak_callback *SynthCallback;
char *text;
unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO,
		*uniqueIdentifier=NULL;
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

void cleanupMemory(char *outText, Pix *image) {
	// Destroy used object and release memory
	api->End();
	delete[] outText;
	pixDestroy(&image);
	if (pulseAudio)
		pa_simple_free(pulseAudio);
}

void setupTesseract() {
	api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with Arabic, with specifying tessdata path
	printf("Start tesseract Initialize:\n");
	if (api->Init("/home/pi/Desktop/project/dependencies/tesseract/tessdata",
			"ara")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}
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
	espeak_SetVoiceByName("mb-ar1");
	espeak_SetParameter(espeakVOICETYPE, 1, 0);
	espeak_SetParameter(espeakPITCH, 40, 0);
	espeak_SetParameter(espeakRATE, 110, 0);
	espeak_SetSynthCallback(synthesisCallback);
}

void setupRaspicam(){
	
	Camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
	//Camera.setEncoding ( raspicam::RASPICAM_ENCODING_PNG );
	Camera.setWidth(640);
    Camera.setHeight(480);
    Camera.setBrightness(70);
	Camera.setSharpness(100);
    Camera.setContrast(100);
	
	if ( !Camera.open() ) {
		printf("Error opening camera");
		exit(1);
	}
	
	sleep(1);    
}

void captureImage(){
	printf("==== start capturing image ====\n");
	
	Camera.startCapture();
	if(!Camera.grab()) {
	    printf("Camera buffer grabbing failed!\n");
	}
	
	int imageSize = Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_GRAY);
	unsigned char *data=new unsigned char[imageSize];
	Camera.retrieve(data);
	
	std::ofstream outFile("test.pgm",std::ios::binary);
    outFile<<"P5\n"<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
    outFile.write(( char* )data,imageSize);
	outFile.close();
	printf("==== image saved to test.pgm ====\n");
	
	}

Mat deskew(double angle, Mat img){
  
  std::vector<cv::Point> points;
  cv::Mat_<uchar>::iterator it = img.begin<uchar>();
  cv::Mat_<uchar>::iterator end = img.end<uchar>();
  for (; it != end; ++it)
    if (*it)
      points.push_back(it.pos());
 
  cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
  cv::Mat rot_mat = cv::getRotationMatrix2D(box.center, angle, 1);
  cv::Mat rotated;
  cv::warpAffine(img, rotated, rot_mat, img.size(), cv::INTER_CUBIC);
  
  cv::Size box_size = box.size;
  if (box.angle < -45.)
    std::swap(box_size.width, box_size.height);
  cv::Mat cropped;
  cv::getRectSubPix(rotated, box_size, box.center, cropped);
   
  bitwise_not(cropped, cropped);
 
  return cropped;
  }

double compute_skew(Mat src) {
   
   //AdaptiveThreshold(src,src,255,ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,13,0);

   cv::Size size = src.size();
   cv::bitwise_not(src, src);
   std::vector<cv::Vec4i> lines;
   cv::HoughLinesP(src, lines, 1, CV_PI/180, 100, size.width / 2.f, 20);
   cv::Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
   double angle = 0.;
   unsigned nb_lines = lines.size();
   for (unsigned i = 0; i < nb_lines; ++i) {
        cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
                 cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 0 ,0));
        angle += atan2((double)lines[i][3] - lines[i][1],
                       (double)lines[i][2] - lines[i][0]);
    }
  angle /= nb_lines; // mean angle, in radians.
  printf("skew angle in degrees: %f \n", angle * 180 / CV_PI );
  return angle * 180 / CV_PI;
}

void imageProcessing(char* argv[]){
	cv::Mat src = cv::imread(argv[1], 0);
	// convert to binary
	cv::threshold(src, src, 127, 255, cv::THRESH_BINARY);
   
	double degrees = compute_skew(src);
	Mat result= deskew(degrees, src);
    imwrite( argv[2], result );	
	}

int main(int argc, char* argv[]) {

	setupPulseAudio();
	//setupRaspicam();
	setupTesseract();
	setupEspeak();
	
	//captureImage();
	//imageProcessing();
  
	Pix *image = pixRead(argv[1]);
	api->SetImage(image);
	char *text;
	text = api->GetUTF8Text();
	printf("OCR output:\n%s", text);

	unsigned int size = strlen(text) + 1;
	espeak_POSITION_TYPE positionType = POS_WORD;
		
	espeak_Synth(text, size, position, positionType, endPosition, flags,	uniqueIdentifier, userData);
	espeak_Synchronize();
	espeak_Terminate();
	cleanupMemory(text, image);
	
    //Camera.release();
	//delete data;
	return 0;
}
