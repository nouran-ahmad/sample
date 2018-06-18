#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include "alsa.h"
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/sample.h>

#define BUFSIZE 1024

espeak_POSITION_TYPE position_type;
espeak_AUDIO_OUTPUT output;
tesseract::TessBaseAPI *api;

char *path = NULL;
int Buflength = 500, Options = 0;
void* user_data;
t_espeak_callback *SynthCallback;
espeak_PARAMETER Parm;
char *text;
unsigned int Size, position = 0, end_position = 0, flags = espeakCHARS_AUTO,
		*unique_identifier;

pa_simple *pulseAudio = NULL;

int synthesisCallback(short* waveData, int sampleCount, espeak_EVENT* event) {

	printf("Synthesis callback received: sampleCount: %d :: %d\r\n",
			sampleCount, event->type);
	if (waveData) {
		int error;
		pa_simple_write(pulseAudio, (char*) waveData, sampleCount * 2, &error);
		pa_simple_drain(pulseAudio, &error);
		//		playback(waveData, sampleCount);
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
	if (api->Init("/home/pi/Desktop/project/sample/tesseract-example/tessdata",
			"ara")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}
	printf("End tesseract Initialize:\n");
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

int main(int argc, char* argv[]) {

	setupPulseAudio();
	printf("Starting tesseract \n");
	setupTesseract();

	Pix *image = pixRead("../img/arabic2.png");
	api->SetImage(image);
	// Get OCR result
	char *text;
	text = api->GetUTF8Text();
	printf("OCR output:\n%s", text);

	printf("Starting espeak \n");
	output = AUDIO_OUTPUT_RETRIEVAL;
	int I, Run = 1, L;
	espeak_Initialize(output, Buflength, path, Options);
	espeak_SetParameter(espeakVOICETYPE, 1, 0);
	const char *langNativeString = "ar";
	espeak_VOICE voice;

	memset(&voice, 0, sizeof(espeak_VOICE)); // Zero out the voice first
	voice.languages = langNativeString;
	voice.name = "mb-ar1";
	voice.variant = 2;
	voice.gender = 1;

	espeak_SetVoiceByProperties(&voice);
	Size = strlen(text) + 1;

	espeak_SetSynthCallback(synthesisCallback);

	espeak_Synth(text, Size, position, position_type, end_position, flags,
			unique_identifier, user_data);
	espeak_Synchronize();

	cleanupMemory(text, image);

	return 0;
}

