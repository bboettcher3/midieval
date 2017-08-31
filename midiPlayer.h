#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "midi.h"

#define SAMPLE_RATE 44100
#define SECS_PER_BEAT 0.5

typedef struct _osc {
	float phase;
	float phaseDelta;
	float frequency;
} osc;

osc voices[10];
int midiFormat;
int numMidiTracks;
int midiTimeDivision;
float secs_per_tick;
static midi_event_node_t *eventNodeMain;


//the callback used for getting audio data to read/write
static int getNextBlock( const void *input,
                                      void *output,
                                      unsigned long frameCount,
                                      const PaStreamCallbackTimeInfo* timeInfo,
                                      PaStreamCallbackFlags statusFlags,
                                      void *userData ) ;
void print_error(void);

void alrmHandler(int blah);

void updatePhase(osc *oscillator);
midi_event_node_t * processMidi(char *filename);
