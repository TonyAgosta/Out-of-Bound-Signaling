#ifndef mylib
#define mylib
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include<arpa/inet.h>

#define HTONLL(x) ((1==htonl(1)) ? (x) : (((uint64_t)htonl((x) & 0xFFFFFFFFUL)) << 32) | htonl((uint32_t)((x) >> 32)))
#define NTOHLL(x) ((1==ntohl(1)) ? (x) : (((uint64_t)ntohl((x) & 0xFFFFFFFFUL)) << 32) | ntohl((uint32_t)((x) >> 32)))

/*SUPERVISOR*/

typedef struct msg
{
    long stima;
    u_int64_t client;
    int server;
    struct msg *next;
} messaggio;

typedef struct descriptor
{
    int fd[2];

} pipee;

pid_t *pidfigli; //per salvare i pid dei figli creati
int k;
int sigalarm = 0;
pipee *pipeserver;
messaggio *l;
messaggio elemento;
messaggio *stimemigliori;

sig_atomic_t contsigin = 0;
sig_atomic_t contsigtstp = 0;

void closeall()
{
    int i = 0;

    while (i < k)
    {

        if (kill(pidfigli[i], SIGTERM) == -1)
            perror("kill");
        i++;
    }
    messaggio *corr = l;
    while (corr != NULL)
    {
        l = l->next;
        free(corr);
        corr = l;
    }
    free(pidfigli);
    free(pipeserver);
    exit(0);
}

void stampalista(messaggio *lista)
{
    if (lista == NULL)
        printf("Lista delle stime vuota\n");
    messaggio *corr = lista;

    while (corr != NULL)
    {
        printf("SUPERVISOR ESTIMATE %ld FOR %lx BASED ON %d\n", corr->stima, corr->client, corr->server);
        corr = corr->next;
    }
}



messaggio *listastima(messaggio *l, messaggio e)
{

    if (l == NULL)
    {
        messaggio *new1 = (messaggio *)malloc(sizeof(messaggio));
        new1->client = e.client;
        new1->stima = e.stima;
        new1->next = NULL;
        new1->server = 1;
        l = new1;
    }
    else
    {

        int trovato = 0;
        messaggio *corr = l;
        while (corr != NULL && !trovato)
        {
            if (corr->client == e.client)
                trovato = 1;
            else
                corr = corr->next;
        }
        if (!trovato)
        {
            messaggio *new1 = (messaggio *)malloc(sizeof(messaggio));
            new1->client = e.client;
            new1->stima = e.stima;
            new1->next = NULL;
            new1->server = 1;
            new1->next = l;
            l = new1;
        }
        else
        {
            corr->server++;
            if (corr->stima > e.stima)
                corr->stima = e.stima;
        }
    }
    return l;
}

/*SERVER*/

typedef struct listafd
{
    pthread_t t_id;
    int fd;
    struct listafd *next;
} listadescriptor;

listadescriptor *lista;
long aggiorna(fd_set set, long fdmax);

long nomeserver;
int numpipe;



/*CLIENT*/

int secret;
long idclient;
int p, k, w;
long *serverscelti;

long *servercasuali(int p);
struct timespec tempo;
long *server;



long *servercasuali(int p)
{
    server = calloc(p, sizeof(long)); //alloco l'array con dimensione p
    int casuale, i;

    for (i = 0; i < p; i++)
    {
        casuale = rand() % k;
        server[i] = casuale + 1;
    }

    return server;
}

#endif