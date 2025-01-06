#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "circular_buffer.h"

typedef struct timespec timespec_t;

int cb_is_full(CircularBuffer_t *cb)
{
    return cb->is_full;
}

int cb_is_empty(CircularBuffer_t *cb)
{
    return cb->head == cb->tail && !cb->is_full;
}

void cb_init(CircularBuffer_t *cb, size_t size)
{
    if (NULL == (cb->buffer = (char **)malloc(size * sizeof(char *))))
        return;

    for (size_t i = 0; i < size; i++)
        cb->buffer[i] = NULL;

    cb->max = size;
    cb->head = 0;
    cb->tail = 0;
    cb->is_full = 0;
    pthread_mutex_init(&cb->lock, NULL);
}

void cb_deinit(CircularBuffer_t *cb)
{
    for (size_t i = 0; i < cb->max; i++)
        free(cb->buffer[i]);

    free(cb->buffer);
    cb->buffer = NULL;
    cb->max = 0;
    cb->head = 0;
    cb->tail = 0;
    cb->is_full = 0;
    pthread_mutex_destroy(&cb->lock);
}

void cb_push(CircularBuffer_t *cb, const char *path)
{
    while (1)
    {
        pthread_mutex_lock(&cb->lock);

        if (!cb->is_full)
        {
            cb->buffer[cb->head] = strdup(path);
            if (cb->buffer[cb->head] == NULL)
            {
                pthread_mutex_unlock(&cb->lock);
                // ERR("strdup");
                return;
            }

            cb->head = (cb->head + 1) % cb->max;

            if (cb->head == cb->tail)
                cb->is_full = 1;

            pthread_mutex_unlock(&cb->lock);
            break;
        }

        pthread_mutex_unlock(&cb->lock);
        msleep(5);
    }
}

void cb_pop(CircularBuffer_t *cb, char **path)
{
    while (1)
    {
        pthread_mutex_lock(&cb->lock);

        if (!cb_is_empty(cb))
        {
            *path = strdup(cb->buffer[cb->tail]);
            if (*path == NULL)
            {
                pthread_mutex_unlock(&cb->lock);
                // ERR("strdup");
                return;
            }

            free(cb->buffer[cb->tail]);
            cb->buffer[cb->tail] = NULL;

            cb->tail = (cb->tail + 1) % cb->max;
            cb->is_full = 0;

            pthread_mutex_unlock(&cb->lock);
            break;
        }

        pthread_mutex_unlock(&cb->lock);
        msleep(5);
    }
}