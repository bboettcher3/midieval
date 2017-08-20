HEADERS = midiPlayer.h
OBJECTS = midiPlayer.o

default: midieval

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

midieval: $(OBJECTS)
	gcc $(OBJECTS) -o $@ -lrt -lasound -lpthread -lm libportaudio.a

clean:
	-rm -f $(OBJECTS)
	-rm -f midieval
