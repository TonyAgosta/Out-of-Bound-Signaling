#define _POSIX_C_SOURCE 200112L

#include "mylib.h"
#include <arpa/inet.h>

#define SOCKNAME "OOB-server-"
#define SOCK_SIZE_BUF 8

uint64_t rand64bit();
//struct timeval t;
static void run_client();
static void cleanup();
int *socketname;

static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
    if ((r=write((int)fd ,bufptr,left)) == -1) {
        if (errno == EINTR) continue;
        return -1;
    }
    if (r == 0) return 0;  
        left    -= r;
    bufptr  += r;
    }
    return 1;
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        perror("Pochi argomenti nel client");
        exit(EXIT_FAILURE);
    }
    if (atexit(cleanup) != 0)
    {
        perror("atexit");
        exit(EXIT_FAILURE);
    }
    p = atoi(argv[1]);
    k = atoi(argv[2]);
    w = atoi(argv[3]);

    if (p < 1 || p > k || w <= 3 * p)
    {
        perror("Numeri scelti male");
        exit(EXIT_FAILURE);
    }
    //socketname=(int*) calloc(p, sizeof(int)); //array che uso per salvare il nome di ogni socket
    srand(time(NULL)*getpid());
    serverscelti = servercasuali(p);
    secret = rand() % 3000 + 1;
    idclient = rand64bit();

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
    //int *socketname;
    socketname=(int*) calloc(p, sizeof(int)); //per salvare i fd delle socket
    for (i = 0; i < p; i++)
    {
        struct sockaddr_un sa;
        memset(&sa, 0, sizeof(sa));
        socketname[i] = socket(AF_UNIX, SOCK_STREAM, 0);

        sprintf(sa.sun_path, "OOB-server-%ld", serverscelti[i]);
        sa.sun_family = AF_UNIX;
        errno=0;
        while (connect(socketname[i], (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {

            if (errno == ENOENT){
                //perror(sa.sun_path);
                sleep(1);
            }
        }
    }
    srand(time(NULL));
    int x;
    uint64_t idhorder = HTONLL(idclient);
    for (i = 0; i < w; i++)
    {
        x = rand() % p;
        //fprintf(stderr,"%d\n",socketname[x]);
        writen(socketname[x], &idhorder, SOCK_SIZE_BUF);
        nanosleep(&tempo, NULL);
    }
    for (i = 0; i < p; i++)
    {
        close(socketname[i]);
    }

    printf("Client %lx ended\n", idclient);
    fflush(NULL);
    //free(socketname);
}

static void cleanup()
{
    free(serverscelti);
    free(socketname);
}
