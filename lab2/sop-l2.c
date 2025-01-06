#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal;

ssize_t bulk_read(int fd, char* buf, size_t count)
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

ssize_t bulk_write(int fd, char* buf, size_t count)
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

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}

void usage(int argc, char* argv[])
{
    printf("%s n f \n", argv[0]);
    printf("\tf - file to be processed\n");
    printf("\t0 < n < 10 - number of child processes\n");
    exit(EXIT_FAILURE);
}

void custom_sleep(unsigned int nanoseconds)
{
    struct timespec t = {0, nanoseconds};

    while (nanosleep(&t, &t) == -1)
        if (errno != EINTR)
            ERR("custom_sleep");
}

void sig_handler(int sig)
{
    last_signal = sig;
}

void sigchld_handler(int sig)
{
    pid_t pid;
    for (;;)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if (pid == 0)
            return;
        if (pid <= 0)
        {
            if (errno == ECHILD)
                return;
            ERR("waitpid");
        }
    }
}

void child_work(char *text_buf, ssize_t text_len, char *f, int i)
{
    while(last_signal != SIGUSR1)
        ;

    struct timespec t = {0, 250 * 1000 * 1000};
    struct timespec rem;

    char file_name[50];
    sprintf(file_name, "%s-%d", f, i);

    int fd;
    if((fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0777)) < 0)
        ERR("open");

    for (int i = 0; i < text_len; i++)
    {
        if (text_buf[i] >= 'a' && text_buf[i] <= 'z' && i % 2 == 0)
            text_buf[i] = text_buf[i] - 32;
        else if (text_buf[i] >= 'A' && text_buf[i] <= 'Z' && i % 2 == 0)
            text_buf[i] = text_buf[i] + 32;

        while (nanosleep(&t, &rem) == -1)
        {
            if (errno == EINTR)
                t = rem;
            else if (last_signal == SIGINT)
            {
                write(fd, text_buf + i, 1) ;
                free(text_buf);
                close(fd);
                exit(EXIT_FAILURE);
            }
            else
                ERR("nanosleep");
        }

        if (write(fd, text_buf + i, 1) < 0)
            ERR("write");
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3 )
        usage(argc, argv);

    int n = atoi(argv[1]);
    char *f = argv[2];
    
    if (n <= 0 || n >= 10)
        usage(argc, argv);

    int fd;
    if((fd = open(f, O_RDONLY)) < 0)
        ERR("open");

    size_t len;
    if ((len = lseek(fd, 0, SEEK_END)) < 0)
        ERR("lseek");
    lseek(fd, 0, SEEK_SET);

    char *text_buf = (char *)malloc(len);
    ssize_t text_len;

    sethandler(sig_handler, SIGUSR1);
    sethandler(sig_handler, SIGINT);
    sethandler(sigchld_handler, SIGCHLD);

    pid_t pid;
    for (int i = 0; i < n; i++)
    {
        pid = fork();
        if (pid < 0)
            ERR("fork");
        if (pid == 0)
        {
            if ((text_len = read(fd, text_buf, len / n + 1)) < 0)
                ERR("read");
            
            if (close(fd) < 0)
                ERR("close");

            child_work(text_buf, text_len, f, i+1);
            free(text_buf);
            return EXIT_SUCCESS;
            break;
        }
    }

    if (pid > 0)
    {
        printf("Parent PID: %d\n", getpid());

        if (kill(0, SIGUSR1))
            ERR("kill");
    }

    close(fd);
    free(text_buf);

    while (wait(NULL) > 0)
        ;

    return EXIT_SUCCESS;
}