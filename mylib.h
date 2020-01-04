//Tony Agosta 544090

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
#include <arpa/inet.h>

#define HTONLL(x) ((1 == htonl(1)) ? (x) : (((uint64_t)htonl((x)&0xFFFFFFFFUL)) << 32) | htonl((uint32_t)((x) >> 32)))
#define NTOHLL(x) ((1 == ntohl(1)) ? (x) : (((uint64_t)ntohl((x)&0xFFFFFFFFUL)) << 32) | ntohl((uint32_t)((x) >> 32)))

/*SUPERVISOR*/

typedef struct msg
{
    long stima;
    uint64_t client;
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

sig_atomic_t contsigin = 0;

//CLEANUP//
void closeall()
{
    int i = 0;

    while (i < k)
    {
        //mando un sigterm ai server per chiuderli in maniera adeguata
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

//stampo la lista di stime
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

//lista che contiene le stime ricevute
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
        //controllo se l'ID del client e` gia presente in lista
        int trovato = 0;
        messaggio *corr = l;
        while (corr != NULL && !trovato)
        {
            if (corr->client == e.client)
                trovato = 1;
            else
                corr = corr->next;
        }
        if (!trovato) //se non l'ho trovato allora faccio il normale inserimento in lista
        {
            messaggio *new1 = (messaggio *)malloc(sizeof(messaggio));
            new1->client = e.client;
            new1->stima = e.stima;
            new1->next = NULL;
            new1->server = 1;
            new1->next = l;
            l = new1;
        }
        else //altrimenti incremento il numero di server a cui si collega e prendo la stima migliore
        {
            corr->server++;
            if (corr->stima > e.stima)
                corr->stima = e.stima;
        }
    }
    return l;
}

/*SERVER*/

//stuttura che uso per creare un lista in cui salvo i fd dei trhead creati e per gestire il multithreading
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

static inline int readn(long fd, void *buf, size_t size)
{
    size_t left = size;
    int r;
    char *bufptr = (char *)buf;
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

/*CLIENT*/

int secret;
long idclient;
int p, k, w;
long *serverscelti;

long *servercasuali(int p);
struct timespec tempo; //setto il timespec per la nanosleep
long *server;

//scelgo casualmente i p server distisnti a cui collegarmi
long *servercasuali(int p)
{
    server = calloc(p, sizeof(long)); //alloco l'array con dimensione p
    int casuale, i = 0;
    int trovato = 0, j = 0;

    for (i = 0; i < p; i++)
    {
        casuale = rand() % k;
        while (!trovato && j < p)
        {
            if (server[i] == casuale)
                trovato = 1;
            j++;
        }
        if (!trovato)
        {
            server[i] = casuale + 1;
        }
        else
            i--;//recupero il ciclo perso
        j = 0;
        trovato = 0;
    }
    return server;
}

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


#endif
