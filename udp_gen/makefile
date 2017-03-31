CC=gcc
CFLAGS=-Wall -O3

all: ug_talker ug_listener

ug_listener: ug_listener.c
	${CC} ${CFLAGS} $^ -o $@

ug_talker: ug_talker.c
	${CC} ${CFLAGS} $^ -o $@

clean:
	rm -rf ug_talker ug_listener *~
