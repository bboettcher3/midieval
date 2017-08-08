#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE 44100

typedef struct _osc {
	float phase;
	float phaseDelta;
	float frequency;
} osc;

osc voices[10];

//the callback used for getting audio data to read/write
static int getNextBlock( const void *input,
                                      void *output,
                                      unsigned long frameCount,
                                      const PaStreamCallbackTimeInfo* timeInfo,
                                      PaStreamCallbackFlags statusFlags,
                                      void *userData ) ;
void print_error(void);

void updatePhase(osc *oscillator);

void parseMidi(FILE *file);
