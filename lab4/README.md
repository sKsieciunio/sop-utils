# Lab4 SOP notes

## Obsługa sygnału oddzielnym wątkiem z sigwaitem
```c
int main() 
{
    pthread_t signal_thread;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    if (pthread_sigmask(SIG_BLOCK, &set, NULL))
        ERR("pthread_sigmask");

    if (pthread_create(&signal_thread, NULL, signal_handler_thread, (void *)&set))
        ERR("pthread_create");

    //...

    pthread_join(signal_thread, NULL);
}
```
```c
void *signal_handler_thread(void *_arg)
{
    sigset_t *set = (sigset_t *)_arg;
    int sig;

    while (1)
    {
        if (sigwait(set, &sig) == 0)
        {
            // obsługa sygnału 
            // if (sig == SIGUSR1) { do something }
        }
        else
            ERR("sigwait");
    }

    return NULL;
}
```

## Tworzenie `n` wątków
```c
typedef struct
{
    pthread_t tid;
    // some data
    int *condition_met;
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
} thread_arg_t;
```
```c
int main()
{
    thread_arg_t *targs = (thread_arg_t *)malloc(n * sizeof(thread_arg_t));
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    for (int i = 0; i < n; i++)
    {
        // thread arguments initialization
    }

    for (int i = 0; i < n; i++)
    {
        if (pthread_create(&(targs[i].tid), NULL, thread_work, &targs[i]))
            ERR("pthread_create");
    }

    // ...

    for (int i = 0; i < n; i++)
    {
        if (pthread_join(targs[i].tid, NULL))
            ERR("pthread_join");
    }

    free(targs);
}
```
```c
void *thread_work(void *_arg)
{
    thread_arg_t *arg = (thread_arg_t *)_arg;

    // do work
}
```

## Wzorce użycia conditional variables
Przykład z użyciem `pthread_cond_timedwait()`
```c
int main()
{
    pthread_t tid;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int contition_met = 0;

    pthread_create(&tid, NULL, wait_thread, /* args with cond and mutex etc.*/);

    sleep(1);
    pthread_mutex_lock(&mutex);
    condition_met = 1;
    pthread_cond_signal(&cond); // or pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}
```
```c
void *wait_thread(void *_arg)
{
    // arg converting

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    pthread_mutex_lock(arg->mutex);
    while (!(*(arg->condition_met)))
    {
        int rc = pthread_cond_wait(arg->cond, arg->mutex, &ts);

        if (rc == ETIMEOUT)
        {
            // time passed
            break;
        }

        if (rc != 0)
            ERR("pthread_cond_wait");
    }
    pthread_mutex_unlock(arg->mutex);
    
    // ...
}
```

## Użycie Semaforów 
by chatGPT
```c
#define NUM_THREADS 5

sem_t semaphore; // Semaphore

void *worker(void *arg) {
    int id = *(int *)arg;
    free(arg);

    printf("Thread %d: Waiting for semaphore...\n", id);

    sem_wait(&semaphore); // Decrement semaphore
    printf("Thread %d: Acquired semaphore.\n", id);

    sleep(2); // Simulate work

    printf("Thread %d: Releasing semaphore.\n", id);
    sem_post(&semaphore); // Increment semaphore

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Initialize the semaphore with a maximum count of 2
    if (sem_init(&semaphore, 0, 2) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&threads[i], NULL, worker, arg) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy the semaphore
    sem_destroy(&semaphore);

    printf("All threads finished.\n");
    return 0;
}

```

## Użycie barier
by chatGPT
```c
#define NUM_THREADS 5

pthread_barrier_t barrier;

void *worker(void *arg) {
    int id = *(int *)arg;
    free(arg);

    printf("Thread %d: Doing some work...\n", id);
    sleep(1);  // Simulate some work

    // Wait at the barrier
    printf("Thread %d: Reached the barrier...\n", id);
    pthread_barrier_wait(&barrier);

    printf("Thread %d: Proceeding after barrier.\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Initialize the barrier with NUM_THREADS threads
    if (pthread_barrier_init(&barrier, NULL, NUM_THREADS) != 0) {
        perror("pthread_barrier_init");
        exit(EXIT_FAILURE);
    }

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&threads[i], NULL, worker, arg) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy the barrier
    pthread_barrier_destroy(&barrier);

    printf("All threads finished.\n");
    return 0;
}

```