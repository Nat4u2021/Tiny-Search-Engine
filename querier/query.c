/* query.c --- reads query from user, consults an index and ranks crawled pages
 *
 * Authors: Nathaniel Mensah
 * Created: Wed Feb 15 21:57:45 2023 (-0500)
 * Version: 1.0
 *
 * Description: program that reads a query from the user, consults the index built by the indexer,
 * ranks documents fetched by the crawler according to their relevance,
 * and prints a list of documents in rank order. In general, queries are a sequence of words
 * separated by spaces with optiona  boolean operators AND and OR, where AND has precedence over OR.
 * By default, all words typed in a query are implicitly connected by logical-AND.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <hash.h>
#include <indexio.h>
#include <pageio.h>

#define MAX_QUERY_LEN 512

/**
 * @brief represents a ranked doc with id, ranked word_count and url
 */
typedef struct doc
{
    int id;
    int word_count;
    char *url;
    char *title;
    char *content;
} rankedDoc_t;

/*************************** PROTOTYPES ********************************/
/**
 * Initializes a ranked document
 *
 * @param id the doc id obtained from the index
 * @param rank the rank of the doc as evaluated from the query
 * @return a pointer to the initialized doc
 */
static rankedDoc_t *init_doc(int id, int rank);

/**
 * get user input from standard in
 *
 * @param buffer buffer for the input
 * @return -1 if end of file(EOF), 0 if successful
 */
static int get_input(char *buffer);

/**
 * split the query into an array of tokens
 *
 * @param query the raw query obtained from stdin
 * @param num_tokens the number of tokens in the raw query
 * @return an array of string tokens
 */
static char **tokenize_query(char *query, int *num_tokens);

/**
 * Normalizes a word (from the query) by converting to lowercase
 *
 * @param word the query token (string)
 * @return a boolean indicating whether the word is valid (alphabetic)
 */
static bool NormalizeWord(char *word);

/**
 * validates query for invalid syntax
 *
 * @param query an array containing the words in the query
 * @param num_tokens the number of tokens in the query
 * @return a boolean indicating whether the query is valid
 */
static bool validate_query(char **query, int num_tokens);

/**
 * parses the arguments from command line and assigns them
 *
 * @param pagedir a pointer to a buffer in which to write crawled pagedir
 * @param indexfile a pointer to a buffer in which to write the index file
 * @return 0 if successful, -1 if invalid
 */
static int parse_args(int argc, char *argv[], char **pagedir, char **indexfile);

/**
 * searches for a query token in the index
 *
 * @param elementp a pointer to an element in the index (elementp contains a word and wordcount)
 * @param key the token to search for
 * @return a boolean indicating whether the element's word is the same as the search token
 */
static bool token_searchfn(void *elementp, const void *key);

/**
 * searches for doc in the queue of documents
 *
 * @related token_searchfn
 */
static bool doc_searchfn(void *elementp, const void *id);

static int comparator(const void *a, const void *b);

static void sort_queue(queue_t **qp);

static void free_doc(rankedDoc_t *dp);

static void free_queue(queue_t *qp);

/**
 * sets the ranked page url, title and description
 *
 * @param ranked_docs the queue of ranked docs
 * @param pagedir the directory containing crawled pages
 */
static void get_metadata(queue_t *ranked_docs, char *pagedir);

/**
 * finds the intersection between two ranked queues
 *
 * @param qp1 the ranked queue
 * @param qp2 the queue of docs containing a token
 * @return a pointer to a queue containing the intersetion
 */
static queue_t *get_intersection(queue_t *qp1, queue_t *qp2);

/**
 * finds the union between two ranked queues in place
 * ie. the result is concatenated into qp1, qp2 is restored to original state
 *
 * @param qp1 the ranked queue
 * @param qp2 the queue of docs containing a token
 * @return a pointer to a queue containing the union
 */
static queue_t *get_union(queue_t *qp1, queue_t *qp2);

