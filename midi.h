#include <stdio.h>
#include <stdlib.h>

#define MIDI_HDR_LEN 14 //midi file header length
#define TRK_HDR_LEN 8 //track header length
#define FRAMES_PER_SEC 1
#define TICKS_PER_BEAT 2

//there are obviously more types, this is just the basic implementation
typedef enum {
	NOTE_ON,
	NOTE_OFF
} eventType;

//typedef struct _midiEvent midiEvent;
//typedef struct _midiTrack midiTrack;

typedef struct _midiEvent {
	eventType type;
	float frame;
	int noteNumber;
	struct _midiEvent *nextEvent;
} midiEvent;

typedef struct _midiTrack {
	int length;
	int numEvents;
	int startIndex;
	struct _midiTrack *nextTrack;
	midiEvent *firstEvent;
} midiTrack;

typedef struct _midi {
	int format;
	int numTracks;
	midiTrack *firstTrack;
	int tickRes;
	float timeDiv;
} midi;

void processMidiFile(char *filename, midi *MIDI);
