#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 4096
#define DEFAUTL_THREADCOUNT 10
#define DEFAULT_SAMPLESIZE 100

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef unsigned int UINT;
typedef struct argsEstimation
{
    pthread_t tid;
    UINT seed;
    int samplesCount;
} argsEstimation_t;

void ReadArguments(int argc, char **argv, int *threadCount, int *sampleCount);
void *pi_estimation(void *args);

int main(int argc, char **argv)
{
    int threadCount, samplesCount;
    double *subresult;
    ReadArguments(argc, argv, &threadCount, &samplesCount);
    
    argsEstimation_t *estimations = (argsEstimation_t *)malloc(sizeof(argsEstimation_t) * threadCount);
    if (estimations == NULL)
        ERR("Malloc error for estimation arguments!");

    srand(time(NULL));

    for (int i = 0; i < threadCount; i++)
    {
        estimations[i].seed = rand();
        estimations[i].samplesCount = samplesCount;
    }

    for (int i = 0; i < threadCount; i++)
    {
        int err = pthread_create(&(estimations[i].tid), NULL, pi_estimation, &estimations[i]);
        if (err != 0)
            ERR("Couldn't create thread");
    }

    double cumulativeResult = 0.0;

    for (int i = 0; i < threadCount; i++)
    {
        int err = pthread_join(estimations[i].tid, (void *)&subresult);
        if (err != 0)
            ERR("Can't join with a thread");

        if (subresult != NULL)
        {
            cumulativeResult += *subresult;
            free(subresult);
        }
    }

    double result = cumulativeResult / threadCount;
    printf("PI ~= %f\n", result);

    free(estimations);
}

void ReadArguments(int argc, char **argv, int *threadCount, int *sampleCount)
{
    *threadCount = DEFAUTL_THREADCOUNT;
    *sampleCount = DEFAULT_SAMPLESIZE;
    
    if (argc >= 2)
    {
        *threadCount = atoi(argv[1]);
        if (*threadCount <= 0)
        {
            printf("Invalid value for threadCount\n");
            exit(EXIT_FAILURE);
        }
    }

    if (argc >= 3)
    {
        *sampleCount = atoi(argv[2]);
        if (*sampleCount <= 0)
        {
            printf("Invalid value for samplesCount\n");
            exit(EXIT_FAILURE);
        }
    }
}

void *pi_estimation(void *voidPtr)
{
    argsEstimation_t *args = voidPtr;
    double *result;

    if (NULL == (result = malloc(sizeof(double))))
        ERR("malloc");

    int insideCount = 0;
    for (int i = 0; i < args->samplesCount; i++)
    {
        double x = ((double)rand_r(&args->seed) / (double)RAND_MAX);
        double y = ((double)rand_r(&args->seed) / (double)RAND_MAX);

        if (sqrt(x * x + y * y) <= 1.0)
            insideCount++;
    }

    *result = 4.0 * (double)insideCount / (double)args->samplesCount;
    return result;
}
