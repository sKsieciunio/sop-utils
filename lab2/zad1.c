#define _GNU_SOURCE
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t signal_count = 0;

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}
 
void sig_handler(int sig)
{
    signal_count++;
}

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;  // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void child_work(int n)
{
    int fd, s, block_count = 0;
    char file_name[64];
    ssize_t count;

    srand(time(NULL) * getpid());
    s = ((rand() % 90) + 10) * 1024 / 8;

    char *buf = (char *)malloc(s);
    if (!buf)
        ERR("malloc");
    memset(buf, '0' + n, s);

    sprintf(file_name, "%d.txt", getpid());

    if ((fd = TEMP_FAILURE_RETRY(open(file_name, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0777))) < 0)
        ERR("open");

    if ((count = bulk_write(fd, buf, s)) < 0)
        ERR("bulk_read");

    if (TEMP_FAILURE_RETRY(close(fd)))
        ERR("close");

    free(buf);
    exit(EXIT_SUCCESS);
}

int main (int argc, char **argv)
{
    if (argc < 2)
        ERR("Usage");

    sethandler(sig_handler, SIGUSR1);

    pid_t pid;
    for (int i = argc - 1; i > 0; i--)
    {
        if ((pid = fork()) < 0)
            ERR("fork");
        if (pid == 0)
            child_work(atoi(argv[i]));
    }

    sethandler(SIG_IGN, SIGUSR1);
    struct timespec t = {0, 10 * 1000};
    for (int i = 0; i < 1000; i += 10)
    {
        nanosleep(&t, NULL);
        kill(0, SIGUSR1);
    }

    while (wait(NULL) > 0)
        ;
}