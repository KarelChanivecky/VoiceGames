/**
* karel on 2021-04-10.
*/

#ifndef GAME_SERVER_THREADED_LIST_DEF_H
#define GAME_SERVER_THREADED_LIST_DEF_H

#include "dc_utils/dlinked_list.h"
#include "pthread.h"
#include "semaphore.h"

typedef struct {
    pthread_mutex_t list_mx;
    dlinked_list * list;
} threaded_list_t;

typedef struct {
    pthread_mutex_t list_mx;
    dlinked_list * list;
    sem_t items_sem;
} extended_thread_list_t;

typedef struct {
    pthread_mutex_t mx;
    size_t size;
    void * arr[];
} threaded_array_t;

threaded_list_t * get_threaded_list();
extended_thread_list_t * get_extended_threaded_list();

#endif //GAME_SERVER_THREADED_LIST_DEF_H
