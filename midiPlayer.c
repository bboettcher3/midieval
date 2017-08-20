#include "midiPlayer.h"

float sawPhase;
PaError err;


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

int main(int argc, char **argv) {

	//input validation
	if (argc != 2) {
		printf("Please include only the filename as the parameter\n");
		return 0;
	}

	//create midi pointer to fill in values
	midi *MIDI = malloc(sizeof(midi));

	processMidiFile(argv[1], MIDI);

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
