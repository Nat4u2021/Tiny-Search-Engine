/*
 * pageio_test.c -- tests the pageio module
 *
 * Author: Nathaniel Mensah
 * Version: 1.0
 *
 * Description: tests the pagesave() and pageload() functions
 * of the pageio utils
 */

#include <stdio.h>
#include "pageio.h"
#include "webpage.h"

int main(void)
{
    char *dirname = "./";
    int id = 1;
    webpage_t *page = pageload(id, dirname);

    if (!page)
    {
        printf("Failed to load page id: %d\n", id);
        return 1;
    }
    if (pagesave(page, ++id, dirname) != 0)
    {
        printf("Failed to save page.\n");
        return 1;
    }

    webpage_t *page_copy = pageload(id, dirname);
    if (!page_copy)
    {
        printf("Failed to load page id: %d\n", id);
        return 1;
    }

    if (webpage_getDepth(page) != webpage_getDepth(page_copy))
        return 1;
    if (webpage_getHTMLlen(page) != webpage_getHTMLlen(page_copy))
        return 1;
    if (strcmp(webpage_getURL(page), webpage_getURL(page_copy)) != 0)
        return 1;
    if (strcmp(webpage_getHTML(page), webpage_getHTML(page_copy)) != 0)
        return 1;

    webpage_delete(page);
    webpage_delete(page_copy);
    printf("Saved and loaded page successfully.\n");
    return 0;
}
