/*
 * Copyright (c) 2026 Antti Tiihala
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
 * libc/search/bst.c
 *      Manage a binary search tree
 */

#include <limits.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>

struct internal_tnode {
	const void *key;
	void *data[4];

	struct internal_tnode *owner;
	struct internal_tnode *members[2];
};

static struct internal_tnode empty_internal_tnode;
static void *non_null_pointer = &empty_internal_tnode;

static struct internal_tnode *alloc_tnode(const void *key, void *owner)
{
	size_t size = sizeof(struct internal_tnode);
	struct internal_tnode *t = malloc(size);

	if (t != NULL) {
		memcpy(t, &empty_internal_tnode, sizeof(*t));
		t->key = key;
		t->owner = owner;
	}

	return t;
}

static void free_tnode(struct internal_tnode *t)
{
	memcpy(t, &empty_internal_tnode, sizeof(*t));
	free(t);
}

static void delete_tnode_leaf(struct internal_tnode *t, void **rootp)
{
	struct internal_tnode *owner = t->owner;

	if (owner == NULL) {
		*rootp = NULL;
	} else {
		int i = (owner->members[0] == t) ? 0 : 1;
		owner->members[i] = NULL;
	}

	free_tnode(t);
}

static void delete_tnode_one(struct internal_tnode *t, void **rootp)
{

	struct internal_tnode *owner = t->owner;
	int i = (t->members[0] != NULL) ? 0 : 1;

	if ((t->members[i]->owner = owner) == NULL) {
		*rootp = t->members[i];
	} else {
		int j = (owner->members[0] == t) ? 0 : 1;
		owner->members[j] = t->members[i];
	}

	free_tnode(t);
}

static void delete_tnode_two(struct internal_tnode *t, void **rootp)
{
	struct internal_tnode *m = t->members[0];

	while (m->members[1] != NULL)
		m = m->members[1];

	t->key = m->key;
	memcpy(&t->data[0], &m->data[0], sizeof(t->data));

	if (m->members[0] == NULL)
		delete_tnode_leaf(m, rootp);
	else
		delete_tnode_one(m, rootp);
}

void *tdelete(const void *key, posix_tnode **rootp,
	int (*compar)(const void *, const void *))
{
	struct internal_tnode *t;

	if (rootp == NULL)
		return NULL;

	t = (struct internal_tnode *)(*rootp);

	while (t != NULL) {
		int i, c;

		if ((c = compar(key, t->key)) == 0) {
			void *owner = t->owner;

			if (owner == NULL)
				owner = non_null_pointer;

			if (t->members[0] == NULL && t->members[1] == NULL) {
				delete_tnode_leaf(t, rootp);
				return owner;
			}

			if (t->members[0] == NULL || t->members[1] == NULL) {
				delete_tnode_one(t, rootp);
				return owner;
			}

			delete_tnode_two(t, rootp);
			return owner;
		}

		i = (c < 0) ? 0 : 1;
		t = t->members[i];
	}

	return NULL;
}

posix_tnode *tfind(const void *key, posix_tnode *const *rootp,
	int (*compar)(const void *, const void *))
{
	struct internal_tnode *t;

	if (rootp == NULL)
		return NULL;

	t = (struct internal_tnode *)(*rootp);

	while (t != NULL) {
		int i, c;

		if ((c = compar(key, t->key)) == 0)
			return t;

		i = (c < 0) ? 0 : 1;
		t = t->members[i];
	}

	return NULL;
}

posix_tnode *tsearch(const void *key, posix_tnode **rootp,
	int (*compar)(const void *, const void *))
{
	struct internal_tnode *t;

	if (rootp == NULL)
		return NULL;

	if ((t = (struct internal_tnode *)(*rootp)) == NULL)
		return (*rootp = alloc_tnode(key, NULL));

	while (t != NULL) {
		struct internal_tnode *owner = t;
		int i, c;

		if ((c = compar(key, t->key)) == 0)
			return t;

		i = (c < 0) ? 0 : 1;
		t = t->members[i];

		if (t == NULL && (t = alloc_tnode(key, owner)) != NULL)
			return (void *)(owner->members[i] = t);
	}

	return NULL;
}

static void internal_twalk(const void *n,
	void (*action)(const posix_tnode *, VISIT, int), int recursion)
{
	const struct internal_tnode *t = n;

	if (recursion == INT_MAX)
		return;

	if (t->members[0] == NULL && t->members[1] == NULL) {
		action((const void *)t, leaf, recursion);
		return;
	}

	action((const void *)t, preorder, recursion);

	if ((n = t->members[0]) != NULL)
		internal_twalk(n, action, recursion + 1);

	action((const void *)t, postorder, recursion);

	if ((n = t->members[1]) != NULL)
		internal_twalk(n, action, recursion + 1);

	action((const void *)t, endorder, recursion);
}

void twalk(const posix_tnode *root,
	void (*action)(const posix_tnode *, VISIT, int))
{
	if (root != NULL)
		internal_twalk(root, action, 0);
}
