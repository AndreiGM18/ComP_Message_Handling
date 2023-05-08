// SPDX-License-Identifier: EUPL-1.2
/* Copyright Mitran Andrei-Gabriel 2023 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <poll.h>

#include "structs.h"
#include "utils.h"
#include "poll_funcs.h"
#include "subscriber.h"

int setup(struct pollfd *pfds, int *nfds, char *id, char *ip, char *port) {
	// Creates a TCP socket
	int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_sock < 0, "socket");

	// Sets TCP_NODELAY socket option to disable the Nagle algorithm
	int optval = 1;
	setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

	// Sets up server address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(port));
	inet_aton(ip, &server_addr.sin_addr);

	// Adds the standard input and the TCP socket to the polling file descriptors
	add_socket(pfds, nfds, STDIN_FILENO);
	add_socket(pfds, nfds, tcp_sock);

	// Connects to the server
	int conn_ret = connect(tcp_sock, (struct sockaddr *)&server_addr,
							sizeof(server_addr));
	DIE(conn_ret < 0, "connect() failed");

	// Sends the client ID to the server
	int send_ret = send(tcp_sock, id, IDSIZ, 0);
	DIE(send_ret < 0, "send() failed");

	return tcp_sock;
}

void create_packet(sub_packet_t *pack, char *buffer, uint8_t type) {
	// "subscribe" or "unsubscribe"
	char *token = strtok(buffer, " ");
	pack->type = type;

	// <topic>
	token = strtok(NULL, " ");
	strcpy(pack->topic, token);

	// "0" or "1"
	token = strtok(NULL, " ");
	pack->sf = token[0] - '0';
}

bool stdin_cmd(int tcp_sock, char *buffer) {
	// Clears the buffer
	memset(buffer, 0, BUFSIZ);

	// Reads a command from the standard input
	fgets(buffer, BUFSIZ, stdin);

	// Creates a packet based on the command type
	sub_packet_t pack;
	memset(&pack, 0, PACKLEN);

	// Creates a packet and sends it to the server
	if (!strncmp(buffer, "exit", 4)) {
		pack.type = EXIT;
		int ret = send(tcp_sock, &pack, PACKLEN, 0);
		DIE(ret < 0, "send() failed");

		// Returns in order to break the main loop
		return false;
	} else if (!strncmp(buffer, "subscribe", 9)) {
		create_packet(&pack, buffer, SUBSCRIBE);

		int ret = send(tcp_sock, &pack, PACKLEN, 0);
		DIE(ret < 0, "send() failed");

		printf("Subscribed to topic.\n");
	} else if (!strncmp(buffer, "unsubscribe", 11)) {
		create_packet(&pack, buffer, UNSUBSCRIBE);

		int ret = send(tcp_sock, &pack, PACKLEN, 0);
		DIE (ret < 0, "send() failed");

		printf("Unsubscribed to topic.\n");
	} else {
		// If the command is invalid, it is ignored
		printf("Invalid command.\n");
	}

	// Returns in order to continue the main loop
	return true;
}

bool server_cmd(int tcp_sock, char *buffer) {
	// Clears the buffer
	memset(buffer, 0, BUFSIZ);

	// Receives message from server
	int ret = recv(tcp_sock, buffer, sizeof(tcp_msg_t), 0);
	DIE(ret < 0, "receive() failed");

	// If there was no data received, returns in order to break the main loop
	if (!ret)
		return false;

	// Casts buffer to a tcp_msg_t struct and prints its content
	tcp_msg_t *msg_recv = (tcp_msg_t *)buffer;
	printf("%s:%hu - %s - %s - %s\n", msg_recv->ip, ntohs(msg_recv->port),
		msg_recv->topic, msg_recv->type, msg_recv->content);

	// Returns in order to continue the main loop
	return true;
}

int main(int argc, char **argv) {
	// Checks if there are enough arguments
	// (at least 4, including the program name)
	DIE(argc < 4, "Not enough arguments (argv).");

	// Sets stdout to unbuffered mode
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	// Creates an array of pollfd structs and initialize the number of fds to 0
	struct pollfd pfds[MAX_PFDS];
	int nfds = 0;
	
	// Sets up a TCP connection
	int tcp_sock = setup(pfds, &nfds, argv[1], argv[2], argv[3]);

	// Main loop of the program, runs until an 'exit' command from stdin is met
	while (true) {
		// Waits for events on the pollfd array
		int ret = poll(pfds, nfds, -1);

		// Checks if poll failed and exit the program if it did
		DIE(ret < 0, "poll() failed");

		// Multipurpose buffer
		char buffer[BUFSIZ];
	
		// If there is input on standard input, handles the command
		// When receiving "exit", it breaks the loop
		if (pfds[0].revents & POLLIN)
			if (!stdin_cmd(tcp_sock, buffer))
				break;

		// If there is input from the server, handles the message
		if (pfds[1].revents & POLLIN)
			if (!server_cmd(tcp_sock, buffer))
				break;
	}

	// Closes the TCP socket
	close(tcp_sock);

	return 0;
}
