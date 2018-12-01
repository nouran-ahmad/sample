#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/sample.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <unistd.h>
 
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
	if (pulseAudio)
		pa_simple_free(pulseAudio);
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



void textToSpeech(string text){
	
	espeak_POSITION_TYPE positionType = POS_WORD;
	unsigned int position = 0, endPosition = 0, flags = espeakCHARS_AUTO;
	
	espeak_Synth(text.c_str(), text.size()+1, position, 
	positionType, endPosition, flags,	NULL, userData);
	espeak_Synchronize();
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
	
int main(int argc, char* argv[]) {

	setupPulseAudio();
	setupEspeak();

	string text= readFile(argv[1]);
	textToSpeech(text);
	
	freeApi();
	return 0;
}



