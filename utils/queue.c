/*
 * queue.c -- Queue ADT
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: implementation of Queue ADT
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

/* the queue representation is hidden from users of the module */
typedef struct qelement
{
    void *element;
    struct qelement *next;
} qelement_t;

typedef struct queueWrapper
{
    qelement_t *front;
    qelement_t *back;
} queueWrapper_t;

/* initialize empty queue */
queue_t *qopen(void)
{
    queueWrapper_t *qp = (queueWrapper_t *)malloc(sizeof(queueWrapper_t));
    if (qp == NULL)
        return NULL;
    qp->front = NULL;
    qp->back = NULL;
    return (queue_t *)qp;
}

/* deallocate a queue, frees everything in it */
void qclose(queue_t *qp)
{
    if (qp == NULL)
        return;
    queueWrapper_t *q = (queueWrapper_t *)qp;
    qelement_t *curr = q->front;
    qelement_t *next;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr->element);
        free(curr);
        curr = next;
    }
    q->front = NULL;
    q->back = NULL;
    free(q);
}

/* put element at the end of the queue
 * returns 0 is successful; nonzero otherwise
 */
int32_t qput(queue_t *qp, void *elementp)
{
    queueWrapper_t *q = (queueWrapper_t *)qp;
    qelement_t *qep = (qelement_t *)malloc(sizeof(qelement_t));
    if (qep == NULL)
    {
        return -1;
    }
    qep->element = elementp;
    qep->next = NULL;
    if (q->back == NULL)
    {
        q->front = qep;
        q->back = qep;
    }
    else
    {
        q->back->next = qep;
        q->back = qep;
    }
    return 0;
}

/* get the first first element from queue, removing it from the queue */
void *qget(queue_t *qp)
{
    if (qp == NULL)
    {
        return NULL;
    }
    queueWrapper_t *q = (queueWrapper_t *)qp;
    qelement_t *qep = q->front;
    if (q->front == NULL)
    {
        return NULL;
    }
    void *data = qep->element;
    q->front = qep->next;
    if (q->front == NULL)
    {
        q->back = NULL;
    }
    free(qep);
    return data;
}

/* apply a function to every element of the queue */
void qapply(queue_t *qp, void (*fn)(void *elementp))
{
    if (qp == NULL)
    {
        return;
    }
    queueWrapper_t *q = (queueWrapper_t *)qp;
    qelement_t *curr;
    for (curr = q->front; curr != NULL; curr = curr->next)
    {
        fn((void *)curr->element);
    }
}

/* search a queue using a supplied boolean function
 * skeyp -- a key to search for
 * searchfn -- a function applied to every element of the queue
 *          -- element - a pointer to an element
 *          -- keyp - the key being searched for (i.e. will be
 *             set to skey at each step of the search
 *          -- returns TRUE or FALSE as defined in bool.h
 * returns a pointer to an element, or NULL if not found
 */
void *qsearch(queue_t *qp, bool (*searchfn)(void *element, const void *keyp), const void *skeyp)
{
    if (qp == NULL)
    {
        return NULL;
    }

    queueWrapper_t *q = (queueWrapper_t *)qp;
    qelement_t *curr;
    for (curr = q->front; curr != NULL; curr = curr->next)
    {
        if (searchfn((void *)(curr->element), skeyp))
        {
            return curr->element;
        }
    }
    return NULL;
}

/* search a queue using a supplied boolean function (as in qsearch),
 * removes the element from the queue and returns a pointer to it or
 * NULL if not found
 */
void *qremove(queue_t *qp, bool (*searchfn)(void *element, const void *keyp), const void *skeyp)
{
    if (qp == NULL)
    {
        return NULL;
    }
    queueWrapper_t *q = (queueWrapper_t *)qp;
    void *data = NULL;

    qelement_t *curr, *prev = NULL;
    bool res;
    for (curr = q->front; curr != NULL; prev = curr, curr = curr->next)
    {
        res = searchfn(curr->element, skeyp);
        if (res)
        {
            if (prev == NULL)
            {
                q->front = curr->next;
            }
            else
            {
                prev->next = curr->next;
            }
            data = curr->element;
            free(curr);
            break;
        }
    }
    return data;
}

/* concatenatenates elements of q2 into q1
 * q2 is dealocated, closed, and unusable upon completion
 */
void qconcat(queue_t *q1p, queue_t *q2p)
{
    if (q1p == NULL || q2p == NULL)
    {
        return;
    }
    queueWrapper_t *q1 = (queueWrapper_t *)q1p;
    queueWrapper_t *q2 = (queueWrapper_t *)q2p;

    if (q1->front == NULL)
    {
        q1->front = q2->front;
        q1->back = q2->back;
    }
    else if (q2->front == NULL)
    {
        qclose(q2p);
        return;
    }
    else
    {
        q1->back->next = q2->front;
        q1->back = q2->back;
    }
    q2->front = NULL;
    q2->back = NULL;
    qclose(q2p);
}
