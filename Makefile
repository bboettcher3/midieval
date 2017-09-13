HEADERS = midiPlayer.h midi.h
OBJECTS = midiPlayer.o midi.c

default: midieval

%.o: %.c $(HEADERS)
	gcc -c $< -o $@ -std=gnu99 -g

midieval: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall -g -std=gnu99 -lrt -lasound -lpthread -lm libportaudio.a

clean:
	-rm -f $(OBJECTS)
	-rm -f midieval
