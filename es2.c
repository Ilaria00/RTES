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

void gestore_init (struct gestore_t *g) {

    sem_init(&g->mutex, 0, 1);

    sem_init(&g->reset_s, 0, 0);
    sem_init(&g->procAoB_s, 0, 0);

    g->reset_attesa = g->AoB_attesa = 0;
    g->reset_esecuzione = g->AoB_esecuzione = 0;
}

void StartProcAorProcB (struct gestore_t *g){

    sem_wait(&g->mutex);

    //se ci sono processi reset in esecuzione mi blocco
    if (g->reset_esecuzione) {
        g->AoB_attesa++;
        sem_post(&g->mutex);
        sem_wait(&g->procAoB_s);
        g->AoB_attesa--;
    }

    //altrimenti eseguo
    g->AoB_esecuzione++;
    sem_post(&g->mutex);
}

void EndProcAorProcB (struct gestore_t *g) {

    sem_wait(&g->mutex);

    //se ci sono processi reset in attesa li sveglio
    if (g->reset_attesa) {
        sem_post(&g->reset_s);
    }
    else {
        sem_post(&g->mutex);
    }
   
}

void StartReset (struct gestore_t *g) {

    sem_wait(&g->mutex);

    //se ci sono processi AoB in esecuzione in questo momento mi blocco
    if (g->AoB_esecuzione) {
        g->reset_attesa++;
        sem_post(&g->mutex);
        sem_wait(&g->reset_s);
        g->reset_attesa--;
    }

    //altrimenti eseguo
    g->reset_esecuzione++;
    sem_post(&g->mutex);
}

void EndReset (struct gestore_t *g) {

    sem_wait(&g->mutex);

    if (g->reset_attesa) {
        sem_post(&g->reset_s);
    }
    else if (g->AoB_attesa) {
        sem_post(&g->procAoB_s);
    }
    else {
        sem_post(&g->mutex);
    }
}

void ProcA(int thread_idx)
{
    StartProcAorProcB(&gestore);

    printf("< %d-A \n", thread_idx); 
    do_something();
    printf("A-%d > \n", thread_idx);

    EndProcAorProcB(&gestore);
}

void ProcB(int thread_idx)
{
    StartProcAorProcB(&gestore);

    printf("< %d-B \n", thread_idx); 
    do_something();
    printf("B-%d \n>", thread_idx);

    EndProcAorProcB(&gestore);
}

void Reset(int thread_idx)
{
    StartReset(&gestore);

    printf("< %d-R \n", thread_idx); 
    do_something();
    printf("R-%d \n>", thread_idx);

    EndReset(&gestore);
}

void* thread_body(void* arg)
{
    int thread_idx = *((int*) arg);
    while (1)
    {
        int r = rand();

        //my_printf("|%d->%s|", thread_idx, (r%3 ==0  ? "A" : (r%3==1 ? "B" : "R")));

        if (r % 3 == 0) ProcA(thread_idx);
        else if (r % 3 == 1) ProcB(thread_idx);
        else if (r % 3 == 2) Reset(thread_idx);
    }
}


int main(int argc, char* argv[])
{
    srand(time(NULL));

    const int k_num_threads = 10;
    pthread_t my_threads[k_num_threads];

    gestore_init(&gestore);

    // stores thread ids in an array for debugging
    int thread_ids[k_num_threads];
    for (int i = 0; i < k_num_threads; i++)
        thread_ids[i] = i;

    // creates and starts threads
    for (int i = 0; i < k_num_threads; i++)
    {
        if (pthread_create(&my_threads[i], NULL, thread_body, (void*) &thread_ids[i]) != 0)
        {
            perror("pthread_create() error\n");
            return 1;
        }
    }

    sleep(60);

    return 0;
}
