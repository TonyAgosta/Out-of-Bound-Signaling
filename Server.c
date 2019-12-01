#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

#define UNIX_PATH_MAX 108
//#define SOCKNAME "./server"
#define N 100

static void run_server();
long aggiorna(fd_set set, long fdmax);
static void *stampamessaggio(void *arg);
int nomeserver;
int numpipe;
//char buf[4];

/**
 * @function readn
 * @brief usata per leggere buffer
 *
 * @param fd fd da cui leggere buffer
 * @param buf buffer da leggere
 * @param size dimensione del buffer da leggere
 *
 * @returns size se lettura avvenuta con successo di tutti i byte richiesti
 *          -1 se durante la lettura c'pe stato un errore
 *          0 se si è arrivati alla fine del file
 */
static inline int readn(long fd, void *buf, size_t size)
{
    size_t left = size;
    int r;
    char *bufptr = (char *)buf;
    int c = strlen(bufptr);

    while (left > 0)
    {

        if ((r = read((int)fd, bufptr, left)) == -1)
        {

            if (errno == EINTR)
                continue;
            return -1;
        }
        if (r == 0)
            return 0;
        left -= r;
        bufptr += r;
    }
    return size;
}

/**
 * @function writen
 * @brief usata per scrivere buffer
 *
 * @param fd fd in cui scrivere buffer
 * @param buf buffer da scrivere
 * @param size dimensione del buffer da scrivere
 *
 * @returns size se scrittura avvenuta con successo di tutti i byte richiesti
 *          -1 se durante la scrittura c'pe stato un errore
 *          0 se si è arrivati alla fine del file
 */
static inline int writen(long fd, void *buf, size_t size)
{
    size_t left = size;
    int r;
    char *bufptr = (char *)buf;
    while (left > 0)
    {

        if ((r = write((int)fd, bufptr, left)) == -1)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (r == 0)
            return 0;
        left -= r;
        bufptr += r;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Numero di argometi errato nel server");
        exit(EXIT_FAILURE);
    }
    //struct sockaddr_un sa;
    nomeserver = (int)(*argv[1]);
    numpipe = (int)(*argv[2]);
    run_server();
    return 0;
}

void eliminaSocket()
{
    char nomeSocket[20];
    sprintf(nomeSocket, "./OOB-server-%d", nomeserver);
    unlink(nomeSocket);
}

static void run_server()
{
    char nomesocket[16];
    char *ns;
    int fd_skt;
    struct sockaddr_un sa;

    strcpy(nomesocket, "OOB-server-");
    ns = malloc(1 * sizeof(int));
    sprintf(ns, "%d", nomeserver);
    strcat(nomesocket, ns);
    if (unlink(nomesocket) == -1)
    {
        perror("Unlink");
        exit(EXIT_FAILURE);
    }
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_skt == 1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    strncpy(sa.sun_path, nomesocket, UNIX_PATH_MAX);
    sa.sun_family = AF_UNIX;

    if (bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("Bind");
        exit(EXIT_FAILURE);
    }

    if (listen(fd_skt, SOMAXCONN) < 0)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    /*if (atexit(eliminaSocket) != 0)
    {
        perror("eliminaSocket");
    }*/
    while (1)
    {

        long fd1;
        pthread_t tid;
        errno = 0;
        if ((fd1 = accept(fd_skt, NULL, 0)) == -1)
        {
            if (errno == EAGAIN)
                continue;
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        printf("SERVER %d CONNECT FROM CLIENT\n", nomeserver);
        if ((pthread_create(&tid, NULL, stampamessaggio, (void *)fd1)) != 0)
        {
            perror("Thread non creato");
            exit(EXIT_FAILURE);
        }
        else if (pthread_detach(tid) != 0)
        {
            perror("detach");
            exit(EXIT_FAILURE);
        }
    }
}

static void *stampamessaggio(void *arg)
{

    long fd = (long)arg;
    int r;
    char *buffer = calloc(5, sizeof(char));
    while (read(fd, buffer, 5) > 0)
    {
        if (r < 0)
        {
            perror("read socket");
            pthread_exit(NULL);
        }
        printf("server %d got %s\n",nomeserver, buffer);
    }
    //close(fd);
    //free(arg);
    pthread_exit(NULL);
}
