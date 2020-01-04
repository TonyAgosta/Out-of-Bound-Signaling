//Tony Agosta 544090

#define _POSIX_C_SOURCE 200112L
#include "mylib.h"

#define UNIX_PATH_MAX 108
#define SOCK_SIZE_BUF 8

static void run_server();
static void *stampamessaggio(void *arg);
long nomeserver;
int numpipe;

//CLENAUP//
//chiudo ed elimino i socket una volta terminato
void eliminaSocket()
{

    listadescriptor *corr = lista;
    while (corr != NULL)
    {
        pthread_join(corr->t_id, (void *)NULL);
        close(corr->t_id);
        corr = corr->next;
    }
    corr = lista;
    while (corr != NULL)
    {
        lista = lista->next;
        free(corr);
        corr = lista;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Numero di argometi errato nel server");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, SIG_IGN);
    nomeserver = (long)(*argv[1]) + 1;
    numpipe = (int)(*argv[2]);
    run_server();

    //all'uscita chiamo la eliminasocket che si occupa di eliminare in modo adeguato i socket creati
    if (atexit(eliminaSocket) != 0)
    {
        perror("eliminaSocket");
    }
    fflush(NULL);
    return 0;
}

static void run_server()
{
    char nomesocket[16]; //variabiale per salvare il nome del socket
    int fd_skt;
    struct sockaddr_un sa;
    sprintf(nomesocket, "OOB-server-%ld", nomeserver); //creo il nome del socket

    //non posso creare un socket se esiste gia un file con quel nome
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

    lista = NULL;

    //attendo la connessio da parte di un client
    while (1)
    {

        long fd1; //fd del del client che si connettera con il server e che poi salvo nella lista per gestire il multithreading
        fd1 = accept(fd_skt, NULL, 0);

        printf("SERVER %ld CONNECT FROM CLIENT\n", nomeserver);
        fflush(NULL);

        //creo un nuovo elemento della lista per rappresentare il nuovo thread
        listadescriptor *new = malloc(sizeof(listadescriptor));
        new->fd = fd1;
        if ((pthread_create(&new->t_id, NULL, stampamessaggio, (void *)new)) != 0)
        {
            perror("Thread non creato");
            exit(EXIT_FAILURE);
        }

        //inserisco il nuovo thread creato in testa alla lista
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
    int bprimo = 0;  //ho già letto il primo messaggio?
    long stima = -1; //data dalla differenza di due messaggi successivi
    long stima1 = -1;
    struct timespec tempo;
    long tempo_prec, ns;
    messaggio m;
    stima = -1;
    uint64_t x;
    int w;
    while (readn(fdlista->fd, &x, SOCK_SIZE_BUF) > 0)
    {
        x = NTOHLL(x); //traduco l'id in host bytes order
        clock_gettime(CLOCK_REALTIME, &tempo);
        ns = ((tempo.tv_sec % 10000) * 1000) + (tempo.tv_nsec / 1000000); //e` il tempo attuale
        printf("SERVER %ld INCOMING FROM %lx @ %ld\n", nomeserver, x, ns);
        if (bprimo == 1) //ho gia` letto il primo messaggio
        {
            stima = ns - tempo_prec;
            tempo_prec = ns;
            //di volta in volta prendo la miglior stima
            if (stima1 == -1)
                stima1 = stima;
            else if (stima1 > stima)
                stima1 = stima;
        }
        else //devo ancora legger il primo messagio
        {
            bprimo = 1;
            tempo_prec = ns;
        }
    }
    if (stima1 == -1) //ho ricevuto un solo messaggio, quindi non posso fare stime
        pthread_exit(NULL);
    //setto i vari campi della struttura che uso per mandare il messaggio al supervisor tramite pipe
    m.client = x;
    m.server = 0; //e` settato sempre a 0; lo incremento,se e` da incrementare, nel supervisor facendo opportuni controlli
    m.stima = stima1 % 10000;
    w = write(numpipe, &m, sizeof(messaggio));

    if (w <= 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    printf("SERVER %ld CLOSING %lx ESTIMATE %ld\n", nomeserver, x, m.stima);
    fflush(NULL);
    pthread_exit(NULL);
}
