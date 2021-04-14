/**
* karel on 2021-04-13.
*/
#include "threaded_list.h"
#include <stdio.h>
#include <stdlib.h>

threaded_list_t * get_threaded_list() {
    threaded_list_t * new_list = (threaded_list_t * ) malloc(sizeof (threaded_list_t));
    if (!new_list) {
        perror("Could not alloc mem");
        exit(MALLOC_ERR);
    }
    pthread_mutex_init(&new_list->list_mx, NULL);
    new_list->list = dlinked_create_list();
    return new_list;
}

extended_thread_list_t * get_extended_threaded_list() {
    extended_thread_list_t * new_list = (extended_thread_list_t * ) malloc(sizeof (extended_thread_list_t ));
    if (!new_list) {
        perror("Could not alloc mem");
        exit(MALLOC_ERR);
    }
    pthread_mutex_init(&new_list->list_mx, NULL);
    new_list->list = dlinked_create_list();
    sem_init(&new_list->items_sem, 0, 0 );
    return new_list;
}
