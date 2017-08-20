#include <stdio.h>
#include "midi.h"
#include <stdlib.h>

void processMidiFile(char *filename, midi *MIDI) {
	//setup file
	FILE *midiFile;
	midiFile = fopen(filename, "rb");
	long fileLen;
	char *buffer;
	fseek(midiFile, 0, SEEK_END);
	fileLen = ftell(midiFile);
	rewind(midiFile);
	buffer = (char *)malloc((fileLen + 1) * sizeof(char));
	fread(buffer, fileLen, 1, midiFile); //read file into buffer
	fclose(midiFile);

	//parse header

	//check for the header ID
	if (buffer[0]!= 0x4D || buffer[1] != 0x54 || buffer[2] != 0x68
		|| buffer[3] != 0x64) {
		printf("Invalid MIDI header. Exiting.");
		exit(-1);
	}
	//get the format (0, 1 or 2) of the midi file
	MIDI->format = buffer[8];

	if (MIDI->format > 2 || MIDI->format < 0) {
		printf("Invalid MIDI format. Exiting.");
		exit(-1);
	}
	//get the number of tracks in the midi file
	MIDI->numTracks = buffer[10];
	//allocate space for tracks
	MIDI->firstTrack = malloc(sizeof(midiTrack));
	//get tick resolution of midi file
	float smpteFrames = buffer[12] & 0x7F00;
	smpteFrames = (29 == smpteFrames) ? 29.97 : smpteFrames;
	int ticksPerFrame = buffer[12] & 0x00FF;
	if (buffer[12] & 0x8000) {
		//set tick resolution
		MIDI->tickRes = 100000 / (smpteFrames * ticksPerFrame);
		//set time division
		MIDI->timeDiv = FRAMES_PER_SEC;
	} else {
		//set tick resolution
		MIDI->tickRes = 500000 / buffer[12];
		//set time division
		MIDI->timeDiv = TICKS_PER_BEAT;
	}
	//done reading header, on to more important business
	//set index of buffer to end of header so that we can read data
	int bufIndex = MIDI_HDR_LEN;
	int i;
	midiTrack *curTrack = MIDI->firstTrack;
	//setup track objects
	for (i = 0; i < MIDI->numTracks; i++) {
		//check track header
		if (buffer[bufIndex] != 0x4D || buffer[bufIndex + 1] != 0x54
	|| buffer[bufIndex + 2] != 0x72 || buffer[bufIndex + 3] != 0x6B) {
			printf("Invalid track header. Exiting.");
			exit(-1);
		}
		//get the length of the track
		curTrack->length = buffer[bufIndex + 4];
		//check for end of track event
		if (buffer[bufIndex + TRK_HDR_LEN + (curTrack->length - 3)] != 0xFF
	|| buffer[bufIndex + TRK_HDR_LEN + (curTrack->length - 2)] != 0x2F
	|| buffer[bufIndex + TRK_HDR_LEN + (curTrack->length - 1)] != 0x00) {
			printf("No end track event found. Exiting");
			exit(-1);
		}
		//set where to read events from track
		curTrack->startIndex = bufIndex + TRK_HDR_LEN;
		//update current index in buffer
		bufIndex += curTrack->length + TRK_HDR_LEN;
		//increment track
		if (i == MIDI->numTracks) {
			curTrack->nextTrack = NULL; //last track, end linked list
		} else {
			midiTrack *tempTrack = malloc(sizeof(midiTrack)); //create next track
			curTrack->nextTrack = tempTrack; //set next track
			curTrack = curTrack->nextTrack; //go to next track
		}
	}
	//parse each track and populate events



	//remember to free midi tracks and events

}
