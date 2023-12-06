/*
 * lhash_test.c -- tests the locked hash module
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: tests for single and multithreading using a locked hash table
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <lhash.h>

static void print_int(void *ep)
{
    printf("%d -> ", *((int *)ep));
}

static bool searchfn(void *ep, const void *key)
{
    int ep_value = *(int *)ep;
    int key_value = atoi((const char *)key);
    return ep_value == key_value;
}

static void *thread_function(void *htp)
{
    static int thread_counter = 1;
    lhash_t *htable = (lhash_t *)htp;
    int thread_id = thread_counter++;

    printf("Thread %d starting...\n", thread_id);

    // put elements in the hash table
    for (int i = 0; i < 4; i++)
    {
        int *data = malloc(sizeof(int));
        *data = i;
        char key[2];
        sprintf(key, "%d", *data);
        lhput(htable, data, key, strlen(key));
        printf("Thread %d added element %d\n", thread_id, i);
        sleep(1);
    }

    // remove from the hash table
    for (int i = 0; i < 3; i++)
    {
        char key[2];
        sprintf(key, "%d", i);
        int *data = (int *)lhremove(htable, searchfn, key, strlen(key));
        if (data != NULL)
        {
            printf("Thread %d removed element %d\n", thread_id, *((int *)data));
        }
        else
        {
            printf("Thread %d did not find element %s\n", thread_id, key);
        }
        free(data);
    }

    // search for an element in the hash table
    char *key = "3";
    void *data = lhsearch(htable, searchfn, key, strlen(key));
    if (data != NULL)
    {
        printf("Thread %d found element %d\n", thread_id, *((int *)data));
    }
    else
    {
        printf("\nThread %d did not find element %s\n", thread_id, key);
    }

    // apply a function to each element in the hash table
    sleep(1);
    printf("Printing remaining elements in the hash table with apply:\n");
    lhapply(htable, print_int);
    printf("\n");
    sleep(1);

    thread_counter = 1;
    return NULL;
}

static void test_threads(int num_threads)
{
    lhash_t *htable = lhopen(10);

    // create threads
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, thread_function, htable);
    }

    // wait for the threads
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("All threads complete\n");
    lhclose(htable);
}

int main(void)
{
    // test single thread
    printf("#################################\n");
    printf("Testing single thread...\n\n");
    test_threads(1);
    printf("#################################\n\n");

    // test multiple threads
    printf("#################################\n");
    printf("Testing multiple threads...\n\n");
    test_threads(4);
    printf("#################################\n");

    exit(EXIT_SUCCESS);
}
