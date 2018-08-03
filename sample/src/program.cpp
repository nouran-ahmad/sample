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

espeak_POSITION_TYPE positionType = POS_WORD;
espeak_AUDIO_OUTPUT output;
tesseract::TessBaseAPI *api;
raspicam::RaspiCam Camera; 

int Buflength = 10000, Options = 0;
void* userData;
t_espeak_callback *SynthCallback;
char *text;
unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO,
		*uniqueIdentifier=NULL;
using namespace std;
pa_simple *pulseAudio = NULL;

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
	output = AUDIO_OUTPUT_RETRIEVAL;
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

int main(int argc, char* argv[]) {

	setupPulseAudio();
	setupRaspicam();
	setupTesseract();
	setupEspeak();
	
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
	
	Pix *image = pixRead("test.pgm");
	api->SetImage(image);
	char *text;
	text = api->GetUTF8Text();
	printf("OCR output:\n%s", text);

	unsigned int size = strlen(text) + 1;
	espeak_Synth(text, size, position, positionType, endPosition, flags,	uniqueIdentifier, userData);
	espeak_Synchronize();
	espeak_Terminate();
	cleanupMemory(text, image);
	
    Camera.release();
	delete data;
	return 0;
}
