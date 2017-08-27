HEADERS = midiPlayer.h midi.h
OBJECTS = midiPlayer.o midi.c

default: midieval

%.o: %.c $(HEADERS)
	gcc -c $< -o $@ -std=gnu99

midieval: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall -std=gnu99 -lrt -lasound -lpthread -lm libportaudio.a

clean:
	-rm -f $(OBJECTS)
	-rm -f midieval
