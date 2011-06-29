CFLAGS=-Wall -O2 -ggdb
LDFLAGS=`sdl-config --libs`

OBJECTS=marfbed.o init.o gfx.o marf.o

all:marfbed

marfbed:$(OBJECTS)

clean:
	rm -f *.o marfbed
