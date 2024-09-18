#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define N 4 //numero auto

#define S 5 //numero sezioni 

struct rotonda_t {
    pthread_mutex_t mutex;

    pthread_cond_t cond_sezione[S];

    bool sezione[S]; //sezione[i] vale 0 se i è libera, 1 se è occupata
    int automobile[N]; //automobile[0] = 1 significa che l'auto 0 sta occupando la sezione 1
} rotonda;

void init_rotonda (struct rotonda_t *r) {
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&r->mutex, &m_attr);

    for (int i=0; i<S; i++) {
        pthread_cond_init(&r->cond_sezione[i], c_attr);
    }

    for (int i=0; i<S; i++) {
        r->sezione[i] = 0;
    }

    for (int i=0; i<N; i++) {
        r->automobile[i] = -1; //inizialmente le auto non stanno occupando nessuna sezione
    }

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);
}

void entra (struct rotonda_t *r, int numeroauto, int sezione) {
    pthread_mutex_lock(&r->mutex);

    printf("Lauto %d chiede di entrare nella sezione %d\n", numeroauto, sezione);
    while (r->sezione[sezione] == 0) {
        /*finché la sezione è occupata*/
        printf("Lauto %d attende il suo ingrsso nella sezione %d occupata\n", numeroauto, sezione);
        pthread_cond_wait(&r->cond_sezione[sezione], &r->mutex);
    }
    printf("L'auto %d e' entrata nella sezione %d\n", numeroauto, sezione);
    /*se non sono appena entrata allora libero la sezione che stavo occupando*/
    if (r->automobile[numeroauto] == (sezione - 1 + S) % S) {
        /*se L'auto numeroauto stava occupando la sezione precedente a quella corrente*/
        printf("L'auto %d libera la sezione precedente %d e sveglia un auto in attesa su %d\n", numeroauto, (sezione - 1 + S) % S, (sezione - 1 + S) % S);
        r->sezione[(sezione - 1 + S) % S] = 0;
        pthread_cond_signal(&r->cond_sezione[(sezione - 1 + S) % S]);
    }
    r->sezione[sezione] = 1;
    r->automobile[numeroauto] = sezione;

    pthread_mutex_unlock(&r->mutex);
}

int sonoarrivato (struct rotonda_t *r, int numeroauto, int destinazione) {
    pthread_mutex_lock(&r->mutex);

    int ret;
    if (r->automobile[numeroauto] == destinazione) {
        printf("Lauto %d e' arrivata nella sezione di uscita %d\n", numeroauto, destinazione);
        ret = 0;
        pthread_mutex_unlock(&r->mutex);
    }
    else {
        ret = 1; 
        printf("L'auto %d prosegue verso %d\n", numeroauto, (r->automobile[numeroauto] + 1)%S);
        pthread_mutex_unlock(&r->mutex);
        entra (&rotonda, numeroauto, (r->automobile[numeroauto] + 1)%S);
    }
    return ret;
}

void esci (struct rotonda_t *r, int numeroauto) {
    pthread_mutex_lock(&r->mutex);

    printf("L'auto %d esce dalla sezione %d\n", numeroauto, r->automobile[numeroauto]);
    /*libero la sezione che sto occupando e sveglio un'eventuale auto in attesa*/
    int sezione_da_liberare = r->automobile[numeroauto];
    r->sezione[sezione_da_liberare] = 0;
    pthread_cond_signal(&r->cond_sezione[sezione_da_liberare]);
    pthread_mutex_unlock(&r->mutex);
}

void *auto_thread (void *arg) 
{
    int sezionediingresso = rand() % S;
    int destinazione = rand() % S;
    int numeroauto = *((int*) arg);

    entra(&rotonda, numeroauto, sezionediingresso);
    do {
        /*percorri la sezione corrente della rotonda*/
    } while (sonoarrivato(&rotonda, numeroauto, destinazione));
    esci(&rotonda, numeroauto);
}

int main (int argc, char* argv[]) {
    ptrhead_t tauto[N];

    int auto_id[N];

    init_rotonda(&rotonda);

    for (int i=0; i<N; i++) {
        auto_id[i] = i;
        pthread_create(&tauto[i], NULL, auto_thread, &auto_id[i]);
    }

    for (int i=0; i<N; i++) {
        pthread_join(tauto[i], NULL);
    }
}
