#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "circular_buffer.h"

#define THREAD_COUNT 5
#define BUFFER_SIZE 5

typedef struct
{
    pthread_t thread;
    CircularBuffer_t *cb;
    int *pTotalFiles;
    int *pProcessedFiles;
    pthread_mutex_t *pmxTotal;
    pthread_mutex_t *pmxProcessed;
    bool *endFlag;
    pthread_mutex_t *pmxEndFlag;
} argsConsumer_t;

void scan_directory(const char *dir_path, CircularBuffer_t *cb, int *totalFiles, pthread_mutex_t *mxTotal);
void *consumer(void *arg);

int main(int argc, char const *argv[])
{
    CircularBuffer_t *cb = (CircularBuffer_t *)malloc(sizeof(CircularBuffer_t) * THREAD_COUNT);
    cb_init(cb, BUFFER_SIZE);

    int totalFiles = 0, processedFiles = 0;
    pthread_mutex_t mxTotal = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxProcessed = PTHREAD_MUTEX_INITIALIZER;

    bool endFlag = false;
    pthread_mutex_t mxEndFlag = PTHREAD_MUTEX_INITIALIZER;

    argsConsumer_t *args = (argsConsumer_t *)malloc(sizeof(argsConsumer_t) * THREAD_COUNT);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        args[i].cb = cb;
        args[i].pTotalFiles = &totalFiles;
        args[i].pProcessedFiles = &processedFiles;
        args[i].pmxTotal = &mxTotal; 
        args[i].pmxProcessed = &mxProcessed;
        args[i].endFlag = &endFlag;
        args[i].pmxEndFlag = &mxEndFlag;
    }

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        if (pthread_create(&args[i].thread, NULL, consumer, &args[i]) != 0)
            ERR("pthread_create");
    }

    scan_directory("./data1", cb, &totalFiles, &mxTotal);

    while (1)
    {
        pthread_mutex_lock(&mxProcessed);
        if (processedFiles == totalFiles)
        {
            pthread_mutex_unlock(&mxProcessed);
            pthread_mutex_lock(&mxEndFlag);
            endFlag = true;
            pthread_mutex_unlock(&mxEndFlag);
            printf("End flag set\n");
            break;
        }
        pthread_mutex_unlock(&mxProcessed);
        usleep(5000);
    }

    printf("All files processed\n");

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        if (pthread_join(args[i].thread, NULL) != 0)
            ERR("pthread_join");
        printf("Thread %d joined\n", i);
    }

}

void *consumer(void *arg)
{
    argsConsumer_t *args = (argsConsumer_t *)arg;

    while (1)
    {
        pthread_mutex_lock(args->pmxEndFlag);
        bool endFlag = *(args->endFlag);
        pthread_mutex_unlock(args->pmxEndFlag);

        printf("End flag: %d\n", endFlag);
        printf("Buffer empty: %d\n", cb_is_empty(args->cb));
        if (endFlag && cb_is_empty(args->cb))
            break;

        char *path = NULL;
        cb_pop(args->cb, &path);
        if (path == NULL)
            ERR("cb_pop");

        FILE *file = fopen(path, "r");
        if (file == NULL)
            ERR("fopen");

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

        pthread_mutex_lock(args->pmxProcessed);
        (*args->pProcessedFiles)++;
        pthread_mutex_unlock(args->pmxProcessed);

        fclose(file);
        free(path);
    }

    return NULL;
}

void scan_directory(const char *dir_path, CircularBuffer_t *cb, int *totalFiles, pthread_mutex_t *mxTotal) {

    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if (dir == NULL)
        ERR("opendir");

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[256];
        if (snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name) >= sizeof(path))
            ERR("snprintf");

        struct stat st;
        if (stat(path, &st) == -1)
            ERR("stat");

        if (S_ISDIR(st.st_mode))
            scan_directory(path, cb, totalFiles, mxTotal);
        else if (S_ISREG(st.st_mode))
            if (strstr(entry->d_name, ".txt") != NULL)
            {
                cb_push(cb, path);
                pthread_mutex_lock(mxTotal);
                (*totalFiles)++;
                pthread_mutex_unlock(mxTotal);
                printf("Pushed %s\n", path);
            }
    }

    closedir(dir);  
}
