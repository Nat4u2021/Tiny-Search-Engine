/*
 * hash.c -- implements a generic hash table as an indexed set of queues.
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: implementation of Hashtable ADT
 */
#include <stdint.h>
#include <stdlib.h>
#include "hash.h"
#include "queue.h"

#define get16bits(d) (*((const uint16_t *)(d)))

typedef struct table
{
	uint32_t tablesize;
	queue_t **hash_table;
} table_t;

/*
 * SuperFastHash() -- produces a number between 0 and the tablesize-1.
 *
 * The following (rather complicated) code, has been taken from Paul
 * Hsieh's website under the terms of the BSD license. It's a hash
 * function used all over the place nowadays, including Google Sparse
 * Hash.
 */
static uint32_t SuperFastHash(const char *data, int len, uint32_t tablesize)
{
	uint32_t hash = len, tmp;
	int rem;

	if (len <= 0 || data == NULL)
		return 0;
	rem = len & 3;
	len >>= 2;
	/* Main loop */
	for (; len > 0; len--)
	{
		hash += get16bits(data);
		tmp = (get16bits(data + 2) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		data += 2 * sizeof(uint16_t);
		hash += hash >> 11;
	}
	/* Handle end cases */
	switch (rem)
	{
	case 3:
		hash += get16bits(data);
		hash ^= hash << 16;
		hash ^= data[sizeof(uint16_t)] << 18;
		hash += hash >> 11;
		break;
	case 2:
		hash += get16bits(data);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1:
		hash += *data;
		hash ^= hash << 10;
		hash += hash >> 1;
	}
	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;
	return hash % tablesize;
}

/* hopen -- opens a hash table with initial size hsize */
hashtable_t *hopen(uint32_t hsize)
{
	if (hsize <= 0)
		return NULL;
	table_t *table = malloc(sizeof(table_t));
	if (table == NULL)
		return NULL;

	table->tablesize = hsize;
	table->hash_table = malloc(hsize * sizeof(queue_t *));

	if (table->hash_table == NULL)
		return NULL;

	for (int i = 0; i < hsize; i++)
	{
		table->hash_table[i] = qopen();
	}
	return (hashtable_t *)table;
}

/* hclose -- closes a hash table */
void hclose(hashtable_t *htp)
{
	if (htp == NULL)
		return;

	table_t *table = (table_t *)htp;
	for (int i = 0; i < table->tablesize; i++)
	{
		qclose(table->hash_table[i]);
	}
	free(table->hash_table);
	free(table);
}

/* hput -- puts an entry into a hash table under designated key
 * returns 0 for success; non-zero otherwise
 */
int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen)
{
	if (htp == NULL || ep == NULL)
		return -1;
	table_t *table = (table_t *)htp;
	uint32_t hash = SuperFastHash(key, keylen, table->tablesize);

	return qput(table->hash_table[hash], ep);
}

/* happly -- applies a function to every entry in hash table */
void happly(hashtable_t *htp, void (*fn)(void *ep))
{
	if (htp == NULL || fn == NULL)
		return;
	table_t *table = (table_t *)htp;
	for (int i = 0; i < table->tablesize; i++)
	{
		qapply(table->hash_table[i], fn);
	}
}

/* hsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 */
void *hsearch(hashtable_t *htp,
			  bool (*searchfn)(void *elementp, const void *searchkeyp),
			  const char *key,
			  int32_t keylen)
{
	if (htp == NULL || searchfn == NULL)
		return NULL;
	table_t *table = (table_t *)htp;
	uint32_t hash = SuperFastHash(key, keylen, table->tablesize);

	return qsearch(table->hash_table[hash], searchfn, key);
}

/* hremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *hremove(hashtable_t *htp,
			  bool (*searchfn)(void *elementp, const void *searchkeyp),
			  const char *key,
			  int32_t keylen)
{
	if (htp == NULL || searchfn == NULL)
		return NULL;
	table_t *table = (table_t *)htp;
	uint32_t hash = SuperFastHash(key, keylen, table->tablesize);

	return qremove(table->hash_table[hash], searchfn, key);
}
