#pragma once
/*
 * indexio.h --- saves and loads an index to a named file
 *
 * Author: Nathaniel Mensah
 * Created: Fri Feb 10 08:30:15 2023 (-0400)
 * Version: 1.0
 *
 * Description: indexsave saves an index to a named file;
 * indexload cloads an index from the named file
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "hash.h"
#include "queue.h"

/* index entry struct
 *
 * @param word - the word to add to the index
 * @param documents - the queue of crawled docs containing the word
 */
typedef struct entry
{
	char *word;
	queue_t *documents;
} entry_t;

/* document struct
 *
 * @param id - document id designated by crawler
 * @param word_count - the count of a specific word in the index in this doc
 */
typedef struct document
{
	int id;
	int word_count;
} document_t;

/* allocate index entry */
entry_t *new_entry(char *word);

/* allocate document */
document_t *new_doc(int id, int word_count);

/*
 * indexsave -- save the index to filename indexnm
 *
 * returns: 0 for success; nonzero otherwise
 */
int32_t indexsave(hashtable_t *index, char *indexnm);

/*
 * indexload -- loads the index from file indexnm
 *
 * returns: non-NULL for success; NULL otherwise
 *
 * the user is responsible for freeing the hash table
 */
hashtable_t *indexload(char *indexnm);

/*
 * free_entries -- frees all entry structs in the index
 */
void free_entries(hashtable_t *index);
