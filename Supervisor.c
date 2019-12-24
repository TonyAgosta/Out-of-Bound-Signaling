#define _POSIX_C_SOURCE 200112L
#include "mylib.h"

void server_run(int k);
static void gestoresigint(int signum);
void closeallsupervisor();

pid_t *pidfigli; //per salvare i pid dei figli creati
pipee *pipeserver;

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
    printf("SUPERVISOR STARTING %d\n", k);
    fflush(NULL);
    server_run(k);
    return 0;
}

void server_run(int k)
{
    int i = 0;

    pidfigli = (pid_t *)calloc(k, sizeof(pid_t)); //alloco spazio per l'array in cui salvo i pid dei figli
    pipeserver = (pipee *)calloc(k, sizeof(pipee));//alloco spazio per l'array in cui salvo i fd delle pipe con i server
    for (i = 0; i < k; i++)
    {
        if (pipe(pipeserver[i].fd) == -1)
        {
            perror("Errore pipe");
            exit(EXIT_FAILURE);
        }
        if ((pidfigli[i] = fork()) == -1)
        {
            perror("Errore fork");
            exit(EXIT_FAILURE);
        }
        if (pidfigli[i] == 0) //sono nel figlio
        {
            close(pipeserver[i].fd[0]); //chiudo la pipe in lettura
            //avvio i server
            if ((execl("server", "supervisor", &i, &pipeserver[i].fd[1], (char *)NULL)) == -1)
            {
                perror("Excl");
                exit(EXIT_FAILURE);
            }
        }
        else
        { //sono nel padre
            
            //rendo la read non bloccante
            int x = fcntl(pipeserver[i].fd[0], F_GETFL, 0);
            fcntl(pipeserver[i].fd[0], F_SETFL, x | O_NONBLOCK);
            close(pipeserver[i].fd[1]);
        }
    }
    srand(time(NULL));
    int numeroserver = rand() % k ;
    int s = -1;
    while (1)
    {
        errno = 0;
        numeroserver = rand() % k;
        //leggo dalla pipe che collega il supervisor con i server
        s = read(pipeserver[numeroserver].fd[0], &elemento, sizeof(messaggio));
        if (s > 0)
        {
            printf("SUPERVISOR ESTIMATE %ld FOR %lx FROM %d\n", elemento.stima, elemento.client, numeroserver+1);
            fflush(NULL);
            l = listastima(l, elemento);
        }
        if (s <= 0)
        {
            if (errno == 11)
                continue;
        }
    }
}

static void gestoresigint(int signum)
{

    if (signum == SIGINT)
    {
        printf("\n");
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

void closeallsupervisor(){
    free(pidfigli);
    free(pipeserver);
}
