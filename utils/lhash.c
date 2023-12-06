/* lhash.c --- locked hashtable implementation
 *
 *
 * Author: Nathaniel Mensah
 * Created: Sat Feb 25 22:30:54 2023 (-0500)
 * Version: 1.0
 *
 * Description: locked generic hastable implementation for multithreading
 */

#include <queue.h>
#include <hash.h>
#include <stdio.h>
#include <lhash.h>
#include <pthread.h>

pthread_mutex_t mutex_h = PTHREAD_MUTEX_INITIALIZER;

/* lhopen -- opens a hash table with initial size hsize */
hashtable_t *lhopen(uint32_t lhsize)
{
    pthread_mutex_lock(&mutex_h);
    lhash_t *lhtp = (lhash_t *)hopen(lhsize);
    pthread_mutex_unlock(&mutex_h);
    return lhtp;
}

/* lhclose -- closes a hash table */
void lhclose(lhash_t *lhtp)
{
    pthread_mutex_lock(&mutex_h);
    hclose((hashtable_t *)lhtp);
    pthread_mutex_unlock(&mutex_h);
    pthread_mutex_destroy(&mutex_h); // Destroy the mutex
}

/* lhput -- puts an entry into a hash table under designated key
 * returns 0 for success; non-zero otherwise
 */
int32_t lhput(lhash_t *lhtp, void *ep, const char *key, int keylen)
{
    pthread_mutex_lock(&mutex_h);
    int32_t status = hput((hashtable_t *)lhtp, ep, key, keylen);
    pthread_mutex_unlock(&mutex_h);
    return status;
}

/* lhapply -- applies a function to every entry in hash table */
void lhapply(lhash_t *lhtp, void (*fn)(void *ep))
{
    pthread_mutex_lock(&mutex_h);
    happly((hashtable_t *)lhtp, fn);
    pthread_mutex_unlock(&mutex_h);
}

/* lhsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if not found
 */
void *lhsearch(lhash_t *lhtp, bool (*searchfn)(void *ep, const void *searchkeyp),
               const char *key, int keylen)
{
    pthread_mutex_lock(&mutex_h);
    void *entry = hsearch((hashtable_t *)lhtp, searchfn, key, keylen);
    pthread_mutex_unlock(&mutex_h);
    return entry;
}

/* lhremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *lhremove(lhash_t *lhtp, bool (*searchfn)(void *ep, const void *searchkeyp),
               const char *key, int keylen)
{
    pthread_mutex_lock(&mutex_h);
    void *data = hremove((hashtable_t *)lhtp, searchfn, key, keylen);
    pthread_mutex_unlock(&mutex_h);
    return data;
}
