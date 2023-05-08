/* SPDX-License-Identifier: EUPL-1.2 */
/* Copyright Mitran Andrei-Gabriel 2023 */

#ifndef _STRUCTS_H
#define _STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "list.h"

// Maximum number of file descriptors and clients allowed (used for listen)
#define MAX_PFDS 1000
#define MAX_CLIENTS 1000

// Sizes of various fields
#define IDSIZ 10
#define TYPESIZ 11
#define TOPICSIZ 51
#define CONTENTSIZ 1501
#define IPV4_LEN 16

// Constants for message types
#define SUBSCRIBE 0
#define UNSUBSCRIBE 1
#define EXIT 2

// Constants for message content types
#define INT 0
#define SHORT_REAL 1
#define FLOAT 2
#define STRING 3

// The subscription packet structure
typedef struct sub_packet_t {
	uint8_t type;
	char topic[TOPICSIZ];
	uint8_t sf;
} sub_packet_t;

// The TCP message structure
typedef struct tcp_msg_t {
	char type[TYPESIZ];
	char topic[TOPICSIZ];
	char content[CONTENTSIZ];
	char ip[IPV4_LEN];
	uint16_t port;
} tcp_msg_t;

// The UDP message structure
typedef struct udp_msg_t {
	char topic[TOPICSIZ - 1];
	uint8_t type;
	char content[CONTENTSIZ - 1];
} udp_msg_t;

// The client structure
typedef struct client_t {
	char id[IDSIZ];
	int socket;
	list_t *unsent; // unsent messages
	list_t *topics; // topics subscribed to
	bool online;
} client_t;

// The topic structure
typedef struct topic_t {
	char name[TOPICSIZ];
	uint8_t sf;
} topic_t;

// The sockets structure
typedef struct sockets_t {
	int tcp_sock;
	struct sockaddr_in tcp_addr;
	int udp_sock;
	struct sockaddr_in udp_addr;
	socklen_t len;
} sockets_t;

// The size of the subscription packet structure
#define PACKLEN sizeof(sub_packet_t)

#endif /* _STRUCTS_H */
