#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define M 5 //numero mittenti

typedef int T;

struct semaforoprivato_t {
    sem_t s;
    int nbw;
    int nw;
} semaforo;

struct gestore_t {
    sem_t mutex; 
    sem_t priv_R; //semaforo del lettore e dello scrittore
    struct semaforoprivato_t priv_W[M];
    int nbr; //thread bloccati
    int nr; //thread in esecuzione
    int num_mex;
    T mex[M];
} gestore;

void semaforoprivato_init (struct semaforoprivato_t *s) {
    sem_init(&s->s, 0, 0);
    s->nbw = 0;
    s->nw = 0;
}

void gestore_init (struct gestore_t *g) {
    sem_init(&g->mutex, 0, 1);
    sem_init(&g->priv_R, 0, 0);
    for (int j=0; j<M; j++) {
        semaforoprivato_init(&g->priv_W[j]);
    }
    g->nr = 0;
    g->nbr = 0;
    g->num_mex = 0;
    for(int k=0; k<M; k++) {
        g->mex[k] = -1; //inizialmente è vuoto
    }
}

void *producer (void *arg, struct gestore_t *g) {
    int thread_idx = *(int*) arg;
    while (1) {
        g->mex[thread_idx] = thread_idx; /*non mi serve fare mutua esclusione sul messaggio
        poiché ciascun thread opera su un indice diverso!*/
        printf("thread %d inserisce '%d'\n", thread_idx, thread_idx);
        sem_wait(&g->mutex);
        g->num_mex++;
        if (g->num_mex == M) {
            sem_post(&g->priv_R);
        }
        sem_post(&g->mutex);
        sem_wait(&g->priv_W[thread_idx].s);
    /*ciascun produttore si blocca sul proprio semaforo, 
    in attesa che il consumatore lo svegli*/
        sleep(1);
    }
    pthread_exit(0);
}

void *consumer (void *arg, struct gestore_t *g) {
    while(1) {
        sem_wait(&g->priv_R);
        sem_wait(&g->mutex);
        printf("Letto il messaggio %d%d%d%d%d", g->mex[0], g->mex[1], g->mex[2], g->mex[3], g->mex[4]);
        g->num_mex = 0;
        for (int k=0; k<M; k++) {
            sem_post(&g->priv_W[k].s);
        }
        sem_post(&g->mutex);
    }
    pthread_exit(0);
}

int main(void){

    pthread_attr_t a;
    pthread_t threads[6];
    int consumer_ids[M] = { 0, 1, 2, 3, 4 };
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
