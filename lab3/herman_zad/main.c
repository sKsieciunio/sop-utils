#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "utils.h"
#include "circular_buffer.h"

#define THREAD_COUNT 5

typedef struct
{
    pthread_t thread;
    CircularBuffer_t *cb;

} argsConsumer_t;

void scan_directory(const char *dir_path, CircularBuffer_t *cb);
void *consumer(void *arg);

int main(int argc, char const *argv[])
{
    CircularBuffer_t *cb = (CircularBuffer_t *)malloc(sizeof(CircularBuffer_t));
    cb_init(cb, 5);

    argsConsumer_t *args = (argsConsumer_t *)malloc(sizeof(argsConsumer_t));
    args->cb = cb;

    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        if (pthread_create(&threads[i], NULL, consumer, args) != 0)
            ERR("pthread_create");
    }

    scan_directory("./data1", cb);

    while(!cb_is_empty(cb))
        ;

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_cancel(threads[i]);
        if (pthread_join(threads[i], NULL) != 0)
            ERR("pthread_join");
    }

}

void *consumer(void *arg)
{
    argsConsumer_t *args = (argsConsumer_t *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1)
    {
        char *path = NULL;

        pthread_cleanup_push(free, path);

        cb_pop(args->cb, &path);
        if (path == NULL)
            ERR("cb_pop");

        FILE *file = fopen(path, "r");
        if (file == NULL)
            ERR("fopen");

        pthread_cleanup_push((void (*)(void *))fclose, file);

        int count[26] = {0};
        char c;
        while ((c = fgetc(file)) != EOF)
        {
            if (c >= 'a' && c <= 'z')
                count[c - 'a']++;
            else if (c >= 'A' && c <= 'Z')
                count[c - 'A']++;
        }

        printf("File: %s\n", path);
        for (int i = 0; i < 26; i++)
        {
            if (count[i] > 0)
                printf("%c: %d\n", 'a' + i, count[i]);
        }

        pthread_cleanup_pop(1);
        pthread_cleanup_pop(1);
    }

    return NULL;
}

void scan_directory(const char *dir_path, CircularBuffer_t *cb) {

    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if (dir == NULL)
        ERR("opendir");

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(path, &st) == -1)
            ERR("stat");

        if (S_ISDIR(st.st_mode))
            scan_directory(path, cb);
        else if (S_ISREG(st.st_mode))
            if (strstr(entry->d_name, ".txt") != NULL)
            {
                cb_push(cb, path);
                printf("Pushed %s\n", path);
            }
    }

    closedir(dir);  
}
