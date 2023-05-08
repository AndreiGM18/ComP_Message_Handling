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
#include "list.h"
#include "utils.h"
#include "poll_funcs.h"
#include "server.h"

sockets_t *setup_server(struct pollfd *pfds, int *nfds, char *port) {
	 // Creates a new TCP socket
	int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_sock < 0, "tcp socket() failed");

	// Sets TCP_NODELAY socket option to disable the Nagle algorithm
	int optval = 1;
	setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

	// Creates a new UDP socket
	int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udp_sock < 0, "udp socket() failed");

	// Sets up the TCP address structure
	struct sockaddr_in tcp_addr;
	memset((char *)&tcp_addr, 0, sizeof(struct sockaddr_in));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(atoi(port));
	tcp_addr.sin_addr.s_addr = INADDR_ANY;

	// Binds the TCP socket to the TCP address
	int tcp_bnd = bind(tcp_sock, (struct sockaddr *)&tcp_addr,
						sizeof(struct sockaddr));
	DIE(tcp_bnd < 0, "tcp bind() failed");

	// Sets up the UDP address structure
	struct sockaddr_in udp_addr;
	memset((char *)&udp_addr, 0, sizeof(struct sockaddr_in));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(atoi(port));
	udp_addr.sin_addr.s_addr = INADDR_ANY;

	// Binds the UDP socket to the UDP address
	int udp_bnd = bind(udp_sock, (struct sockaddr *)&udp_addr,
						sizeof(struct sockaddr));
	DIE(udp_bnd < 0, "udp bind() failed");

	// Listens on the TCP socket
	int tcp_lsn = listen(tcp_sock, MAX_CLIENTS);
	DIE(tcp_lsn < 0, "tcp listen() failed");

	// Adds the STDIN_FILENO, TCP socket, and UDP socket file descriptors
	// to the file descriptor array and update the number of file descriptors
	add_socket(pfds, nfds, STDIN_FILENO);
	add_socket(pfds, nfds, tcp_sock);
	add_socket(pfds, nfds, udp_sock);

	// Creates and initializes the sockets_t struct that contains the TCP
	// and UDP sockets
	sockets_t *socks = malloc(sizeof(sockets_t));
	DIE(!socks, "sockets malloc() failed");
	socks->tcp_sock = tcp_sock;
	socks->tcp_addr = tcp_addr;
	socks->udp_sock = udp_sock;
	socks->udp_addr = udp_addr;
	socks->len = sizeof(struct sockaddr);

	return socks;
}

bool stdin_cmd(char *buffer) {
	// Clears the buffer
	memset(buffer, 0, BUFSIZ);

	// Reads user input from standard input
	fgets(buffer, BUFSIZ, stdin);

	// Checks if user input is "exit" to signal program should exit
	if (!strncmp(buffer, "exit", 4))
		return false;

	// If input is invalid, prints error and exits
	DIE(strncmp(buffer, "exit", 4), "Invalid input from STDIN.");

	return true;
}

void tcp(struct pollfd *pfds, int *nfds, list_t *clients, sockets_t *socks,
			char *buffer) {
	// Clears the buffer
	memset(buffer, 0, BUFSIZ);

	// Accepts a new connection on the TCP socket
	struct sockaddr_in new_tcp;
	int socket = accept(socks->tcp_sock, (struct sockaddr *)&new_tcp,
						&socks->len);
	DIE(socket < 0, "new socket accept() failed");

	// Receives the ID of the client
	int ret = recv(socket, buffer, IDSIZ, 0);
	DIE(ret < 0, "recv() failed");

	// Checks if the client already exists in the clients list
	client_t *found = NULL;
	node_t *client_node = clients->head;
	while (client_node) {
		client_t *client = (client_t *)client_node->data;
		if (!strcmp(client->id, buffer)) {
			found = client;
			break;
		}
		client_node = client_node->next;
	}

	// If the client does not exist, adds it to the clients list and
	// sets up its fields
	if (!found) {
		add_socket(pfds, nfds, socket);

		client_t *new = calloc(1, sizeof(client_t));
		DIE(!new, "new client calloc() failed");

		strcpy(new->id, buffer);
		new->socket = socket;
		new->online = true;
		new->unsent = list_create(sizeof(tcp_msg_t));
		new->topics = list_create(sizeof(topic_t));

		list_add_head(clients, new);

		// Prints a message indicating a new client has connected
		printf("New client %s connected from %s:%d.\n", new->id,
			inet_ntoa(new_tcp.sin_addr), ntohs(new_tcp.sin_port));

		free(new);
	}
	// If the client exists and is offline, reconnects it and
	// sends unsent messages
	else if (found && !found->online) {
		// Is back online
		add_socket(pfds, nfds, socket);
		found->socket = socket;
		found->online = true;

		// Prints a message indicating the client has reconnected
		printf("New client %s connected from %s:%d.\n", found->id,
			inet_ntoa(new_tcp.sin_addr), ntohs(new_tcp.sin_port));

		// Sends unsent messages, clearing the unsent messages list
		node_t *unsent_node = found->unsent->head, *next;
		while (unsent_node) {
			tcp_msg_t unsent = *(tcp_msg_t *)unsent_node->data;
			int ret = send(found->socket, &unsent, found->unsent->data_size, 0);
			DIE(ret < 0, "unsent send() failed");

			next = unsent_node->next;

			free(unsent_node->data);
			free(unsent_node);

			unsent_node = next;
		}
	}
	// If the client exists and is already online, closes the connection
	else {
		close(socket);
		printf("Client %s already connected.\n", found->id);
	}
}

