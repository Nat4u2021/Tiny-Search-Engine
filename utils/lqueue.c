/*
 * lqueue.c -- Locked Queue ADT
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: implementation of Locked Queue ADT for multi-threaded processing
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <lqueue.h>
#include <queue.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* initialize empty locked queue */
lqueue_t *lqopen(void)
{
    pthread_mutex_lock(&mutex); // Lock the mutex
    queue_t *lqueue = qopen();
    pthread_mutex_unlock(&mutex); // Unlock the mutex
    return (lqueue_t *)lqueue;
}

/* deallocate a locked queue, frees everything in it */
void lqclose(lqueue_t *lqueue)
{
    pthread_mutex_lock(&mutex); // Lock the mutex
    qclose((queue_t *)lqueue);
    pthread_mutex_unlock(&mutex);  // Unlock the mutex
    pthread_mutex_destroy(&mutex); // Destroy the mutex
}

/* put element at the end of the locked queue
 * returns 0 is successful; nonzero otherwise
 */
int32_t lqput(lqueue_t *lqueue, void *elementp)
{
    int32_t status;             // keep track of whether the operation was successful
    pthread_mutex_lock(&mutex); // Lock the mutex
    status = qput((queue_t *)lqueue, elementp);
    pthread_mutex_unlock(&mutex); // Unlock the mutex
    return status;
}

/* get the first first element from locked queue, removing it from the queue */
void *lqget(lqueue_t *lqueue)
{
    pthread_mutex_lock(&mutex); // Lock the mutex
    void *data = qget(lqueue);
    pthread_mutex_unlock(&mutex); // Unlock the mutex
    return data;
}

/* apply a function to every element of the locked queue */
void lqapply(lqueue_t *lqueue, void (*fn)(void *elementp))
{
    pthread_mutex_lock(&mutex); // Lock the mutex
    qapply((queue_t *)lqueue, fn);
    pthread_mutex_unlock(&mutex); // Unlock the mutex
}

/* search a locked queue using a supplied boolean function
 * skeyp -- a key to search for
 * searchfn -- a function applied to every element of the queue
 *          -- element - a pointer to an element
 *          -- keyp - the key being searched for (i.e. will be
 *             set to skey at each step of the search
 *          -- returns TRUE or FALSE as defined in bool.h
 * returns a pointer to an element, or NULL if not found
 */
void *lqsearch(lqueue_t *lqueue, bool (*searchfn)(void *element, const void *keyp), const void *skeyp)
{
    pthread_mutex_lock(&mutex); // Lock the mutex
    void *data = qsearch((queue_t *)lqueue, searchfn, skeyp);
    pthread_mutex_unlock(&mutex); // Unlock the mutex
    return data;
}
