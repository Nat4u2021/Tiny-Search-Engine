/*
 * indexio_test.c -- tests the indexio module
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: tests the indexsave() and indexload() functions
 * of the indexio utils
 */

#include <stdio.h>
#include "indexio.h"

int main(void)
{
    char *indexnm = "test_index";
    printf("Loading index...\n");
    hashtable_t *index = indexload(indexnm);
    if (index)
    {
        printf("Index loaded successfully from: %s\n", indexnm);
    }
    else
    {
        exit(EXIT_FAILURE);
    }

    char *indexcp = "test_indexcp";
    int status = indexsave(index, indexcp);
    if (status != 0)
    {
        printf("Failed to save index to %s\n", indexcp);
        exit(EXIT_FAILURE);
    }
    printf("Saved index successfully to: %s\n", indexcp);
    free_entries(index);
    hclose(index);

    index = indexload(indexcp);
    if (index)
    {
        printf("Index loaded successfully from: %s\n", indexcp);
    }

    free_entries(index);
    hclose(index);
    exit(EXIT_SUCCESS);
}
