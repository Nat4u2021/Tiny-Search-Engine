/* crawler.c --- web crawler
 * 
 * 
 * Authors: Nathaniel Mensah
 * Created: Tue Jan 31 23:57:45 2023 (-0500)
 * Version: 1.0
 * 
 * Description: crawls a website extracting embedded urls
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <webpage.h>
#include <lqueue.h>
#include <lhash.h>
#include <pageio.h>
#include <pthread.h>

#define hsize 1000    // hashtable size

static void crawl(int thread_id);
static bool searchfn(void* elementp, const void* searchkeyp);
static void* thread_start(void *arg);

lqueue_t *qp;
lhash_t *hp;
char *seed_url, *dirname;
int max_depth, pages_added=1, pages_retrieved=0, id=1;
pthread_mutex_t crawl_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
    if (argc != 4) {
        printf("Usage: crawler <seedurl> <pagedir> <maxdepth>\n");
        exit(EXIT_FAILURE);
    }

    seed_url = malloc(sizeof(char) * strlen(argv[1]) + 1);
    strcpy(seed_url, argv[1]);
    dirname = argv[2];
    max_depth = atoi(argv[3]);
    if(!seed_url || !dirname){
        printf("Error: invalid seed_url or save directory.\n");
        exit(EXIT_FAILURE);
    }
    if(max_depth < 0){
        printf("Error: Max_depth must be 0 or greater.\n");
        exit(EXIT_FAILURE);
    }
    int num_threads= 3;

    /*************************** SAVE SEED PAGE ***************************/
    /* check save directory */
    struct stat st;
    if (stat(dirname, &st) != 0 || !S_ISDIR(st.st_mode)){
        if ((mkdir(dirname, 0755))!= 0){
            printf("Failed to create save directory: %s\n", dirname);
            exit(EXIT_FAILURE);
        }
    }

    /* initialize seed_page */
    webpage_t *seed_page;
    if(!(seed_page=webpage_new(seed_url,0,NULL))){
        printf("Error! Failed to initialize webpage.");
        exit(EXIT_FAILURE);
    }

    /* fetch html */
    if(!webpage_fetch(seed_page)) {
        printf("Error! Failed to fetch html.");
        exit(EXIT_FAILURE);
    }
    
    qp = lqopen();
    hp = lhopen(hsize);
    lqput(qp,seed_page);
    lhput(hp,seed_url,seed_url,strlen(seed_url));
    pagesave(seed_page,id++,dirname);
    /**********************************************************************/

    /******************************** THREADS *****************************/
    pthread_t threads[num_threads];
    for(int i = 0; i < num_threads; i++) {
        if(pthread_create(&threads[i], NULL, thread_start, (void*)(intptr_t)i)) {
            printf("Error creating thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    
    for(int i = 0; i < num_threads; i++) {
        if(pthread_join(threads[i], NULL)) {
            printf("Error joining thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    /**********************************************************************/

    lhclose(hp);
    lqclose(qp);
    pthread_mutex_destroy(&crawl_mutex);
    exit(EXIT_SUCCESS);
}

static void crawl(int thread_id){
    int pos = 0, depth = 0;
    webpage_t *page, *curr;
    char *url;
    int status;
    //printf("id: %d entry\n", thread_id);

    /* BFS */
    while((curr=(webpage_t*)lqget(qp)) || (pages_retrieved<pages_added)){
        if(!curr) {
            continue;
        }
        pos = 0, depth = 0;
        depth = webpage_getDepth(curr);

        /* crawl page and retrieve all urls */
        while (depth<max_depth && (pos = webpage_getNextURL(curr, pos, &url)) > 0) {
            printf("Thread %d Found url: %s ", thread_id, url);
            
            if(IsInternalURL(url)) {
                printf("[internal]\n");
                pthread_mutex_lock(&crawl_mutex); /*!******!*/
                if (lhsearch(hp, searchfn, url, strlen(url)) == NULL){
                    if(!(page=webpage_new(url,depth+1,NULL))) {
                        printf("Error! Failed to initialize internal webpage.\n");
                        exit(EXIT_FAILURE);
                    }

                    if(!webpage_fetch(page)) {
                        printf("Error! Failed to fetch html from internal page.\n");
                        pthread_mutex_unlock(&crawl_mutex);
                        free(url);
                        webpage_delete(page);
                        continue;
                    }
                    
                    lqput(qp, page);
                    pages_added++;
                    lhput(hp,url,url,strlen(url));
                    status = pagesave(page, id++, dirname);
                    pthread_mutex_unlock(&crawl_mutex); /*!******!*/
                    if (status!=0){
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                    pthread_mutex_unlock(&crawl_mutex);
                    printf("[url: %s already in queue]\n",url);
                    free(url);
                }
            }
            else{
                printf("[external]\n");
                free(url);
            }
        }
        pthread_mutex_lock(&crawl_mutex);
        pages_retrieved++;
        pthread_mutex_unlock(&crawl_mutex);
        webpage_delete(curr);
    }
    //printf("id: %d exit\n", thread_id);
    //printf("added: %d, retrieved: %d\n",pages_added, pages_retrieved);
}

static bool searchfn(void* elementp, const void* searchkeyp){
    char *p = (char*)elementp;
    return strcmp(p,(char*)searchkeyp) == 0;
}

static void *thread_start(void *arg) {
    int thread_id = (intptr_t)arg;
    crawl(thread_id);
    return NULL;
}

/*
* use case: crawler https://thayer.github.io/engs50/ ../pages 2
* 0 - 1
* 1 - 7
* 2 - 42
* 3 - 82
*/
