CC=gcc
CFLAGS=-Wall -O3

all: tg_server tg_client

tg_server: tg_server.c
	${CC} ${CFLAGS} $^ -o $@ 

tg_client: tg_client.c
	${CC} ${CFLAGS} $^ -o $@ 

clean:
	rm -rf tg_server tg_client *~
