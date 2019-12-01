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

#define N 100

typedef struct descriptor
{
    int fd[2];
    int nameserver;

} pipee;

typedef struct strutturastinghe
{
    int s;
    int id;
    int i;
} struttura;

int *pidfigli; //per salvare i pid dei figli creati
int k;
int sigalarm = 0;
int stampafine = 0; //se diventa 1 il supervisor deve chiudere
int stampa = 0;     //se diventa 1 il supervisor deve stampare la tabella delle stime
pipee *pipeserver;
struttura *str;
sig_atomic_t contsigin = 0;
sig_atomic_t contsigtstp = 0;

struttura *tokenizer(struttura *str, int len, const char delim[2], char *b, int m, int num)
{

    char *str1 = NULL;
    char *token1 = NULL;

    token1 = strtok(b, delim);
    str[m].s = strtol(token1, NULL, 10);
    str[m].i = num;
    if ((token1 = strtok(b, delim)) == NULL)
    {
        perror("Errore strtok");
        exit(EXIT_FAILURE);
    }
    str[m].id = strtol(token1, NULL, 10);
    return str;
}

static void gestoresigint(int signum)
{

    if (sigalarm == 1)
    {
        stampafine = 1;
        return;
        
    }
    //dopo aver controllato che  alarm non e` stato gia inviato posso settare i vari flag
    stampa = 1;
    sigalarm = 1;
    alarm(1);
}

static void gestoresigalarm(int signum)
{
    //se sono qui e` passato piu di un secondo
    sigalarm = 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("Inserire il numero di server da lanciare");
        exit(EXIT_FAILURE);
    }
    k = atoi(argv[1]); //numero di server da lanciare
    /****************************************************************************/
    //in questo tratto gestisco i segnali//
    struct sigaction t;
    struct sigaction d;
    memset(&t, 0, sizeof(t));
    memset(&d, 0, sizeof(d));
    t.sa_handler = gestoresigint;
    d.sa_handler = gestoresigalarm;
    if ((sigaction(SIGINT, &t, NULL)) != 0)
    {
        perror("Errore nella SIGINT\n");
        exit(EXIT_FAILURE);
    }
    if ((sigaction(SIGALRM, &d, NULL)) != 0)
    {
        perror("Errore nella SIGALARM\n");
        exit(EXIT_FAILURE);
    }
    /***************************************************************************/
    
    /***************************************************************************/
    //in questo tratto gestisco la connesione supervisor-server//
    int i;
    char namesocket[10];
    char numsocket[10];
    pidfigli = calloc(k, sizeof(int));
    pipeserver = calloc(k, sizeof(pipee));
    str = calloc(N, sizeof(struttura)); //DEVO CAMBIARE N PRIMA O POI
    printf("SUPERVISOR STARTING %d\n", k);
    char *buf = calloc(N, sizeof(char *));

    for (i = 1; i < k+1; i++)
    {
        if (pipe(pipeserver[i].fd) == -1)
        {
            perror("Errore pipe");
            exit(EXIT_FAILURE);
        }
        pipeserver[i].nameserver = i;
        if ((pidfigli[i] = fork()) == -1)
        {
            perror("Errore fork");
            exit(EXIT_FAILURE);
        }
        //printf("x=%d\n",x);
        if (pidfigli[i] == 0) //sono nel figlio
        {
            close(pipeserver[i].fd[0]); //chiudo la pipe in lettura
            if ((execl("/home/tony/Desktop/Progetto_Sistemi/server.out", "server.out", &pipeserver[i].nameserver, &pipeserver[i].fd[1], (char *)NULL)) == -1)
            {
                perror("Excl");
                exit(EXIT_FAILURE);
            }
        }
        else //sono nel padre
            close(pipeserver[i].fd[1]);
    }
    int j = 2 * k;
    int s;
    while (j > 0)
    {
        int numeropipe = rand() % k;
        read(pipeserver[numeropipe].fd[0], buf, N);
        str = tokenizer(str, N, " ", buf, j, numeropipe);
        //printf("SUPERVISOR ESTIMATE %d FOR %d FROM %d\n", str[numeropipe].s, str[numeropipe].id, str[numeropipe].i);
        //printf("SUPERVISOR ESTIMATE %d\n FOR %d\n FROM %d\n", );
        printf("\n");
        j--;
    }

    if (stampa == 1)
        printf("Stampa\n");
    if (stampafine == 1)
    {
        printf("SUPERVISOR EXITING\n");
    }
    return 0;
}
