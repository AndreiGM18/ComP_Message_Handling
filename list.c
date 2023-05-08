// SPDX-License-Identifier: EUPL-1.2
/* Copyright Mitran Andrei-Gabriel 2023 */

#include "list.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

list_t* list_create(unsigned int data_size)
{
	// Allocates memory for a new list.
	list_t *new_list = malloc(sizeof(list_t));
	DIE(!new_list, "new list malloc() failed");

	// Initializes the new list.
	new_list->head = NULL;
	new_list->data_size = data_size;

	// Returns the new list.
	return new_list;
}

void list_add_head(list_t* list, void* new_data)
{
	// If list is NULL, returns.
	if (!list)
		return;

	// Allocates memory for a new node.
	node_t *new = (node_t *)malloc(sizeof(node_t));
	DIE(!new, "new node malloc() failed");

	// Allocates memory for the data in the new node.
	new->data = calloc(1, list->data_size);
	DIE(!new->data, "new node's data calloc() failed");

	// Copies the new data into the new node.
	memcpy(new->data, new_data, list->data_size);

	// Adds the new node to the head of the list.
	new->next = list->head;
	list->head = new;
}

void list_free(list_t** list)
{
	// If the list is NULL, returns.
	if (!(*list))
		return;

	// If the head of the list is NULL, frees the list and sets the pointer to NULL.
	if (!(*list)->head) {
		free(*list);
		*list = NULL;
		return;
	}

	// Frees the head node and its data.
	node_t *it = (*list)->head;
	node_t *tmp = it->next;

	free(it->data);
	free(it);

	// Frees the rest of the nodes and their data.
	while (tmp) {
		it = tmp;
		tmp = it->next;
		free(it->data);
		free(it);
	}

	// Frees the list and sets the pointer to NULL.
	free(*list);
	*list = NULL;
}
