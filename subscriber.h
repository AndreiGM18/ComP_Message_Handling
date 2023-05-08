/* SPDX-License-Identifier: EUPL-1.2 */
/* Copyright Mitran Andrei-Gabriel 2023 */

#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "structs.h"

/**
 * @brief Sets up a connection to a server and sends the client's ID.
 *
 * @param pfds Pointer to an array of pollfd structs
 * @param nfds Pointer to the number of file descriptors in the pfds array
 * @param id The client's ID.
 * @param ip The IP address of the server to connect to.
 * @param port The port number to connect to on the server.
 *
 * @return The TCP socket file descriptor.
 */
int setup(struct pollfd *pfds, int *nfds, char *id, char *ip, char *port);

/**
 * @brief Processes a command entered by the user on standard input.
 *
 * @param tcp_sock The TCP socket file descriptor.
 * @param buffer The buffer to store the command.
 *
 * @return Whether the program should continue running.
 */
bool stdin_cmd(int tcp_sock, char *buffer);

/**
 * @brief Takes a pointer to a sub_packet_t struct, the buffer and the packet's
 * type and fills in the fields of the struct based on the content of the
 * buffer and the type.
 *
 * @param pack - A pointer to the sub_packet_t struct to be filled in.
 * @param buffer - A string buffer to be parsed for topic and sf fields.
 * @param type - The type of the packet to be created.
 */
void create_packet(sub_packet_t *pack, char *buffer, uint8_t type);

/**
 * @brief Processes a message received from the server.
 *
 * @param tcp_sock The TCP socket file descriptor.
 * @param buffer The buffer to store the message.
 *
 * @return Whether the program should continue running.
 */
bool server_cmd(int tcp_sock, char *buffer);

#endif /* _SUBSCRIBER_H_ */