void udp(list_t *clients, sockets_t *socks, char *buffer) {
	// Clears the buffer
	memset(buffer, 0, BUFSIZ);

	// Receives a UDP message from the socket and store it in the buffer
	int ret = recvfrom(socks->udp_sock, buffer, sizeof(udp_msg_t), 0,
						(struct sockaddr *)&socks->udp_addr, &socks->len);
	DIE(ret < 0, "udp recvfrom() failed");

	// Declares variables to store the received UDP message and the
	// TCP message to be sent
	tcp_msg_t tcp_send;
	memset(&tcp_send, 0, sizeof(tcp_msg_t));
	udp_msg_t *udp_recv;
	udp_recv = (udp_msg_t *)buffer;

	// Extracts the topic and ensures that it is null-terminated
	strcpy(tcp_send.topic, udp_recv->topic);
	tcp_send.topic[50] = '\0';

	// Depending on the message type, extracts the message's content
	// and stores it in the TCP message structure
	if (udp_recv->type == INT) {
		// Converts to host order
		uint32_t int_num = ntohl(*(uint32_t *)(udp_recv->content + 1));

		// Changes the sign, if necessary
		if (udp_recv->content[0] == 1)
			int_num = int_num * (-1);

		sprintf(tcp_send.content, "%d", int_num);

		strcpy(tcp_send.type, "INT");
	} else if (udp_recv->type == SHORT_REAL) {
		// Converts to host order
		double short_real = ntohs(*(uint16_t *)(udp_recv->content));

		// Converts the number to a short real
		// Also shifts the decimal point two places
		short_real = short_real / 100;

		strcpy(tcp_send.type, "SHORT_REAL");

		sprintf(tcp_send.content, "%.2f", short_real);
	} else if (udp_recv->type == FLOAT) {
		// Converts to host order
		double float_num = ntohl(*(uint32_t *)(udp_recv->content + 1));

		// Gets the decimal point's position
		int floating_point = 1;
		for (int i = 0; i < udp_recv->content[5]; ++i)
			floating_point *= 10;

		// Converts the number to a float, also shifts the decimal point
		float_num = float_num / floating_point;

		strcpy(tcp_send.type, "FLOAT");

		// Changes the sign, if necessary
		if (udp_recv->content[0] == 1)
			float_num = float_num * (-1);

		sprintf(tcp_send.content, "%lf", float_num);
	} else if (udp_recv->type == STRING) {
		strcpy(tcp_send.type, "STRING");
		strcpy(tcp_send.content, udp_recv->content);
	}

	// Loops through all clients in the list of clients and sends the TCP
	// message to clients that have subscribed to the message's topic
	node_t *client_node = clients->head;
	while (client_node) {
		client_t *client = (client_t *)client_node->data;

		// Loops through all topics subscribed to by the client and checks if
		// the topic of the received message matches any of them
		node_t *topic_node = client->topics->head;
		while (topic_node) {
			topic_t *topic = (topic_t *)topic_node->data;

			if (!strcmp(topic->name, tcp_send.topic)) {
				// If the client is online, sends the message
				if (client->online) {
					int ret = send(client->socket, &tcp_send,
						sizeof(tcp_msg_t), 0);
					DIE(ret < 0, "send() failed");
				}
				// If not, it stores the message for when the client
				// comes back online
				else {
					if (topic->sf == 1) {
						tcp_msg_t *new_tcp_struct = malloc(sizeof(tcp_msg_t));
						memcpy(new_tcp_struct, &tcp_send, sizeof(tcp_msg_t));
						list_add_head(client->unsent, new_tcp_struct);
						free(new_tcp_struct);
					}
				}
				break;
			}
			topic_node = topic_node->next;
		}
		client_node = client_node->next;
	}
}

