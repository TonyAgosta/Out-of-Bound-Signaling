#define _POSIX_C_SOURCE 200112L

#include "mylib.h"
#include <arpa/inet.h>

#define SOCKNAME "OOB-server-"
#define SOCK_SIZE_BUF 8

uint64_t rand64bit();
static void run_client();
static void cleanup();
int *socketname;

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

    //ignoro il segnale di sigint
    signal(SIGINT, SIG_IGN);

    //all'uscita chiamo la cleanup che si occupa di liberare tutta la memoria allocata
    if (atexit(cleanup) != 0)
    {
        perror("atexit");
        exit(EXIT_FAILURE);
    }
    p = atoi(argv[1]);
    k = atoi(argv[2]);
    w = atoi(argv[3]);

    //condizioni sui parametri
    if (p < 1 || p > k || w <= 3 * p)
    {
        perror("Numeri scelti male");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) * getpid());

    //richiamo la funzione che sceglie casualmente i p server distinti
    serverscelti = servercasuali(p);

    //genero il secret e l'ID del client
    secret = rand() % 3000 + 1;
    idclient = rand64bit();

    //faccio la conversione da millisecondi a nanosecondi
    int secondi = secret / 1000;
    tempo.tv_nsec = (secret % 1000) * 1000000;
    tempo.tv_sec = secondi;
    printf("CLIENT %lx SECRET %d\n", idclient, secret);

    run_client();
    fflush(NULL);
    return 0;
}

uint64_t rand64bit()
{
    uint64_t x = 0;
    x ^= ((uint64_t)rand() & 0xFFFF);
    x ^= ((uint64_t)rand() & 0xFFFF) << 16;
    x ^= ((uint64_t)rand() & 0xFFFF) << 32;
    x ^= ((uint64_t)rand() & 0xFFFF) << 48;
    return x;
}

static void run_client()
{
    int i;

    //inizializzo l'array che uso per salvare i fd delle socket
    socketname = (int *)calloc(p, sizeof(int));

    //inizia il ciclo che crea i socket con i server scelti prima
    for (i = 0; i < p; i++)
    {
        struct sockaddr_un sa;
        memset(&sa, 0, sizeof(sa));
        socketname[i] = socket(AF_UNIX, SOCK_STREAM, 0);

        sprintf(sa.sun_path, "OOB-server-%ld", serverscelti[i]);
        sa.sun_family = AF_UNIX;
        errno = 0;

        //riprovo finche il socket non viene creato
        while (connect(socketname[i], (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {

            if (errno == ENOENT)
            {
                sleep(1);
            }
        }
    }
    srand(time(NULL));
    int x;

    //faccio la conversione del ID del client prima di mandarlo al server
    uint64_t idhorder = HTONLL(idclient);
    //inizia il ciclo di invio dei W messaggi
    for (i = 0; i < w; i++)
    {
        x = rand() % p;
        writen(socketname[x], &idhorder, SOCK_SIZE_BUF);
        nanosleep(&tempo, NULL);
    }
    for (i = 0; i < p; i++)
    {
        close(socketname[i]);
    }

    printf("Client %lx ended\n", idclient);
    fflush(NULL);
}

static void cleanup()
{
    free(serverscelti);
    free(socketname);
}
