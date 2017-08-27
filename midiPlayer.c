#include "midiPlayer.h"

float sawPhase;
PaError err;

int main(int argc, char **argv) {

	//input validation
	if (argc != 2) {
		printf("Please include only the filename as the parameter\n");
		return 0;
	}

    //create first event pointer
    midi_event_node_t *curEvent;

	processMidi(argv[1], curEvent);

	PaStream *stream;

	Pa_Sleep(1*1000);

	sawPhase = 0;
	//initialize portAudio
	err = Pa_Initialize();
	if (err != paNoError) { print_error(); }

	//start default stream
	err = Pa_OpenDefaultStream( &stream,
					0, //input channels
					2, //output channels
					paFloat32, //32 bit float output
					SAMPLE_RATE,
					256, //samples per block
					getNextBlock, //callback function
					&sawPhase ); //pointer passed to callback
	if (err != paNoError) { print_error(); }

	err = Pa_StartStream(stream); //begin streaming
	if (err != paNoError) { print_error(); }

	//wait for a bit
	Pa_Sleep(3*1000);

	//close stream
	err = Pa_StopStream(stream);
	if (err != paNoError) { print_error(); }
	err = Pa_CloseStream(stream);
	if (err != paNoError) { print_error(); }


	//check for input to end program
	//char input[1];
	//fgets(input,sizeof(input),stdin);
	//while (input[0] != '\n') {
	//	fgets(input,sizeof(input),stdin);
	//}
	printf("Ending program\n");

	Pa_Terminate();

	return 0;
}

void processMidi(char *filename, midi_event_node_t *firstEventNode) {

	//make midi object
	midi_t *midi;
	int status;

	//open midi file
	status = midi_open(filename, &midi);

	//check status
	if (status) {
		printf("Failed to open midi file\n");
		exit(-1);
	}
    firstEventNode = NULL;
    //loop through tracks and gather events
    for (int i = 0; i < midi->hdr.tracks; i++) {
        int curTime = 0;
        //retrieve midi track
        midi_track_t *track = midi_get_track(midi, i);
        //check for track existence
        if (track == NULL) {
            printf("Failed to find track %d\n", i);
            exit(-1);
        }

        //parse events
        midi_iter_track(track);
        midi_event_t *curEvent;
        midi_event_node_t *curEventNode;
        while (midi_track_has_next(track)) {
            //assign first event pointer for use by main program
            if (firstEventNode == NULL) {
                firstEventNode = track->cur;
                curEventNode = firstEventNode;
            }

            curEvent = midi_track_next(track);

            if (curEvent->type == MIDI_TYPE_EVENT) {
                if (curEvent->cmd == MIDI_EVENT_NOTE_ON ||
                        curEvent->cmd == MIDI_EVENT_NOTE_OFF) {
                    //note on or off event detected
                    curEventNode->next = track->cur;
                    curEventNode = track->cur;

                    printf("Track data 0: %d, 1: %d\n", curEvent->data[0], curEvent->data[1]);

                }

            }

        }

        //free track allocated in midi_get_track
        midi_free_track(track);
    }


}

static int getNextBlock( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData ) {
    float *out = (float*) outputBuffer;
    (void) inputBuffer; //even if not using input have this to prevent warning
    int i;


    //loop to write samples
    for (i = 0; i < framesPerBuffer; i++) {

        *out++ = sawPhase;
        *out++ = sawPhase;

        sawPhase += .01f;

        if (sawPhase >= 1.0f) {
            sawPhase -= 2.0f;
        }

    }
    return 0;
}

void updatePhase(osc *oscillator) {
    double cyclesPerSample = oscillator->frequency / SAMPLE_RATE;
    oscillator->phaseDelta = cyclesPerSample * 2 * M_PI;
    oscillator->phase = oscillator->phase + oscillator->phaseDelta;
}

void print_error(void) {
    Pa_Terminate();
    printf("An error occured with portaudio\n");
    printf("Error : %s\n", Pa_GetErrorText(err));
}