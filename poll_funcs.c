// SPDX-License-Identifier: EUPL-1.2
/* Copyright Mitran Andrei-Gabriel 2023 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

#include "poll_funcs.h"

void add_socket(struct pollfd *pfds, int *nfds, int socket) {
	// Sets the file descriptor of the new socket in the pollfd array
	pfds[*nfds].fd = socket;

	// Sets the events the poller is interested in for the new socket
	pfds[*nfds].events = POLLIN;

	// Updates the number of fds
	++(*nfds);
}

void remove_socket(struct pollfd *pfds, int *nfds, int i) {
	// Shifts all elements in the array after the one being removed to the left
	// by one
	for (int j = i; j < *nfds - 1; ++j)
		pfds[j] = pfds[j + 1];

	// Clears the last element in the array
	pfds[*nfds - 1].fd = EMPTY;
	pfds[*nfds - 1].events = pfds[*nfds - 1].revents = 0;

	// Updates the number of fds
	--(*nfds);
}
