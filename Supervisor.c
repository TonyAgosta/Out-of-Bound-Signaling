#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>

#define N 100

typedef struct msg
{
    long stima;
    long client;
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
int stampafine = 0; //se diventa 1 il supervisor deve chiudere
int stampa = 0;     //se diventa 1 il supervisor deve stampare la tabella delle stime
void server_run(int k);
pipee *pipeserver;
messaggio *l;
messaggio elemento;
messaggio *stimemigliori;

sig_atomic_t contsigin = 0;
sig_atomic_t contsigtstp = 0;

void closeall()
{
    int i = 1;

    while (i <= k)
    {

        //se errno=ESRCH vuol dire che il server è già stato chiuso.
        if (kill(pidfigli[i], SIGTERM) == -1)
            perror("kill");
        i++;
        //waitpid(pidfigli[i], NULL, WUNTRACED);
    }

    while (l != NULL){
        free(l);
        l=l->next;
    }
    free(pipeserver);
    exit(0);
}

void stampalista(messaggio *lista)
{
    messaggio *corr = lista;
    printf("stampalista\n");
    while (corr != NULL)
    {
        printf("SUPERVISOR ESTIMATE %ld FOR %ld BASED ON %d\n", corr->stima, corr->client, corr->server);
        corr = corr->next;
    }
}

static void gestoresigint(int signum)
{

    // printf("signit\n");
    if (signum == SIGINT)
    {
        contsigin++;

        if (contsigin == 1)
            alarm(1);
        if (contsigin == 2)
        {
            stampalista(l);
            printf("SUPERVISOR EXITING\n");
            closeall();
        }
    }
    if (signum == SIGALRM)
    {
        if (contsigin == 1)
        {
            contsigin = 0;
            stampalista(l);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("Inserire il numero di server da lanciare");
        exit(EXIT_FAILURE);
    }
    k = atoi(argv[1]); //numero di server da lanciare
    struct sigaction t;
    struct sigaction d;
    memset(&t, 0, sizeof(t));
    memset(&d, 0, sizeof(d));
    t.sa_handler = gestoresigint;
    //d.sa_handler = gestoresigalarm;
    if ((sigaction(SIGINT, &t, NULL)) != 0)
    {
        perror("Errore nella SIGINT\n");
        exit(EXIT_FAILURE);
    }
    if ((sigaction(SIGALRM, &t, NULL)) != 0)
    {
        perror("Errore nella SIGALARM\n");
        exit(EXIT_FAILURE);
    }
    /*if (atexit(closeall) != 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }*/
    printf("SUPERVISOR STARTING %d\n", k);
    server_run(k);
    return 0;
}
messaggio *listastima(messaggio *l, messaggio e)
{

    if (l == NULL)
    {
        messaggio *new = malloc(sizeof(messaggio));
        new->client = e.client;
        new->stima = e.stima;
        new->next = NULL;
        new->server = 1;
        l = new;
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
            messaggio *new = malloc(sizeof(messaggio));
            new->client = e.client;
            new->stima = e.stima;
            new->next = NULL;
            new->next = l;
            l = new;
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
void server_run(int k)
{
    int i;

    pidfigli = (pid_t *)calloc(k+1, sizeof(pid_t));
    pipeserver = (pipee *)calloc(k+1, sizeof(pipee));
    for (i = 1; i < k + 1; i++)
    {
        if (pipe(pipeserver[i].fd) == -1)
        {
            perror("Errore pipe");
            exit(EXIT_FAILURE);
        }
        //pipeserver[i].nameserver = i;
        if ((pidfigli[i] = fork()) == -1)
        {
            perror("Errore fork");
            exit(EXIT_FAILURE);
        }
        if (pidfigli[i] == 0) //sono nel figlio
        {
            close(pipeserver[i].fd[0]); //chiudo la pipe in lettura
            if ((execl("/home/tony/Desktop/Progetto_Sistemi/server.out", "server.out", &i, &pipeserver[i].fd[1], (char *)NULL)) == -1)
            {
                perror("Excl");
                exit(EXIT_FAILURE);
            }
        }
        else
        { //sono nel padre
            close(pipeserver[i].fd[1]);
            int x = fcntl(pipeserver[i].fd[0], F_GETFL, 0);
            fcntl(pipeserver[i].fd[0], F_SETFL, x | O_NONBLOCK);
        }
        //free(pipeserver);
    }
    srand(time(NULL));
    int numeroserver = rand() % (k+1) + 1;
    int s=-1;
    while (1)
    {
        errno = 0;
        numeroserver = rand() % (k+1) + 1;
        s = read(pipeserver[numeroserver].fd[0], &elemento, sizeof(messaggio));
        if (s > 0)
        {
            //printf("elemento.stima: %ld, elemento.client:%ld, elemento.server:%d\n", elemento.stima, elemento.client, elemento.server);
            l = listastima(l, elemento);
        }
        if (s <= 0)
        {
            if (errno == 11)
                continue;
        }
    }
}
