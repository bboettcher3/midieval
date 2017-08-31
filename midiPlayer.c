#include "midiPlayer.h"

float sawPhase;
bool isDone;
PaError err;


int main(int argc, char **argv) {

	//input validation
	if (argc != 2) {
		printf("Please include only the filename as the parameter\n");
		return 0;
	}

	eventNodeMain = malloc(sizeof(midi_event_node_t));

	eventNodeMain = processMidi(argv[1]);

	PaStream *stream;

	Pa_Sleep(1*1000);

	sawPhase = 0;

	printf("-------------- Initializing Audio --------------\n");

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

	printf("-------------- Done Initializing Audio --------------\n");

	//wait for a bit
	Pa_Sleep(3*1000);
	isDone = false;

	struct sigaction sa;
	memset (&sa, 0, sizeof(sa));
	sa.sa_handler = alrmHandler;
	sa.sa_flags = 0;
	if (sigaction(SIGALRM, &sa, NULL) != 0) {
		printf("Error setting up alarm\n");
	}
	struct itimerval timerMain;
	//printf("playing note: %d\n", eventNodeMain->event.data[0]);
	//printf("next note: %d\n", curEventNode->next->event.data[0]);
	eventNodeMain = eventNodeMain->next;
	timerMain.it_value.tv_sec = 1;
	timerMain.it_value.tv_usec = 0;
	timerMain.it_interval.tv_sec = 3;
	timerMain.it_interval.tv_usec = 0;
	if (setitimer(ITIMER_REAL, &timerMain, NULL) < 0) {
		printf("Error sending alarm\n");
		exit(-1);
	}
    	while (!isDone) {

    	}

	//close stream
	err = Pa_StopStream(stream);
	if (err != paNoError) { print_error(); }
	err = Pa_CloseStream(stream);
	if (err != paNoError) { print_error(); }

	//midi_close(
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

midi_event_node_t * processMidi(char *filename) {

	//make midi object
	midi_t *midi;
	int status;
	midi_track_t *track;

	midi_event_node_t *firstEventNode;

	//open midi file
	status = midi_open(filename, &midi);

	//check status
	if (status) {
		printf("Failed to open midi file\n");
		exit(-1);
	}

	//printf("header dd: %d\n", midi->hdr.dd);
	secs_per_tick = SECS_PER_BEAT / 256;
    //loop through tracks and gather events
    for (int i = 0; i < midi->hdr.tracks; i++) {
        int curTime = 0;
        //retrieve midi track
        track = midi_get_track(midi, i);
        //check for track existence
        if (track == NULL) {
            printf("Failed to find track %d\n", i);
            exit(-1);
        }

        //parse events
        midi_iter_track(track);
        midi_event_t *curEvent;
        midi_event_node_t *curEventNode;
	//firstEventNode = track->cur;
	curEventNode = track->cur;
        while (midi_track_has_next(track)) {
            //assign first event pointer for use by main program
            //if (firstEventNode == NULL) {
            //    firstEventNode = track->cur;
            //    curEventNode = firstEventNode;
            //}

            curEvent = midi_track_next(track);

            if (curEvent->type == MIDI_TYPE_EVENT) {
                if (curEvent->cmd == MIDI_EVENT_NOTE_ON ||
                        curEvent->cmd == MIDI_EVENT_NOTE_OFF) {
                    //note on or off event detected
                    curEventNode->next = track->cur;
                    curEventNode = track->cur;
                    //printf("Track data 0: %d, 1: %d\n", curEvent->data[0], curEvent->data[1]);

                }

            }

        }

        //free track allocated in midi_get_track
        //midi_free_track(track);
    }
	return track->head;

}

static int getNextBlock( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData ) {
    float *out = (float*) outputBuffer;
    (void) inputBuffer; //even if not using input have this to prevent warning
    int i;

	//voice *curVoice = firstVoice;
    //loop to write samples
    for (i = 0; i < framesPerBuffer; i++) {
	voice *curVoice = firstVoice;
	*out = 0;
	*(out + 1) = 0;
	while (curVoice != NULL) {
		updatePhase(curVoice->oscillator);
		//printf("phase: %f\n", curVoice->oscillator->phase);
		*out += (float)sin(2* M_PI * curVoice->oscillator->phase);
		*(out + 1) += (float)sin(2* M_PI * curVoice->oscillator->phase);
		curVoice = curVoice->next;
	}

	//printf("out 1: %f, out 2: %f\n", *out, *(out+1));
	out += 2;
	//*out++ = sawPhase;
        //*out++ = sawPhase;

        //sawPhase += .01f;

        //if (sawPhase >= 1.0f) {
        //    sawPhase -= 2.0f;
        //}

    }
    return 0;
}

void updatePhase(osc *oscillator) {
    double cyclesPerSample = oscillator->frequency / SAMPLE_RATE;
    oscillator->phaseDelta = cyclesPerSample * 2 * M_PI;
    oscillator->phase = oscillator->phase + oscillator->phaseDelta;
}

void alrmHandler(int blah) {

	//get note number of current event
	if (eventNodeMain->event.cmd == MIDI_EVENT_NOTE_ON ||
		eventNodeMain->event.data[1] != 0) {
		printf("Playing note: %d\n", eventNodeMain->event.data[0]);
		//add voice
		voice *newVoice = malloc(sizeof(voice));
		newVoice->midiNum = eventNodeMain->event.data[0];
		newVoice->oscillator = malloc(sizeof(osc));
		newVoice->oscillator->frequency = mtof(newVoice->midiNum);
		newVoice->oscillator->phase = 0;
		newVoice->next = NULL;

		if (firstVoice == NULL) {
			firstVoice = newVoice;
			printf("setting first voice\n");
		} else {
			voice *curVoice = firstVoice;
			while (curVoice->next != NULL) {
				curVoice = curVoice->next;
			}
			curVoice->next = newVoice;
		}

	} else {
		printf("removing note\n");
		//remove voice
		bool wasFound = false;
		voice *curVoice = firstVoice;
		while (curVoice->next != NULL) {
			if (curVoice->next->midiNum == eventNodeMain->event.data[0]) {
				//free(curVoice->next);
				//TODO: free this properly
				curVoice->next = curVoice->next->next;
				wasFound = true;
			}
		}
		if (!wasFound) {
			firstVoice = NULL;
			free(firstVoice);
		}
	}
	if (eventNodeMain->next == NULL) {
		isDone = true;
		return;
	} else {
		eventNodeMain = eventNodeMain->next;
	}
	//get delta time into seconds for timer
	float dtSecsWhole = eventNodeMain->next->event.td * secs_per_tick;
	int dtSecs = (int)dtSecsWhole;
	float dtUSecs = (dtSecsWhole - dtSecs) * 1000000.0f;
	struct itimerval timer;
	if (dtSecsWhole == 0.0f) {
                //set a very small amount of time between consecutive hits
                dtSecs = 0;
                dtUSecs = 10.0f;
        }
	timer.it_value.tv_sec = dtSecs;
	timer.it_value.tv_usec = dtUSecs;

	if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
		printf("Error setting alarm in handler\n");
		exit(-1);
	}
	//printf("timer set\n");
}

void print_error(void) {
    Pa_Terminate();
    printf("An error occured with portaudio\n");
    printf("Error : %s\n", Pa_GetErrorText(err));
}

float mtof(int midiNote) {
	return mtofarray[midiNote];
}
