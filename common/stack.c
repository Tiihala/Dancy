/*
 * Copyright (c) 2020 Antti Tiihala
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * stack.c
 *      Simple stack object
 */

#include <stddef.h>

int stack_create(void **instance);
int stack_delete(void *stack);

int stack_push(void *stack, size_t size, const void *data);
int stack_pop(void *stack, size_t size, void *data);

extern void free(void *ptr);
extern void *malloc(size_t size);
extern void *memcpy(void *s1, const void *s2, size_t n);

struct stack_node {
	struct stack_node *next;
	unsigned char *data;
	size_t size;
};

struct stack_instance {
	struct stack_node *head;
};

#define this_stack ((struct stack_instance *)(stack))

int stack_create(void **instance)
{
	struct stack_instance *stack;

	stack = malloc(sizeof(*stack));
	if ((*instance = stack) == NULL)
		return 1;

	stack->head = NULL;
	return 0;
}

int stack_delete(void *stack)
{
	if (stack == NULL)
		return 1;

	while (this_stack->head != NULL) {
		struct stack_node *node = this_stack->head;

		this_stack->head = node->next;
		free(node->data);
		free(node);
	}

	free(stack);
	return 0;
}

int stack_push(void *stack, size_t size, const void *data)
{
	struct stack_node *node = malloc(sizeof(*node));

	if (node == NULL)
		return 1;

	node->next = this_stack->head;
	node->size = size;
	node->data = NULL;

	if (size != 0) {
		node->data = malloc(size);
		if (node->data == NULL) {
			free(node);
			return 1;
		}
	}

	this_stack->head = node;

	if (size != 0)
		memcpy(node->data, data, size);
	return 0;
}

int stack_pop(void *stack, size_t size, void *data)
{
	struct stack_node *node = this_stack->head;

	if (node == NULL)
		return 1;

	this_stack->head = node->next;

	if (size > node->size)
		size = node->size;

	if (size != 0)
		memcpy(data, node->data, size);

	free(node->data);
	free(node);
	return 0;
}
