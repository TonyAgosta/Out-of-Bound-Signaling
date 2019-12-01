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
int idclient;
int p, k, w;
int *serverscelti;
static void run_client();
int *servercasuali(int p);



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

    if (argc < 4)
    {
        perror("Pochi argomenti nel client");
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
    secret = rand() % 3000+1;
    idclient = rand()%10;
    printf("CLIENT %d SECRET %d\n", idclient, secret);
    run_client();
    return 0;
}
int *servercasuali(int p)
{
    int *server = calloc(p, sizeof(int)); //alloco l'array con dimensione p
    int casuale,i;

	
	for(i=0; i<k; i++) {
		casuale = (rand()%(k-i)) + i;
        server[i]=casuale;   
    }
	
	return server;
}

static void run_client()
{
    int i=1;
    long *socketname; //array che uso per salvare il nome di ogni socket
    char *ns, *OO;
    socketname = calloc(p+1, sizeof(long)); //alloco l'array con dimensione p
    char **OOarray;
    OOarray = malloc(p * sizeof(char *));
    int k = -1;
    for ( i = 1; i < p+1; i++)
    {
        struct sockaddr_un sa;
        socketname[i] = socket(AF_UNIX, SOCK_STREAM, 0);
        
        OO = malloc(16 * sizeof(char *));
        strcpy(OO, "OOB-server-");
        ns = calloc(1,sizeof(int));
        sprintf(ns, "%d", serverscelti[i]);
        strcat(OO, ns);
        strcpy(sa.sun_path, OO);
        printf("%s\n",sa.sun_path);
        sa.sun_family = AF_UNIX;
        OOarray[i] = malloc(16 * sizeof(char *));
        strcpy(OOarray[i], OO);
        while (connect(socketname[i], (struct sockaddr *)&sa, sizeof(sa))<0)
        {

            if(errno!=ENOENT) perror(sa.sun_path);
            sleep(1);
        }
    
    
    //i=0;
    char buffer[5];
    strcpy(buffer,"ciao");
    //for(i=0;i<p;i++){
        //int x=rand()%p;
        //printf("x=%d\n",x);
        //printf("socketname=%d\n",socketname[j]);
        //long fd=3;
        int v=write(socketname[i],&buffer,5);
    }
    printf("Client %d ended\n",idclient);
    //}
}