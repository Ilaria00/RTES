#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define SHAVING_ITERATIONS 1000
#define PAYING_ITERATIONS  1000

#define NUM_CLI 10

struct gestore_t {
    pthread_mutex_t m;

    pthread_cond_t divano;
    pthread_cond_t cassiere;
    pthread_cond_t barbiere;

    int c_barbiere, c_divano, c_cassiere;
}

void gestore_init (struct gestore_t *g) {
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_condattr_init(&condattr);

    ptrhead_mutex_init(&g->m, &mutexattr);
    ptrhead_cond_init(&g->divano, &condattr);
    pthread_cond_init(&g->cassiere, &condattr);
    pthread_cond_init(&g->barbiere, &condattr);

    g->c_barbiere = 0;
    g->c_divano = 0;
    g->c_cassiere = 0;
}

void acquisisci_divano (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    //mi blocco finché i posti sul divano sono occupati
    while (g->c_divano >= 4) {
        pthread_cond_wait(&g->divano);
    }
    g->c_divano++;

    pthread_mutex_unlock(&g->m);
}

void acquisisci_barbiere (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    //mi blocco finché i barbieri sono occupati
    while (g->c_barbiere >= 3) {
        pthread_cond_wait(&g->barbiere);
    }
    g->c_divano--;
    pthread_cond_signal(&g->divano);
    g->c_barbiere++;

    //taglio capelli
    i = SHAVING_ITERATIONS
    while(i-- > 0) {};

    pthread_mutex_unlock(&g->m);
}

void acquisisci_cassiere (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    while (p->c_cassiere >= 1) {
        pthread_cond_wait(&g->cassiere);
    }
    g->c_barbiere--
    pthread_cond_signal(&g->barbiere);
    g->c_cassiere++;

    //pagamento
    i = PAYING_ITERATIONS;
    while(i-- > 0) {};

    pthread_mutex_unlock(&g->m);
}

void *cliente (void *arg) {
      int thread_id = *((int*) arg);
      int i;
    while(true) {
        acquisisci_divano(&g);
        acquisisci_barbiere(&g);
        acquisisci_cassiere(&g);
        printf("%d> FATTO!\n", thread_id); fflush(stdout);
        // e ricomincio...
        sleep(1);
    }
}

int main(int argc, char* argv[])
{
    pthread_t threads[NUM_CLI];
    int thread_ids[NUM_CLI];

    gestore_init(&g_gestore);

    for (int i = 0; i < NUM_CLI; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, cliente, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_CLI; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

