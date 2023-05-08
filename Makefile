CFLAGS = -Wall -Wextra

PORT_SERVER = 12345

IP_SERVER = 127.0.0.1

ID = C1

all: server subscriber

server: server.c list.c poll_funcs.c
	gcc $(CFLAGS) -o server server.c list.c poll_funcs.c

subscriber: subscriber.c poll_funcs.c
	gcc $(CFLAGS) -o subscriber subscriber.c poll_funcs.c

.PHONY: clean run_server run_subscriber

run_server:
	./server ${PORT_SERVER}

run_subscriber:
	./subscriber $(ID) ${IP_SERVER} ${PORT_SERVER}

clean:
	rm -f server subscriber
