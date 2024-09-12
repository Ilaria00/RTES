#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define do_something() usleep(rand() % 1000 + 1000)

struct gestore_t {
    sem_t mutex;
    sem_t reset_s;
    sem_t procAoB_s;
    
    int reset_attesa, AoB_attesa;
    int reset_esecuzione, AoB_esecuzione;
} gestore;

pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
int id_A = 0;
int id_B = 0;
int id_R = 0;

void gestore_init (struct gestore_t *g) {
    sem_init(&g->mutex, 0, 1);
    sem_init(&g->reset_s, 0, 0);
    sem_init(&g->procAoB_s, 0, 0);
    g->reset_attesa = g->AoB_attesa = 0;
    g->reset_esecuzione = g->AoB_esecuzione = 0;
}

void StartProcAorProcB (struct gestore_t *g){
    sem_wait(&g->mutex);
    if (g->reset_esecuzione || g->AoB_esecuzione) {
        g->AoB_attesa++;
        sem_post(&g->mutex);
        sem_wait(&g->procAoB_s);
        g->AoB_attesa--;
    } else {
        g->AoB_esecuzione++;
        sem_post(&g->mutex);
    }
}

void EndProcAorProcB (struct gestore_t *g) {
    sem_wait(&g->mutex);
    g->AoB_esecuzione--;
    if (g->reset_attesa) {
        g->reset_esecuzione++; 
        sem_post(&g->reset_s);
    } else if (g->AoB_attesa) {
        g->AoB_esecuzione++; 
        sem_post(&g->procAoB_s);
    } else {
        sem_post(&g->mutex);
    }
}

void StartReset (struct gestore_t *g) {
    sem_wait(&g->mutex);
    if (g->AoB_esecuzione || g->reset_esecuzione) {
        g->reset_attesa++;
        sem_post(&g->mutex);
        sem_wait(&g->reset_s);
        g->reset_attesa--;
    } else {
        g->reset_esecuzione++;
        sem_post(&g->mutex);
    }
}

void EndReset (struct gestore_t *g) {
    sem_wait(&g->mutex);
    g->reset_esecuzione--;
    if (g->reset_attesa) {
        g->reset_esecuzione++; 
        sem_post(&g->reset_s);
    } else if (g->AoB_attesa) {
        g->AoB_esecuzione++; 
        sem_post(&g->procAoB_s);
    } else {
        sem_post(&g->mutex);
    }
}

void pausetta(void) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t, NULL);
}

void ProcA(void) {
    printf("Eseguo A-%d\n", id_A);
}

void ProcB(void) {
    printf("Eseguo B-%d\n", id_B);
}

void Reset(void) {
    printf("Eseguo R-%d\n", id_R);
}

void *PA(void *arg) {
    pthread_mutex_lock(&id_mutex);
    id_A++;
    int local_id = id_A;  // Copia locale per evitare race condition
    pthread_mutex_unlock(&id_mutex);

    printf("Inizio A-%d\n", local_id);
    StartProcAorProcB(&gestore);
    ProcA();
    EndProcAorProcB(&gestore);
    printf("Fine A-%d\n", local_id);
    return 0;
}

void *PB(void *arg) {
    pthread_mutex_lock(&id_mutex);
    id_B++;
    int local_id = id_B;
    pthread_mutex_unlock(&id_mutex);

    printf("Inizio B-%d\n", local_id);
    StartProcAorProcB(&gestore);
    ProcB();
    EndProcAorProcB(&gestore);
    printf("Fine B-%d\n", local_id);
    return 0;
}

void *PR(void *arg) {
    pthread_mutex_lock(&id_mutex);
    id_R++;
    int local_id = id_R;
    pthread_mutex_unlock(&id_mutex);

    printf("Inizio R-%d\n", local_id);
    StartReset(&gestore);
    Reset();
    EndReset(&gestore);
    printf("Fine R-%d\n", local_id);
    pausetta();
    return 0;
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    pthread_t threads[3];
    
    /* Inizializzo il sistema */
    gestore_init(&gestore);

    /* Inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    /* Creo i thread */
    pthread_create(&threads[0], NULL, PA, NULL);
    pthread_create(&threads[1], NULL, PB, NULL);
    pthread_create(&threads[2], NULL, PR, NULL);

    /* Attendo la terminazione dei thread */
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
