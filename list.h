/* SPDX-License-Identifier: EUPL-1.2 */
/* Copyright Mitran Andrei-Gabriel 2023 */

#ifndef _LIST_H_
#define _LIST_H_

// A node in a linked list
typedef struct node_t {
	void* data;
	struct node_t* next;
} node_t;

// A linked list
typedef struct list_t {
node_t* head; // pointer to the head of the list
unsigned int data_size; // size of the data in each node
} list_t;

/**
 * @brief Creates a new linked list.
 *
 * @param data_size The size of the data for each node in the list.
 *
 * @return A pointer to the newly created list, or NULL if malloc fails.
 */
list_t* list_create(unsigned int data_size);

/**
 * @brief Adds a new node with the specified data to the head of the list.
 *
 * @param list A pointer to the list.
 * @param new_data A pointer to the data to be added to the list.
 */
void list_add_head(list_t* list, void* new_data);

/**
 * @brief Frees the memory allocated for the list.
 *
 * @param list A pointer to the pointer to the list.
 */
void list_free(list_t** list);

#endif /* _LIST_H_ */
