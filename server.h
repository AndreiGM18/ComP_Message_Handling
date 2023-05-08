/* SPDX-License-Identifier: EUPL-1.2 */
/* Copyright Mitran Andrei-Gabriel 2023 */

#ifndef _SERVER_H_
#define _SERVER_H_

#include "structs.h"

/**
 * @brief Sets up a TCP and UDP server on the specified port and returns a
 * struct containing the socket file descriptors and socket addresses.
 *
 * @param pfds Pointer to an array of pollfd structs
 * @param nfds Pointer to the number of file descriptors in the pfds array
 * @param port The port number to listen on
 * @return A pointer to a struct containing the socket fds and addrs
 */
sockets_t *setup_server(struct pollfd *pfds, int *nfds, char *port);

/**
 * @brief Reads user input from standard input and checks if it is the "exit"
 * command.
 *
 * @param buffer The buffer to store the user input in
 *
 * @return True if the input is not "exit", false otherwise
 */
bool stdin_cmd(char *buffer);

/**
 * @brief Handles TCP connections by accepting a new client and adding it to the
 * clients list if it does not already exist.
 * If the client already exists, it reconnects the client and sends any unsent
 * messages.
 *
 * @param pfds Pointer to an array of pollfd structs
 * @param nfds Pointer to the number of file descriptors in the pfds array
 * @param clients Pointer to the list of connected clients
 * @param socks Pointer to the struct containing the socket fds and addrs
 * @param buffer The buffer to store incoming data in
 */
void tcp(struct pollfd *pfds, int *nfds, list_t *clients, sockets_t *socks,
			char *buffer);

/**
 * @brief Handles incoming UDP messages by forwarding them to subscribed
 * clients.
 *
 * @param clients Pointer to the list of connected clients
 * @param socks Pointer to the struct containing the socket fds and addrs
 * @param buffer The buffer to store incoming data in
 */
void udp(list_t *clients, sockets_t *socks, char *buffer);

/**
 * @brief Handles packets from subscribers.
 *
 * @param pfds Pointer to an array of pollfd structs
 * @param nfds Pointer to the number of file descriptors in the pfds array
 * @param clients Pointer to the list of connected clients
 * @param i The client's fd's position relative to the array of pollfd structs
 * @param buffer The buffer to store incoming data in
 */
void subscriber_protocol(struct pollfd *pfds, int *nfds, list_t *clients,
							int i, char *buffer);

#endif /* _SERVER_H_ */
