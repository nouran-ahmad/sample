#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include "alsa.h"

espeak_POSITION_TYPE position_type;
espeak_AUDIO_OUTPUT output;
tesseract::TessBaseAPI *api;

char *path=NULL;
int Buflength = 500, Options=0;
void* user_data;
t_espeak_callback *SynthCallback;
espeak_PARAMETER Parm;

char voiceName[] = {"mb-ar1"};
char *text;
unsigned int Size,position=0, end_position=0, flags=espeakCHARS_AUTO, *unique_identifier;

int synthesisCallback(short* waveData, int sampleCount, espeak_EVENT* event) 
{
	printf("Synthesis callback received: sampleCount: %d.\r\n", sampleCount );
        if(waveData) {
		playback(waveData, sampleCount);
	} 
}

void cleanupMemory(char *outText, Pix *image){
	// Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);
}

void setupTesseract(){
	api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with Arabic, without specifying tessdata path
	printf("Start tesseract Initialize:\n");
    if (api->Init(NULL, "ara")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }
	printf("End tesseract Initialize:\n");
}

int main(int argc, char* argv[] ) 
{
	printf("Starting tesseract \n");
	setupTesseract();
	    
    Pix *image = pixRead("./sample/arabic2.png");
    api->SetImage(image);
    // Get OCR result
    char *text;
    text = api->GetUTF8Text();
    printf("OCR output:\n%s", text);
	
    printf("Starting espeak \n");
    output = AUDIO_OUTPUT_RETRIEVAL;
    int I, Run = 1, L;    
    espeak_Initialize(output, Buflength, path, Options ); 
    espeak_SetVoiceByName(voiceName);
    const char *langNativeString = "ar";
    espeak_VOICE voice;

    memset(&voice, 0, sizeof(espeak_VOICE)); // Zero out the voice first
    voice.languages = langNativeString;
    voice.name = "mb-ar1";
    voice.variant = 2;
    voice.gender = 1;
    
    espeak_SetVoiceByProperties(&voice);
    Size = strlen(text)+1;    

    espeak_SetSynthCallback(synthesisCallback);
    
    espeak_Synth( text, Size, position, position_type, end_position, flags,
      unique_identifier, user_data );
    
    espeak_Synchronize();

    int i;
    for(i=0;i<1000000;i++); 

	cleanupMemory(text, image);
    
    return 0;
}

