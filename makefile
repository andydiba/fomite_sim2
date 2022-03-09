CC = gcc
CFLAGS = -Wall
TARGET = sim
OBJFILES = sim.o queue.o probability.o hash.o utilities.o pathogen.o zone.o person.o 

$(TARGET): $(OBJFILES) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) -lm

sim.o: sim.c queue.h probability.h hash.h utilities.h person.h pathogen.h zone.h
	$(CC) $(CFLAGS) -c sim.c 

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

probability.o: probability.c probability.h
	$(CC) $(CFLAGS) -c probability.c 

hash.o: hash.c hash.h utilities.h
	$(CC) $(CFLAGS) -c hash.c

utilities.o: utilities.c utilities.h
	$(CC) $(CFLAGS) -c utilities.c

pathogen.o: pathogen.c pathogen.h probability.h utilities.h
	$(CC) $(CFLAGS) -c pathogen.c

person.o: person.c person.h probability.h utilities.h
	$(CC) $(CFLAGS) -c person.c
	
zone.o: zone.c zone.h utilities.h
	$(CC) $(CFLAGS) -c zone.c

clean:
	rm $(TARGET) $(OBJFILES)