/*************************** MAIN ******************************/
int main(int argc, char *argv[])
{
    /* use case: query ../pages index < good-queries.txt > output */
    char *pagedir, *index_file;
    if (parse_args(argc, argv, &pagedir, &index_file) != 0)
    {
        exit(EXIT_FAILURE);
    }
    const char *and = "and", * or = "or";
    hashtable_t *index = indexload(index_file);

    char query[MAX_QUERY_LEN];
    char **tokenized_query, *token, *curr_operator;
    int num_tokens, top;
    entry_t *ep;
    rankedDoc_t *doc;
    queue_t *ranked_docs, **stack = NULL, *tmp, *qp1, *qp2;

    while (1)
    {
        num_tokens = 0;
        top = -1;
        curr_operator = "";

        /* get input from stdin */
        if (get_input(query) != 0)
        {
            break;
        }

        /* no query is entered */
        if (!query[0])
        {
            continue;
        }

        /* get array of tokens from query */
        tokenized_query = tokenize_query(query, &num_tokens);

        /* validate query */
        if (!tokenized_query || !validate_query(tokenized_query, num_tokens))
        {
            for (int i = 0; i < num_tokens; i++)
            {
                free(tokenized_query[i]);
            }
            free(tokenized_query);
            printf("[invalid query]\n");
            query[0] = '\0';
            continue;
        }

        /* process tokens in Backus-Naur Form */
        for (int i = 0; i < num_tokens; i++)
        {
            token = tokenized_query[i];

            /* if token is an operator, update current operator and continue */
            if (strcmp(token, and) == 0 || strcmp(token, or) == 0)
            {
                curr_operator = token;
                continue;
            }

            ep = hsearch(index, token_searchfn, token, strlen(token));
            tmp = qopen();
            if (ep)
            { // if token is present in index push its queue
                tmp = get_union(tmp, ep->documents);
            }
            stack = (queue_t **)realloc(stack, sizeof(queue_t *) * (top + 2));
            stack[++top] = tmp;

            /* if last operator is and, get intersect of prev two queues in stack */
            if (strcmp(curr_operator, and) == 0)
            {
                qp1 = stack[top--];
                qp2 = stack[top--];
                stack[++top] = get_intersection(qp1, qp2);
            }
        }

        /* union everything left in the stack */
        while (top > 0)
        {
            qp1 = stack[top--];
            qp2 = stack[top--];
            stack[++top] = get_union(qp1, qp2);
            free_queue(qp2);
        }
        ranked_docs = stack[top];

        /* set metadata -> url, title, content */
        get_metadata(ranked_docs, pagedir);

        /* sort ranked docs */
        sort_queue(ranked_docs);

        /* print docs' rank & url */
        while ((doc = qget(ranked_docs)))
        {
            printf("title: %s\nrank:%d doc:%d : %s\n", doc->title, doc->word_count, doc->id, doc->url);
            printf("%s...\n\n", doc->content);
            free_doc(doc);
        }

        /* free memory */
        for (int i = 0; i < num_tokens; i++)
        {
            free(tokenized_query[i]);
        }
        free(tokenized_query);
        free_queue(ranked_docs);
    }

    /* free memory */
    free(stack);
    free_entries(index);
    hclose(index);
    free(pagedir);
    free(index_file);
    exit(EXIT_SUCCESS);
}

/****************************************************************************/

/******************************** FUNCTIONS *********************************/
static int get_input(char *buffer)
{
    if (!buffer)
        return 1;
    buffer[0] = '\0';
    printf("> ");
    if (scanf("%[^\n]", buffer) == EOF)
    {
        printf("\n");
        return -1;
    }
    getchar(); // strip newline character
    return 0;
}

static char **tokenize_query(char *query, int *num_tokens)
{
    char **tokenized_query = NULL, *prev_token = NULL;
    char *token = strtok(query, " \t");
    int count = 0;
    while (token)
    {
        if (!NormalizeWord(token))
        {
            /* free memory */
            for (int i = 0; i < count; i++)
            {
                free(tokenized_query[i]);
            }
            free(tokenized_query);
            return NULL;
        }

        if (strlen(token) < 3 && strcmp(token, "or") != 0)
        {
            token = strtok(NULL, " \t");
            continue;
        }
        if (prev_token && strcmp(prev_token, "and") != 0 && strcmp(prev_token, "or") != 0)
        {
            // insert an implicit "and" before the current token
            if (strcmp(token, "and") != 0 && strcmp(token, "or") != 0)
            {
                tokenized_query = realloc(tokenized_query, (count + 1) * sizeof(char *));
                tokenized_query[count] = malloc(strlen("and") + 1);
                strcpy(tokenized_query[count], "and");
                count++;
            }
        }

        tokenized_query = realloc(tokenized_query, (count + 1) * sizeof(char *));
        tokenized_query[count] = malloc(strlen(token) + 1);
        strcpy(tokenized_query[count], token);
        prev_token = token;
        token = strtok(NULL, " \t");
        count++;
    }

    *num_tokens = count;
    return tokenized_query;
}

