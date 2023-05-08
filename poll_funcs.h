/* SPDX-License-Identifier: EUPL-1.2 */
/* Copyright Mitran Andrei-Gabriel 2023 */

#ifndef _POLL_FUNCS_H
#define _POLL_FUNCS_H

#define EMPTY -1

/**
 * @brief Adds a socket to the pollfd array
 *
 * @param pfds pointer to the pollfd array
 * @param nfds pointer to the number of file descriptors in the pollfd array
 * @param socket file descriptor to be added to the pollfd array
 */
void add_socket(struct pollfd *pfds, int *nfds, int socket);

/**
 * @brief Removes a socket from the pollfd array
 *
 * @param pfds pointer to the pollfd array
 * @param nfds pointer to the number of file descriptors in the pollfd array
 * @param i index of the file descriptor to be removed from the pollfd array
 */
void remove_socket(struct pollfd *pfds, int *nfds, int i);

#endif /* _POLL_FUNCS_H */
