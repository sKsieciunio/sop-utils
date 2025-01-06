#pragma once
#include <pthread.h>

typedef struct {
    char **buffer;
    size_t head;
    size_t tail;
    size_t max;
    int is_full;
    pthread_mutex_t lock;
} CircularBuffer_t;

int cb_is_full(CircularBuffer_t *cb);
int cb_is_empty(CircularBuffer_t *cb);
void cb_init(CircularBuffer_t *cb, size_t size);
void cb_deinit(CircularBuffer_t *cb);
void cb_push(CircularBuffer_t *cb, const char *path);
void cb_pop(CircularBuffer_t *cb, char **path);