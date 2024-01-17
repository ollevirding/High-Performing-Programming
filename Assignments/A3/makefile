CFLAGS=-W -O3 -ffast-math -g -march=native
INCLUDES=-I/opt/X11/include
LDFLAGS=-L/opt/X11/lib -lX11 -lm -g -ffunction-sections
FILE=galsim.c


galsim: galsim.o graphics.o
	gcc -o galsim galsim.o graphics.o $(LDFLAGS)

galSim.o: $(FILE) graphics.h
	gcc $(CFLAGS) $(INCLUDES) -c $(FILE)

graphics.o: graphics.c graphics.h
	gcc $(CFLAGS) $(INCLUDES) -c graphics.c

clean:
	rm -f ./galsim *.o
