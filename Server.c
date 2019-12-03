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

typedef struct msg
{
    int stima;
    long client;
    int server;
    struct msg *next;
} messaggio;

typedef struct listafd
{
    pthread_t t_id;
    int fd;
    struct listafd *next;
} listadescriptor;

listadescriptor *lista;
static void run_server();
long aggiorna(fd_set set, long fdmax);
static void *stampamessaggio(void *arg);
int nomeserver;
int numpipe;
//char buf[4];

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Numero di argometi errato nel server");
        exit(EXIT_FAILURE);
    }

    nomeserver = (int)(*argv[1])+1;
    numpipe = (int)(*argv[2]);
    struct sigaction t;
    memset(&t, 0, sizeof(t));
    t.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &t, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    run_server();
    return 0;
}

void eliminaSocket()
{

    listadescriptor *corr = lista;
    while (corr != NULL)
    {
        pthread_join(corr->t_id, (void *)NULL);
        close(corr->t_id);
        corr = corr->next;
    }
    free(lista);
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
    if (atexit(eliminaSocket) != 0)
    {
        perror("eliminaSocket");
    }

    lista = NULL;
    while (1)
    {

        long fd1;
        errno = 0;
        if ((fd1 = accept(fd_skt, NULL, 0)) == -1)
        {
            if (errno == EAGAIN)
                continue;
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        printf("SERVER %d CONNECT FROM CLIENT\n", nomeserver);
        listadescriptor *new = malloc(sizeof(listadescriptor));
        new->fd = fd1;
        if ((pthread_create(&new->t_id, NULL, stampamessaggio, (void *)new)) != 0)
        {
            perror("Thread non creato");
            exit(EXIT_FAILURE);
        }
        if (lista == NULL)
        {

            new->next = NULL;
            lista = new;
        }
        else
        {
            new->next = lista;
            lista = new;
        }
    }
}

static void *stampamessaggio(void *arg)
{

    listadescriptor *fdlista = (listadescriptor *)arg;
    int bprimo = 0; //ho già letto il primo messaggio?
    long buffer;
    long stima = -1; //data dalla differenza di due messaggi successivis
    long stima1 = -1;
    struct timespec tempo;
    long tempo_prec, ns;
    messaggio m;
    stima = -1;

    while (read(fdlista->fd, &buffer, sizeof(long)) > 0)
    {
        clock_gettime(CLOCK_REALTIME, &tempo);
        ns = ((tempo.tv_sec % 10000) * 1000) + (tempo.tv_nsec / 1000000); //e` il tempo attuale
        printf("SERVER %d INCOMING FROM %ld @ %ld\n", nomeserver, buffer, ns);

        if (bprimo == 1)
        {
            stima = ns - tempo_prec;
            tempo_prec = ns;
            if (stima1 == -1)
                stima1 = stima;
            else if (stima1 > stima)
                stima1 = stima;
        }
        else
        {
            bprimo = 1;
            tempo_prec = ns;
        }
    }
    if (stima1 == -1)
        pthread_exit(NULL);
    m.client = buffer;
    m.server = 0;
    m.stima = stima1 % 10000;

    printf("SERVER %d CLOSING %ld ESTIMATE %d\n", nomeserver, buffer, m.stima);
    int w = write(numpipe, &m, sizeof(messaggio));
    if (w <= 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    /*else{
        printf("ho scritto\n");
    }*/
    close(lista->fd);
    pthread_exit(NULL);
}
