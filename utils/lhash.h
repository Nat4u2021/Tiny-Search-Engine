#pragma once
/* lhash.h --- locked hashtable header file
 *
 *
 * Author: Nathaniel Mensah
 * Created: Sat Feb 25 22:30:54 2023 (-0500)
 * Version: 1.0
 *
 * Description: prototypes for locked hashtable functions
 *
 */
#include <stdint.h>
#include <stdbool.h>

typedef void lhash_t; /* representation of a locked hashtable hidden */

/* lhopen -- opens a hash table with initial size hsize */
lhash_t *lhopen(uint32_t hsize);

/* lhclose -- closes a hash table */
void lhclose(lhash_t *lhtp);

/* lhput -- puts an entry into a hash table under designated key
 * returns 0 for success; non-zero otherwise
 */
int32_t lhput(lhash_t *lhtp, void *ep, const char *key, int keylen);

/* lhapply -- applies a function to every entry in hash table */
void lhapply(lhash_t *lhtp, void (*fn)(void *ep));

/* lhsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 */
void *lhsearch(lhash_t *lhtp,
			   bool (*searchfn)(void *elementp, const void *searchkeyp),
			   const char *key,
			   int32_t keylen);

/* lhremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *lhremove(lhash_t *lhtp,
			   bool (*searchfn)(void *elementp, const void *searchkeyp),
			   const char *key,
			   int32_t keylen);