static rankedDoc_t *init_doc(int id, int rank)
{
    rankedDoc_t *doc;
    if (!(doc = (rankedDoc_t *)malloc(sizeof(rankedDoc_t))))
    {
        printf("Error in allocating memory\n");
        return NULL;
    }
    doc->id = id;
    doc->word_count = rank;
    doc->title = NULL;
    doc->url = NULL;
    doc->content = NULL;
    return doc;
}

static bool NormalizeWord(char *word)
{
    if (!word)
        return false;
    int len = strlen(word);
    int i;
    for (i = 0; i < len; i++)
    {
        if (!isalpha(word[i]))
        {
            return false;
        }
        word[i] = tolower(word[i]);
    }
    return true;
}

static bool validate_query(char **query, int num_tokens)
{
    if (strcmp(query[0], "and") == 0 || strcmp(query[0], "or") == 0 ||
        strcmp(query[num_tokens - 1], "and") == 0 ||
        strcmp(query[num_tokens - 1], "or") == 0)
    {
        return false;
    }
    for (int i = 0; i < num_tokens - 1; i++)
    {
        if (strcmp(query[i], "and") == 0)
        {
            if (strcmp(query[i + 1], "and") == 0 || strcmp(query[i + 1], "or") == 0)
                return false;
        }
        else if (strcmp(query[i], "or") == 0)
        {
            if (strcmp(query[i + 1], "and") == 0 || strcmp(query[i + 1], "or") == 0)
                return false;
        }
    }
    return true;
}

static queue_t *get_intersection(queue_t *qp1, queue_t *qp2)
{
    queue_t *intersect = qopen();
    if (!qp1 || !qp2 || !intersect)
        return NULL;
    // document_t *doc;
    rankedDoc_t *dp, *doc;
    int word_count, rank;
    while ((doc = qget(qp1)))
    {
        if ((dp = qsearch(qp2, doc_searchfn, &(doc->id))))
        {
            word_count = dp->word_count;
            rank = doc->word_count;
            doc->word_count = word_count < rank ? word_count : rank;
            qput(intersect, doc);
        }
        else
            free_doc(doc);
    }
    qclose(qp1);
    free_queue(qp2);
    // qclose(qp2);
    return intersect;
}

static queue_t *get_union(queue_t *qp1, queue_t *qp2)
{
    queue_t *tmp = qopen();
    if (!qp1 || !qp2 || !tmp)
        return NULL;
    document_t *dp;
    rankedDoc_t *doc;
    int word_count, rank;
    while ((dp = qget(qp2)))
    {
        word_count = dp->word_count;
        if ((doc = qsearch(qp1, doc_searchfn, &(dp->id))))
        {
            rank = doc->word_count;
            doc->word_count = rank + word_count;
        }
        else
        {
            doc = init_doc(dp->id, word_count);
            qput(qp1, doc);
        }
        qput(tmp, dp);
    }

    // restore qp2
    while ((dp = qget(tmp)))
        qput(qp2, dp);
    qclose(tmp);
    return qp1;
}

static void get_metadata(queue_t *ranked_docs, char *pagedir)
{
    rankedDoc_t *dp;
    queue_t *tmp = qopen();
    int id, len;
    webpage_t *page;
    char *url, *html, *start, *end, *content;
    while ((dp = qget(ranked_docs)))
    {
        id = dp->id;
        if ((page = pageload(id, pagedir)))
        {
            url = webpage_getURL(page);
            html = webpage_getHTML(page);
            if ((dp->url = malloc(strlen(url) + 1)))
            {
                strcpy(dp->url, url);
            }
            start = strstr(html, "<title>");
            if (start != NULL)
            {
                end = strstr(start, "</title>");
                if (end != NULL)
                {
                    len = end - start - strlen("<title>");
                    if ((dp->title = malloc(len + 1)))
                    {
                        strncpy(dp->title, start + strlen("<title>"), len);
                        dp->title[len] = '\0';
                    }
                }
            }
            start = NULL, end = NULL;
            start = strstr(html, "<meta name=\"description\"");
            if (start != NULL)
            {
                content = strstr(start, "content=\"");
                if (content != NULL)
                {
                    content += strlen("content=\"");
                    end = strchr(content, '\"');
                    if (end != NULL)
                    {
                        int len = (int)(end - content);
                        if (len > 128)
                        {
                            len = 128;
                        }
                        if ((dp->content = malloc(len + 1)))
                        {
                            strncpy(dp->content, content, len);
                            dp->content[len] = '\0';
                        }
                    }
                }
            }
        }
        qput(tmp, dp);
        webpage_delete(page);
    }

    while ((dp = qget(tmp)))
        qput(ranked_docs, dp);
    qclose(tmp);
}

