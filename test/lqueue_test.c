/*
 * lqueue_test.c -- tests the locked queue module
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: tests for single and multithreading using a locked queue
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <lqueue.h>

static void double_int(void *ep)
{
    *((int *)ep) *= 2;
}

static void print_int(void *ep)
{
    printf("%d -> ", *((int *)ep));
}

static bool compare_int(void *ep, const void *keyp)
{
    return *((int *)ep) == *((int *)keyp);
}

static void *thread_function(void *qp)
{
    static int thread_counter = 1;
    lqueue_t *lqueue = (lqueue_t *)qp;
    int thread_id = thread_counter++;

    printf("Thread %d starting...\n", thread_id);

    // put elements in the queue
    for (int i = 0; i < 5; i++)
    {
        int *data = malloc(sizeof(int));
        *data = i;
        lqput(lqueue, data);
        printf("Thread %d added element %d\n", thread_id, i);
        sleep(1);
    }

    // get from the queue
    for (int i = 0; i < 3; i++)
    {
        int *data = (int *)lqget(lqueue);
        printf("Thread %d got element %d\n", thread_id, *data);
        free(data);
        sleep(1);
    }

    // search for an element in the queue
    int key = 4;
    int *data = (int *)lqsearch(lqueue, compare_int, &key);
    if (data != NULL)
    {
        printf("Thread %d found element %d\n", thread_id, *data);
    }
    else
    {
        printf("Thread %d did not find element %d\n", thread_id, key);
    }

    // apply a function to each element in the queue
    lqapply(lqueue, double_int);

    thread_counter = 1;
    return NULL;
}

static void test_threads(int num_threads)
{
    lqueue_t *lqueue = lqopen();

    // create threads
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, thread_function, lqueue);
    }

    // wait for the threads
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("All threads complete\n");

    // print the remaining elements in the queue
    printf("Remaining elements in the queue:\n");
    lqapply(lqueue, print_int);
    printf("\n\n");

    lqclose(lqueue);
}

int main(void)
{
    // test single thread
    printf("Testing single thread...\n\n");
    test_threads(1);

    // test multiple threads
    printf("Testing multiple threads...\n\n");
    test_threads(4);

    exit(EXIT_SUCCESS);
}
