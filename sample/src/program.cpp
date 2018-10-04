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
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
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
void cleanUp( Pix *image){
	pixDestroy(&image);
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
	espeak_SetParameter(espeakPITCH, 20, 0);
	espeak_SetParameter(espeakRATE, 110, 0);
	espeak_SetVoiceByName("mb-ar2");
	espeak_SetSynthCallback(synthesisCallback);
}

void setupRaspicam(){
	Camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
	//Camera.set( CV_CAP_PROP_CONTRAST, 60 );
	//Camera.setEncoding ( raspicam::RASPICAM_ENCODING_PNG );
	//Camera.setWidth(640);
    //Camera.setHeight(480);
    //Camera.setBrightness(70);
	//Camera.setSharpness(100);
	
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

double compute_skew(Mat src) {
   
   cv::Size size = src.size();
   bitwise_not(src, src);
   std::vector<cv::Vec4i> lines;
   HoughLinesP(src, lines, 1, CV_PI/180, 100, size.width / 2.f, 20);
   Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
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


Mat captureImage(){
	printf("==== capturing image ====\n");
	if(!Camera.grab()) {
	    printf("Camera buffer grabbing failed!\n");
	}
	Mat image;
	Camera.retrieve(image);
	return image;
	}

Mat imageProcessing(Mat src){
	//Mat src = imread(fileName, 0);
	// convert to binary ADAPTIVE_THRESH_GAUSSIAN_C or ADAPTIVE_THRESH_MEAN_C
	adaptiveThreshold(src,src,255,ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,11,5);
	//threshold(src, src, 127, 255, cv::THRESH_BINARY);
    return src;
   
	//double degrees = compute_skew(src);
	//if((0.00 < degrees < 1.00 )|| (0.00 > degrees > -1.00))
		//return src;
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
    cout<<"\n";
    cout<<result;
    cout<<"\n";
    return result;
	}

Pix* matToPix(Mat src){
	Pix *image = pixCreate(src.size().width, src.size().height, 8);

	for(int i=0; i<src.rows; i++) 
		for(int j=0; j<src.cols; j++) 
			pixSetPixel(image, j,i, (l_uint32) src.at<uchar>(i,j));
	return image;
	}

void imageToSpeech(Mat img){
	printf("image To speech \n");
	api->SetImage((uchar*)img.data, img.size().width, img.size().height, 
	img.channels(), img.step1());
	char *text = api->GetUTF8Text();
    if(strlen(text) == 0)
    {
		printf("No text detected\n");
		return;
	}
	ofstream log;
	log.open("../img/ocrlog.txt");
	//log<<string(text);
	//string textWithDiacr = string(text);
    string textWithDiacr = diacritizeText(string(text));
    log<<textWithDiacr.c_str();
	log.close();
	
	espeak_POSITION_TYPE positionType = POS_WORD;
	unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO;
	espeak_Synth(textWithDiacr.c_str(), textWithDiacr.size(), position, 
	positionType, endPosition, flags,	NULL, userData);
	espeak_Synchronize();
	delete[] text;
}

Mat combineImages(Mat image1, Mat image2, Mat image3){
	vector<Mat> imgs;
    printf("imgs push\n");
	imgs.push_back(image3);
	imgs.push_back(image2);
	imgs.push_back(image1);
	Mat pano;
	Stitcher::Mode mode = Stitcher::PANORAMA;
    printf("create stiticher\n");
    Ptr<Stitcher> stitcher = Stitcher::create(mode, false);
    printf("before stitiching\n");
    Stitcher::Status status = stitcher->stitch(imgs, pano);
      if (status != Stitcher::OK)
    {
        cout << "Can't stitch images, error code = " << int(status) << endl;
    }
    return pano;
	}
	
int main(int argc, char* argv[]) {

	setupPulseAudio();
	wiringPiSetup();
	setupRaspicam();
	setupTesseract();
	setupEspeak();
	
	int pinNumber = 16;
	pinMode(pinNumber, INPUT);
	//while(true){
			
			//while(int status = digitalRead(pinNumber) != 0){
			//status = digitalRead(pinNumber);
			//printf("Waiting for input = %d\n", status);
			//sleep(1);
		//}
		sleep(1);
		//Mat img1 = imread(argv[1]);
		//Mat img2 = imread(argv[2]);
		//Mat img3 = imread(argv[3]);
		//Mat result = combineImages(img1,img2, img3);
		//imwrite(argv[4], result);
		//Mat enhancedImage = captureImage();
		//imwrite(argv[1], enhancedImage);
		Mat enhancedImage = imread(argv[1]);
		printf("start image pre processing \n");
		//Mat enhancedImage = imageProcessing(image);
		imageToSpeech(enhancedImage);
	//}
	freeApi();
    Camera.release();
	return 0;
}
