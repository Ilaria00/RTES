#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define N 10 //numero auto

#define S 4 //numero sezioni

struct semaforoprivato_t {
    sem_t s;
    int c_attesa; //numero di thread in attesa su quel semaforo
    bool occupata;
};

struct rotonda_t {
    sem_t mutex;

    struct semaforoprivato_t sezione[S];

    int auto[N]; //auto[0] = 1 significa che l'auto 0 sta occupando la sezione 1

} rotonda;

void init_semaforoprivato (struct semaforoprivato_t *s) {
    sem_init(&s->s, 0, 0);
    s->c_attesa = 0;
    s->occupata = false;
}

void init_rotonda (struct rotonda_t *r) {
    sem_init(&r->mutex, 0, 1);

    for (int i=0; i<S; i++) {
        init_semaforoprivato(&r->sezione[i]);
    }

    for (int i=0; i<N; i++) {
        r->auto[i] = -1;
    }
}

void entra (struct rotonda_t *r, int numeroauto, int sezione) {
    sem_wait(&r->mutex);

    /*se la sezione è occupata mi blocco*/
    if (r->sezione[sezione].occupata){
        printf("L'auto %d attende di entrare nella sezione %d perche' momentaneamente occupata\n", numeroauto, sezione);
        r->sezione[sezione].c_attesa++;
        sem_post(&r->mutex);
        sem_wait(&r->sezione[sezione].s);
        r->sezione[sezione].c_attesa--;
    }

    /*occupo la sezione che si e' liberata
    e se non sono appena entrata in rotonda
    libero la sezione occupata precedentemente*/
    if (r->auto[numeroauto] == sezione - 1) {
        /*allora significa che prima stavo occupando un'altra sezioen che ora libero*/
        r->sezione[sezione - 1].occupata = false;
        /*e se c'era qualcuno inattesa per entrare in quella sezioen lo sblocco*/
        if (r->sezione[sezione - 1].c_attesa) {
            sem_post(&r->sezione[sezione - 1].s);
        }
    }
    /*altrimenti significa che vale -1 ovvero che sono appena entrata e che non stavo occupando nulla precedentemente*/
    printf("La sezione %d si e' liberata e ora l'auto %d la sta occupando\n", sezione, numeroauto);
    r->sezione[sezione].occupata = true;
    r->auto[numeroauto] = sezione;
    sem_post(&r->mutex);
}

int sonoarrivato (struct rotonda_t *r, int numeroauto, int destinazione) {
    sem_wait(&r->mutex);
    int ret;

    /*se la sezione che numeroauto sta occupando e' quella di destinazione allora ritorna 0*/
    if (r->auto[numeroauto] == destinazione) {
        printf("L'auto %d si trova nella sezione di uscita %d\n", numeroauto, destinazione);
        ret = 0;
        sem_post(&r->mutex);
    }
    else {
        printf("L'auto %d prosegue verso %d\n", numeroauto, r->auto[numeroauto] + 1);
        ret = 1;
        sem_post(&r->mutex);
        entra(&r, numeroauto, r->auto[numeroauto] + 1);
    }
    return ret;
}

void esci (struct rotonda_t *r, int numeroauto) {
    sem_wait(&r->mutex);
    int sezione_occupata;
    printf("L'auto %d esce dalla rotonda\n", numeroauto);
    sezione_occupata = r->auto[numeroauto];
    r->sezione[sezione_occupata].occupata = false;
    if (r->sezione[sezione_occupata].c_attesa) {
            sem_post(&r->sezione[sezione_occupata].s);
    }
    else {
        sem_post(&r->mutex);
    }
}

void *auto (void *arg) 
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

int main (int argc, char* argv[]) 
{
    pthread_t tauto[N];
    int tauto_id[N];

    init_rotonda(&rotonda);

    for (int i=0; i<N; i++) {
        tauto_id[i] = i;
        pthread_create(&tauto[i], NULL, auto, &tauto_id[i]);
    }

    for (int i=0; i<N; i++) {
        pthread_join(tauto[i], NULL);
    }

    return 0;
}
