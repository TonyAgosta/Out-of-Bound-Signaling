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

#define SOCKNAME "OOB-server-"
#define N 100

int secret;
long idclient;
int p, k, w;
int *serverscelti;
static void run_client();
int *servercasuali(int p);
struct timespec tempo;
int *server;


static void eliminasocket(){
    free(serverscelti);
    //free(server);
}

int main(int argc, char *argv[])
{

    if (argc < 4)
    {
        perror("Pochi argomenti nel client");
        exit(EXIT_FAILURE);
    }
    if(atexit(eliminasocket)!=0){
        perror("atexit");
        exit(EXIT_FAILURE);
    }
    p = atoi(argv[1]);
    k = atoi(argv[2]);
    w = atoi(argv[3]);
    //fout = fopen(argv[4], "w");
    if (p < 1 || p > k || w <= 3 * p)
    {
        perror("Numeri scelti male");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    serverscelti = servercasuali(p);
    secret = rand() % 30 + 1;
    idclient = rand() % 10;

    int secondi = secret / 1000;
    tempo.tv_nsec = (secret % 1000) * 1000000;
    tempo.tv_sec = secondi;
    printf("CLIENT %ld SECRET %d\n", idclient, secret);
    run_client();
    return 0;
}
int *servercasuali(int p)
{
    server = calloc(p, sizeof(int)); //alloco l'array con dimensione p
    int casuale, i;//j=1;
    //int fatto = 0,trovato=0;

   /* while (!fatto)
    {
        if(j==p+1) fatto=1;
        casuale = rand() % k + 1;
        for(i=1;i<p+1;i++){
            if(server[i]==casuale) trovato=1;
        }
        if(!trovato){
            server[j]=casuale;
            j++;
        }
    }*/

    for (i = 1; i < p+1 ; i++)
    {
        casuale = rand() % k+1;
        server[i] = casuale;
        
    }

    return server;
}

static void run_client()
{
    int i = 1;
    long *socketname; //array che uso per salvare il nome di ogni socket
    char *ns, *OO;
    socketname = calloc(p + 1, sizeof(long)); //alloco l'array con dimensione p
    //int k = -1;
    for (i = 1; i < p + 1; i++)
    {
        struct sockaddr_un sa;
        socketname[i] = socket(AF_UNIX, SOCK_STREAM, 0);

        OO = malloc(16 * sizeof(char *));
        strcpy(OO, "OOB-server-");
        ns = calloc(1, sizeof(int));
        sprintf(ns, "%d", serverscelti[i]);
        strcat(OO, ns);
        strcpy(sa.sun_path, OO);
        printf("%s\n", sa.sun_path);
        sa.sun_family = AF_UNIX;
        while (connect(socketname[i], (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {

            if (errno != ENOENT)
                perror(sa.sun_path);
            sleep(1);
        }
    }
    srand(time(NULL));
    for (i = 0; i < w; i++)
    {
        int x = rand() % p + 1;
        write(socketname[x], &idclient, sizeof(idclient));
        nanosleep(&tempo, NULL);
    }
    for(i=1;i<p+1;i++){
        close(socketname[i]);
    }

    printf("Client %ld ended\n", idclient);
}