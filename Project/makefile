CC = gcc
CFLAGS = -O3 -fopenmp -ffast-math -march=native
LDLIBS = -lm
RM = /bin/rm -f
OBJS = main.o
PROGRAM = solver

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LDLIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	$(RM) $(PROGRAM) $(OBJS)