#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define SHAVING_ITERATIONS 1000
#define PAYING_ITERATIONS  1000

#define NUM_CLI 10

struct gestore_t {
    pthread_mutex_t m;

    pthread_cond_t divano;
    pthread_cond_t cassiere;
    pthread_cond_t barbiere;

    int c_barbiere, c_divano;
    bool cassiere_libero;
} g;

void gestore_init (struct gestore_t *g) {
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_condattr_init(&condattr);

    pthread_mutex_init(&g->m, &mutexattr);
    pthread_cond_init(&g->divano, &condattr);
    pthread_cond_init(&g->cassiere, &condattr);
    pthread_cond_init(&g->barbiere, &condattr);

    g->c_barbiere = 0;
    g->c_divano = 0;
    g->cassiere_libero = true;

    pthread_condattr_destroy(&condattr);
    pthread_mutexattr_destroy(&mutexattr);
}

void acquisisci_divano (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    //mi blocco finché i posti sul divano sono occupati
    while (g->c_divano >= 4) {
        pthread_cond_wait(&g->divano, &g->m);
    }
    g->c_divano++;

    pthread_mutex_unlock(&g->m);
}

void acquisisci_barbiere (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    //mi blocco finché i barbieri sono occupati
    while (g->c_barbiere >= 3) {
        pthread_cond_wait(&g->barbiere, &g->m);
    }
    g->c_divano--;
    pthread_cond_signal(&g->divano);
    g->c_barbiere++;

    pthread_mutex_unlock(&g->m);
}

void acquisisci_cassiere (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    while (!g->cassiere_libero) {
        pthread_cond_wait(&g->cassiere, &g->m);
    }
    g->c_barbiere--;
    g->cassiere_libero = false;
    pthread_cond_signal(&g->barbiere);

    pthread_mutex_unlock(&g->m);
}

void libero_cassiere (struct gestore_t *g) {
    pthread_mutex_lock(&g->m);

    g->cassiere_libero = true;
    pthread_cond_signal(&g->cassiere);

    pthread_mutex_unlock(&g->m);
}

void *cliente (void *arg) {
    int thread_id = *((int*) arg);
    while(true) {
        acquisisci_divano(&g);
        acquisisci_barbiere(&g);
        //taglio capelli
        int i = SHAVING_ITERATIONS;
        while(i-- > 0) {};
        acquisisci_cassiere(&g);
        //pagamento
        int i = PAYING_ITERATIONS;
        while(i-- > 0) {};
        libero_cassiere(&g);
        printf("%d> FATTO!\n", thread_id); fflush(stdout);
        // e ricomincio...
        sleep(1);
    }
}

int main(int argc, char* argv[])
{
    pthread_t threads[NUM_CLI];
    int thread_ids[NUM_CLI];

    gestore_init(&g);

    for (int i = 0; i < NUM_CLI; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, cliente, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_CLI; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

