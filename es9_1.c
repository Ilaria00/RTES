#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#define M 5 //numero mittenti

typedef int T;

struct semaforoprivato_t {
    sem_t s;
    int nbw;
    int nw;
} semaforo;

void semaforoprivato_init (struct semaforoprivato_t *s) {
    sem_init(&s->s, 0, 0);
    s->nbw = 0;
    s->nw = 0;
}

struct gestore_t {
    sem_t mutex; 
    sem_t priv_R; //semaforo del lettore e dello scrittore
    semaforoprivato_t priv_W[M];
    int nbr; //thread bloccati
    int nr; //thread in esecuzione
    int num_mex;
    T mex[M];
} gestore;

void gestore_init (struct gestore_t *g) {
    sem_init(&g->mutex, 0, 1);
    sem_init(&g->priv_R, 0, 0);
    for (int i=0; i<M; i++) {
        semaforoprivato_init(&g->priv_W[i]);
    }
    g->nr = 0;
    g->nbr = 0;
    g->num_mex = 0;
    for(int i=0; i<M; i++) {
       g-> mex[i] = -1; //inizialmente è vuoto
    }
}

void send (struct gestore_t *g, int i) {
    g->mex[i] = i; /*non mi serve fare mutua esclusione sul messaggio
    poiché ciascun thread opera su un indice diverso!*/

    sem_wait(&g->mutex);
    g->num_mex++;
    if (g->num_mex == 5) {
        sem_post(&g->priv_R);
    }
    sem_post(&g->mutex);
    sem_wait(&g->priv_W[i].s);
    /*ciascun produttore si blocca sul proprio semaforo, 
    in attesa che il consumatore lo svegli*/
}

void receive (struct gestore_t *g) {
    sem_wait(&g->priv_R);
    sem_wait(&g->mutex);

    g->num_mex = 0;
    for (int i=0; i<M; i++) {
        sem_post(&g->priv_W[i].s);
    }
    sem_post(&g->mutex);
}

void *producer (void *arg) {
    int thread_idx = *(int*) arg;
    while (1) {
        send(&gestore, thread_idx);
        sleep(1);
    }
    pthread_exit(0);
}

void *consumer (void *arg) {
    while(1) {
        receive(&gestore);
        sleep(1);
    }
    pthread_exit(0);
}

int main(void){

    pthread_attr_t a;
    pthread_t threads[6];
    int consumer_ids[5] = { 0, 1, 2, 3, 4 };
    int current_consumer = 0;

    pthread_attr_init(&a);

    gestore_init(&gestore);

    for(int i = 0; i < 6; i++){

        if(i == 0) 
            pthread_create(&threads[i], &a, consumer, NULL); // il primo processo è il lettore
        else{
            pthread_create(&threads[current_consumer], &a, producer, (void*) &consumer_ids[current_consumer]); // gli altri sono consumatori, ai quali assegno un indice
            current_consumer++;
        }

    }

    for(int i = 0; i < 6; i++){

        int *retval;
        pthread_join(threads[i], (void**) &retval);

    }

    return 0;

}
