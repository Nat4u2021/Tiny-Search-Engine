/*
 * pageio.c -- saving and loading crawler webpage files util
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: pagesave saves an existing webpage to a file with a
 * numbered name (e.g. 1,2,3 etc); pageload creates a new page by
 * loading a numbered file. For pagesave, the directory must exist and
 * be writable; for loadpage it must be readable.
 */

#include "pageio.h"
#include "webpage.h"

/*
 * pagesave -- save the page in filename id in directory dirnm
 *
 * returns: 0 for success; nonzero otherwise
 *
 * The suggested format for the file is:
 *   <url>
 *   <depth>
 *   <html-length>
 *   <html>
 */
int32_t pagesave(webpage_t *pagep, int id, char *dirnm)
{
    if (!pagep || !dirnm)
    {
        return -1;
    }

    char *url = webpage_getURL(pagep);
    int depth = webpage_getDepth(pagep);
    int len = webpage_getHTMLlen(pagep);
    char *html_content = webpage_getHTML(pagep);

    /* generate path */
    char path[1024];
    sprintf(path, "%s/%d", dirnm, id);

    /* open file */
    FILE *file = fopen(path, "w");
    if (file == NULL || access(path, W_OK) != 0)
    {
        printf("Failed to create file for url: %s\n", url);
        return 1;
    }

    /* write into the file */
    fprintf(file, "%s\n%d\n%d\n%s", url, depth, len, html_content);

    fclose(file);
    return 0;
}

/*
 * pageload -- loads the numbered filename <id> in directory <dirnm>
 * into a new webpage
 *
 * returns: non-NULL for success; NULL otherwise
 */
webpage_t *pageload(int id, char *dirnm)
{
    if (!dirnm)
        return NULL;

    char path[1024];
    sprintf(path, "%s/%d", dirnm, id);

    FILE *file = fopen(path, "r");
    if (file == NULL || access(path, R_OK) != 0)
    {
        return NULL;
    }

    /* scan url */
    char url[1024];
    if (fscanf(file, "%s\n", url) != 1)
    {
        printf("Failed to read URL from file id: %d\n", id);
        fclose(file);
        return NULL;
    }

    /* scan depth and html_length */
    int depth, html_length;
    if (fscanf(file, "%d\n", &depth) != 1)
    {
        printf("Failed to read depth from file id: %d\n", id);
        fclose(file);
        return NULL;
    }

    if (fscanf(file, "%d\n", &html_length) != 1)
    {
        printf("Failed to read character count from file id: %d\n", id);
        fclose(file);
        return NULL;
    }

    /* scan the html */
    char *html = malloc((html_length + 1) * sizeof(char));
    int idx = 0;
    char c;
    while ((c = fgetc(file)) != EOF && idx < html_length)
    {
        html[idx++] = c;
    }

    html[idx] = '\0';
    fclose(file);

    /* create webpage */
    webpage_t *page = webpage_new(url, depth, html);
    if (!page)
    {
        printf("Failed to initialize webpage\n");
        return NULL;
    }

    return page;
}
