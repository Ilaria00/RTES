#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define N 7 //numero auto

#define S 3 //numero sezioni

struct semaforoprivato_t {
    sem_t s;
    int c_attesa; //numero di thread in attesa su quel semaforo
    bool occupata;
};

struct rotonda_t {
    sem_t mutex;

    struct semaforoprivato_t sezione[S];

    int automobile[N]; //auto[0] = 1 significa che l'auto 0 sta occupando la sezione 1

    int num_sezionioccupate;

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
        r->automobile[i] = -1;
    }
    r->num_sezionioccupate = 0;
}

void entra (struct rotonda_t *r, int numeroauto, int sezione) {
    sem_wait(&r->mutex);

    /*se la sezione è occupata mi blocco*/
    if (r->sezione[sezione].occupata || r->num_sezionioccupate == S-1){
        printf("L'auto %d attende di entrare nella sezione %d perche' momentaneamente occupata\n", numeroauto, sezione);
        r->sezione[sezione].c_attesa++;
        sem_post(&r->mutex);
        sem_wait(&r->sezione[sezione].s);
        r->sezione[sezione].c_attesa--;
    }

    /*occupo la sezione che si e' liberata
    e se non sono appena entrata in rotonda
    libero la sezione occupata precedentemente*/
    printf("dopo entra e prima dell'if: r->automobile[%d] = %d\n", numeroauto, r->automobile[numeroauto]);
    if (r->automobile[numeroauto] == ((sezione - 1 + S)%S)) {
        /*allora significa che prima stavo occupando un'altra sezione che ora libero*/
        printf("Prima l'auto %d stava occupando la sezione %d che ora viene liberata\n Ora le sezioni occupate sono %d\n", numeroauto, (sezione - 1 + S)%S, r->num_sezionioccupate);
        r->sezione[(sezione - 1 + S)%S].occupata = false;
        r->num_sezionioccupate--;
        /*e se c'era qualcuno in attesa per entrare in quella sezione lo sblocco*/
        if (r->sezione[(sezione - 1 + S)%S].c_attesa) {
            sem_post(&r->sezione[(sezione - 1 + S)%S].s);
        }
    }
    /*altrimenti significa che vale -1 ovvero che sono appena entrata e che non stavo occupando nulla precedentemente*/
    printf("La sezione %d si e' liberata e ora l'auto %d la sta occupando\n", sezione, numeroauto);
    r->num_sezionioccupate++;
    r->sezione[sezione].occupata = true;
    r->automobile[numeroauto] = sezione;
    printf("dopo entra e dopo if: r->automobile[%d] = %d\n", numeroauto, r->automobile[numeroauto]);
    sem_post(&r->mutex);
}

int sonoarrivato (struct rotonda_t *r, int numeroauto, int destinazione) {
    sem_wait(&r->mutex);
    int ret;

    /*se la sezione che numeroauto sta occupando e' quella di destinazione allora ritorna 0*/
    if (r->automobile[numeroauto] == destinazione) {
        printf("L'auto %d si trova nella sezione di uscita %d\n", numeroauto, destinazione);
        ret = 0;
        sem_post(&r->mutex);
    }
    else {
        printf("L'auto %d prosegue verso %d\n", numeroauto, (r->automobile[numeroauto] + 1)%S);
        ret = 1;
        sem_post(&r->mutex);
        printf("prima di entra: r->automobile[%d] = %d\n", numeroauto, r->automobile[numeroauto]);
        entra(&rotonda, numeroauto, (r->automobile[numeroauto] + 1)%S);
    }
    return ret;
}

void esci (struct rotonda_t *r, int numeroauto) {
    sem_wait(&r->mutex);
    int sezione_occupata;
    printf("L'auto %d esce dalla rotonda e stava\n", numeroauto);
    sezione_occupata = r->automobile[numeroauto];
    r->sezione[sezione_occupata].occupata = false;
    r->num_sezionioccupate--;
    if (r->sezione[sezione_occupata].c_attesa) {
            sem_post(&r->sezione[sezione_occupata].s);
    }
    else {
        sem_post(&r->mutex);
    }
}

void *automobile (void *arg) 
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
        pthread_create(&tauto[i], NULL, automobile, &tauto_id[i]);
    }

    for (int i=0; i<N; i++) {
        pthread_join(tauto[i], NULL);
    }

    return 0;
}
