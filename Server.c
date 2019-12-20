#define _POSIX_C_SOURCE 200112L
#include "mylib.h"

#define UNIX_PATH_MAX 108

static void run_server();
static void *stampamessaggio(void *arg);
void eliminaSocket()
{

    listadescriptor *corr = lista;
    while (corr != NULL)
    {
        pthread_join(corr->t_id, (void *)NULL);
        close(corr->t_id);
        corr = corr->next;
    }
    corr=lista;
    while(corr!=NULL){
        lista=lista->next;
        free(corr);
        corr=lista;
    }
    
}

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
    fflush(NULL);
    return 0;
}


static void run_server()
{
    char nomesocket[16];
    int fd_skt;
    struct sockaddr_un sa;
    sprintf(nomesocket,"OOB-server-%ld",nomeserver);
    unlink(nomesocket);
    
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
        printf("SERVER %ld CONNECT FROM CLIENT\n", nomeserver);
        fflush(NULL);
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
    long stima = -1; //data dalla differenza di due messaggi successivis
    long stima1 = -1;
    struct timespec tempo;
    long tempo_prec, ns;
    messaggio m;
    stima = -1;
    uint64_t x=-1;
    while (read(fdlista->fd, &x, sizeof(long)) > 0)
    {
        x = NTOHLL(x);
        clock_gettime(CLOCK_REALTIME, &tempo);
        ns = ((tempo.tv_sec % 10000) * 1000) + (tempo.tv_nsec / 1000000); //e` il tempo attuale
        printf("SERVER %ld INCOMING FROM %lx @ %ld\n", nomeserver, x, ns);
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
    m.client = x;
    m.server = 0;
    m.stima = stima1 % 10000;
    int w = write(numpipe, &m, sizeof(messaggio));
    
    if (w <= 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
	printf("SERVER %ld CLOSING %lx ESTIMATE %ld\n", nomeserver, x, m.stima);
	fflush(NULL);
    close(lista->fd);
    pthread_exit(NULL);
}
