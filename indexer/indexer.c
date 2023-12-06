/* indexer.c --- indexes the words in crawled pages based on frequency
 *
 * Authors: Nathaniel Mensah
 * Created: Tue Jan 31 23:57:45 2023 (-0500)
 * Version: 1.0
 *
 * Description: The indexer is a program that reads the html associated with
 * every webpage fetched by the crawler; it constructs in memory an index
 * data structure that can be used to look up a word and find out 1) which documents (in the crawler
 * directory) contain the word, and 2) how many times the word occurs in that document.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pageio.h>
#include <indexio.h>
#include <hash.h>
#include <queue.h>

#define hsize 1000 // hashtable size

static int total_count = 0;

/* searches for entry in the hash table */
static bool entry_searchfn(void *elementp, const void *searchkeyp)
{
	entry_t *ep = (entry_t *)elementp;
	return strcmp(ep->word, (char *)searchkeyp) == 0;
}

/* searches for document in queue */
static bool doc_searchfn(void *elementp, const void *id)
{
	document_t *dp = (document_t *)elementp;
	return dp->id == *(int *)id;
}

/* total word count in the queue ie. word count for a specific word */
static void queue_sum_fn(void *elementp)
{
	document_t *dp = (document_t *)elementp;
	total_count += dp->word_count;
}

/* total word count */
static void total_sum_fn(void *ep)
{
	entry_t *p = (entry_t *)ep;
	qapply(p->documents, queue_sum_fn);
}

static int compare_func(const void *a, const void *b)
{
	const int *val_a = (const int *)a;
	const int *val_b = (const int *)b;
	if (*val_a < *val_b)
	{
		return -1;
	}
	else if (*val_a > *val_b)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void NormalizeWord(char *word)
{
	if (!word)
		return;

	int len = strlen(word);

	if (len < 3)
	{
		word[0] = '\0';
	}

	int i;
	for (i = 0; i < len; i++)
	{
		if (!isalpha(word[i]))
		{
			word[0] = '\0';
			return;
		}
		word[i] = tolower(word[i]);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("usage: indexer <pagedir> <indexnm>\n");
		exit(EXIT_FAILURE);
	}

	char *dirname = argv[1];
	struct stat st_dir;

	/* check if <pagedir> exists */
	if (stat(dirname, &st_dir) != 0 || !S_ISDIR(st_dir.st_mode))
	{
		printf("Error: %s doesn't exist\n", dirname);
		exit(EXIT_FAILURE);
	}

	hashtable_t *index = hopen(hsize);
	webpage_t *page;
	DIR *dir;
	struct dirent *dir_entry;
	int count = 0;
	int *files = NULL;

	/* open directory */
	dir = opendir(dirname);
	if (dir == NULL)
	{
		printf("Failed to open %s\n", dirname);
		exit(EXIT_FAILURE);
	}

	/* add file ids to array */
	while ((dir_entry = readdir(dir)) != NULL)
	{
		if (dir_entry->d_name[0] != '.')
		{
			int id = atoi(dir_entry->d_name);
			count++;
			files = realloc(files, count * sizeof(int));
			files[count - 1] = id;
		}
	}
	closedir(dir);

	/* sort the files in order using compare_func */
	qsort(files, count, sizeof(int), compare_func);

	/* loop over files */
	for (int i = 0; i < count; i++)
	{
		printf("loading page id: %d ...\n", files[i]);
		page = pageload(files[i], dirname);

		if (!page)
			exit(EXIT_FAILURE);

		int pos = 0;
		char *word;
		entry_t *ep;

		while ((pos = webpage_getNextWord(page, pos, &word)) > 0)
		{
			NormalizeWord(word);
			if (word[0] != '\0')
			{
				if (hsearch(index, entry_searchfn, word, strlen(word)))
				{
					ep = (entry_t *)hsearch(index, entry_searchfn, word, strlen(word));
					document_t *dp;
					if ((dp = qsearch(ep->documents, doc_searchfn, &files[i])))
					{
						dp->word_count = dp->word_count + 1;
					}
					else
					{
						dp = new_doc(files[i], 1);
						qput(ep->documents, dp);
					}
				}
				else
				{
					ep = new_entry(word);
					document_t *dp = new_doc(files[i], 1);
					qput(ep->documents, dp);
					hput(index, ep, word, strlen(word));
				}
				// printf("%s\n",word);
			}
			free(word);
		}
		printf("page id: %d loaded successfully.\n", files[i]);
		webpage_delete(page);
	}

	happly(index, total_sum_fn);
	printf("Total word count in hashtable: %d\n", total_count);

	free(files);
	if (indexsave(index, argv[2]) != 0)
	{
		exit(EXIT_FAILURE);
	}
	free_entries(index);
	hclose(index);
	exit(EXIT_SUCCESS);
}

/*
 * 1 - 141
 * 2 - 73
 * 3 - 109
 */
