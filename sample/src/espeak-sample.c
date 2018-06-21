#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include "alsa.h"

espeak_POSITION_TYPE position_type;
espeak_AUDIO_OUTPUT output;

char *espeakDataPath=NULL;
int Buflength = 500, Options=0;
void* userData;
t_espeak_callback *SynthCallback;
espeak_PARAMETER Parm;

char Voice[] = {"English"};

char *text = {"hello world this is an english test"};
unsigned int size,position=0, end_position=0, flags=espeakCHARS_AUTO, *uniqueIdentifier;

int synthesisCallback(short* waveData, int sampleCount, espeak_EVENT* event) 
{
	printf("Synthesis callback received: sampleCount: %d.\r\n", sampleCount );
        
        if(waveData) {
		playback(waveData, sampleCount);

	} 
}

int main(int argc, char* argv[] ) 
{
    printf("Testing \n");

    output = AUDIO_OUTPUT_RETRIEVAL;
    int I, Run = 1, L;    
    espeak_Initialize(output, Buflength, espeakDataPath, Options ); 
    espeak_SetVoiceByName(Voice);
    const char *langNativeString = "en"; //Default to US English
    espeak_VOICE voice;

    memset(&voice, 0, sizeof(espeak_VOICE)); // Zero out the voice first
    voice.languages = langNativeString;
    voice.name = "US";
    voice.variant = 2;
    voice.gender = 1;
    
    espeak_SetVoiceByProperties(&voice);
    size = strlen(text)+1;    

    espeak_SetSynthCallback(synthesisCallback);
    
    espeak_Synth( text, size, position, position_type, end_position, flags,
      uniqueIdentifier, userData );
    
    espeak_Synchronize();

    int i;
    for(i=0;i<1000000;i++); 

    return 0;
}

