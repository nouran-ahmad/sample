#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/sample.h>

espeak_POSITION_TYPE positionType = POS_WORD;
espeak_AUDIO_OUTPUT output;
tesseract::TessBaseAPI *api;

int Buflength = 10000, Options = 0;
void* userData;
t_espeak_callback *SynthCallback;
char *text;
unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO,
		*uniqueIdentifier=NULL;

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
	if (api->Init("../tessdata",
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

int main(int argc, char* argv[]) {

	setupPulseAudio();
	setupTesseract();
	setupEspeak();

	Pix *image = pixRead("../img/arabic2.png");
	api->SetImage(image);
	char *text;
	text = api->GetUTF8Text();
	printf("OCR output:\n%s", text);

	unsigned int size = strlen(text) + 1;
	espeak_Synth(text, size, position, positionType, endPosition, flags,
			uniqueIdentifier, userData);
	espeak_Synchronize();

	espeak_Terminate();
	cleanupMemory(text, image);
	
	return 0;
}