static int parse_args(int argc, char *argv[], char **pagedir, char **indexfile)
{
    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "usage: query <pageDirectory> <indexFile> [-q]\n");
        return -1;
    }
    if (argc == 4 && strcmp(argv[3], "-q") != 0)
    {
        fprintf(stderr, "usage: query <pageDirectory> <indexFile> [-q]\n");
        return -1;
    }
    if (!(*pagedir = malloc(strlen(argv[1]) + 1)))
    {
        fprintf(stderr, "error: failed to allocate memory for page directory\n");
        return -1;
    }
    if (!(*indexfile = malloc(strlen(argv[2]) + 1)))
    {
        fprintf(stderr, "error: failed to allocate memory for page directory\n");
        return -1;
    }
    struct stat dir_stat, file_stat;
    // Verify pagedir exists and is a directory
    if (stat(argv[1], &dir_stat) != 0)
    {
        fprintf(stderr, "Error: pagedir '%s' does not exist or cannot be accessed.\n", argv[1]);
        return -1;
    }
    if (!S_ISDIR(dir_stat.st_mode))
    {
        fprintf(stderr, "Error: '%s' is not a directory.\n", argv[1]);
        return -1;
    }

    // Verify indexfile exists and is a readable file
    if (stat(argv[2], &file_stat) != 0)
    {
        fprintf(stderr, "Error: indexfile '%s' does not exist or cannot be accessed.\n", argv[2]);
        return -1;
    }
    if (!S_ISREG(file_stat.st_mode))
    {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", argv[2]);
        return -1;
    }
    if (access(argv[2], R_OK) != 0)
    {
        fprintf(stderr, "Error: '%s' cannot be read.\n", argv[2]);
        return -1;
    }
    strcpy(*pagedir, argv[1]);
    strcpy(*indexfile, argv[2]);
    return 0;
}

static bool token_searchfn(void *elementp, const void *key)
{
    entry_t *ep = (entry_t *)elementp;
    return strcmp(ep->word, (char *)key) == 0;
}

static bool doc_searchfn(void *elementp, const void *id)
{
    rankedDoc_t *dp = (rankedDoc_t *)elementp;
    return dp->id == *((int *)id);
}

static int comparator(const void *a, const void *b)
{
    const rankedDoc_t *doc_a = *(const rankedDoc_t **)a;
    const rankedDoc_t *doc_b = *(const rankedDoc_t **)b;
    if (doc_a->word_count < doc_b->word_count)
    {
        return 1;
    }
    else if (doc_a->word_count > doc_b->word_count)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static void sort_queue(queue_t **qp)
{
    queue_t **tmp = NULL;
    int count = 0;

    // loop through the queue, adding each document to the array
    rankedDoc_t *doc;
    while ((doc = (rankedDoc_t *)qget(qp)) != NULL)
    {
        // resize the array to hold another element
        count++;
        tmp = realloc(tmp, count * sizeof(rankedDoc_t *));

        // add the document to the end of the array
        tmp[count - 1] = doc;
    }

    // sort the array based on the rank
    qsort(tmp, count, sizeof(rankedDoc_t *), comparator);

    // add the sorted documents back to the queue
    for (int i = 0; i < count; i++)
    {
        qput(qp, tmp[i]);
    }

    free(tmp);
}

static void free_doc(rankedDoc_t *dp)
{
    if (dp->title)
        free(dp->title);
    if (dp->url)
        free(dp->url);
    if (dp->content)
        free(dp->content);
    if (dp)
        free(dp);
}

static void free_queue(queue_t *qp)
{
    rankedDoc_t *dp;
    while ((dp = qget(qp)))
        free_doc(dp);
    qclose(qp);
}