void subscriber_protocol(struct pollfd *pfds, int *nfds, list_t *clients, int i,
							char *buffer) {
	// Clears the buffer
	memset(buffer, 0, BUFSIZ);

	// Gets the file descriptor associated with the client
	int fd = pfds[i].fd;

	// Receives data from the client
	int ret = recv(fd, buffer, PACKLEN, 0);
	DIE(ret < 0, "subscriber recv() failed");

	// Checks if data was received
	if (ret) {
		// Casts the received data to a subscription packet
		sub_packet_t *input = (sub_packet_t *)buffer;

		// Searches for the client in the list of clients
		node_t *client_node = clients->head;
		client_t *found = NULL;
		while (client_node) {
			client_t *client = (client_t *)client_node->data;
			if (fd == client->socket) {
				found = client;
				break;
			}
			client_node = client_node->next;
		}

		// Handles the subscription request
		if (input->type == SUBSCRIBE) {
			topic_t *topic_found = NULL;

			// Searches for the topic in the client's list of subscribed topics
			node_t *topic_node = found->topics->head;
			while (topic_node) {
				topic_t *topic = (topic_t *)topic_node->data;
				if (!strcmp(topic->name, input->topic)) {
					topic_found = topic;
					break;
				}
				topic_node = topic_node->next;
			}

			// Adds the topic to the client's list of subscribed topics,
			// if not found
			if (!topic_found) {
				topic_t *new_topic = malloc(sizeof(topic_t));
				strcpy(new_topic->name, input->topic);
				new_topic->sf = input->sf;
				list_add_head(found->topics, new_topic);
				free(new_topic);
			}
		}
		// Handles the unsubscription request 
		else if (input->type == UNSUBSCRIBE) {
			node_t *topic_node = found->topics->head;
			topic_t *topic = (topic_t *)topic_node->data;

			// Removes the topic from the client's list of subscribed topics
			// Has two cases: if the head must or any other node must be removed
			if (!strcmp(topic->name, input->topic)) {
				found->topics->head = found->topics->head->next;
				free(topic_node->data);
				free(topic_node);
			} else {
				node_t *pred = topic_node;
				topic_node = topic_node->next;
				while (topic_node) {
					topic_t *topic = (topic_t *)topic_node->data;
					if (!strcmp(topic->name, input->topic)) {
						pred->next = topic_node->next;
						free(topic_node->data);
						free(topic_node);
						break;
					}
					pred = topic_node;
					topic_node = topic_node->next;
				}
			}
		}
		// Handles the client's exit request
		else if (input->type == EXIT) {
			// Finds the client and sets them as offline
			node_t *client_node = clients->head;
			while (client_node) {
				client_t *client = (client_t *)client_node->data;
				if (fd == client->socket) {
					printf("Client %s disconnected.\n", client->id);

					// Is now offline
					client->online = false;
					client->socket = EMPTY;
					close(fd);
					remove_socket(pfds, nfds, i);

					break;
				}
			client_node = client_node->next;
			}
		}
	}
}

int main(int argc, char **argv) {
	// Checks if there are enough arguments
	// (at least 2, including the program name)
	DIE(argc < 2, "Not enough arguments (argv).");

	// Sets stdout to unbuffered mode
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	// Creates an array of pollfd structs and initialize the number of fds to 0
	struct pollfd pfds[MAX_PFDS];
	int nfds = 0;

	// Sets up the server sockets and add them to the pollfd array
	sockets_t *socks = setup_server(pfds, &nfds, argv[1]);

	// Creats a linked list of clients
	list_t *clients = list_create(sizeof(client_t));

	// Main loop of the program, runs until an 'exit' command from stdin is met
	while (true) {
		// Waits for events on the pollfd array
		int ret = poll(pfds, nfds, -1);

		// Checks if poll failed and exit the program if it did
		DIE(ret < 0, "poll() failed");

		// Multipurpose buffer
		char buffer[BUFSIZ];

		// Handles input from stdin
		// When receiving "exit", it breaks the loop
		if (pfds[0].revents & POLLIN)
			if (!stdin_cmd(buffer))
				break;

		// Handles packets from subscriber TCP clients
		for (int i = 3; i < nfds; ++i)
			if (pfds[i].revents & POLLIN)
				subscriber_protocol(pfds, &nfds, clients, i, buffer);

		// Handles new TCP connections (TCP clients)
		if (pfds[1].revents & POLLIN)
			tcp(pfds, &nfds, clients, socks, buffer);

		// Handles UDP connections and sends messages to the TCP clients that are
		// interested in what the UDP client posted about
		if (pfds[2].revents & POLLIN)
			udp(clients, socks, buffer);
	}

	// Closes all file descriptors in the pollfd array
	for (int i = 0; i < nfds; ++i) {
		close(pfds[i].fd);
	}

	// Frees all resources used by each client
	node_t *client_node = clients->head;
	while (client_node) {
		client_t *client = (client_t *)client_node->data;
		client_node = client_node->next;

		list_free(&client->unsent);
		list_free(&client->topics);
	}

	// Frees the linked list of clients
	list_free(&clients);

	// Frees the server sockets
	free(socks);

	return 0;
}
