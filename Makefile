CC=gcc
CFLAGS=-Wall -g
LINK=-lpthread

all: libscheduler.so

build: libscheduler.so

libscheduler.so: scheduler.o priq.o
	$(CC) $^ -shared -o $@ $(CFLAGS)

scheduler.o: scheduler.c
	$(CC) $^ -fPIC -c -o $@ $(CFLAGS) $(LINK)

priq.o: priq.c
	$(CC) $^ -fPIC -c -o $@ $(CFLAGS) $(LINK)

clean:
	rm -rf scheduler.o priq.o libscheduler.so
