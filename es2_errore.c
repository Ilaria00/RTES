
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>


#define my_printf(x, ...) printf(x, ##__VA_ARGS__); fflush(stdout)
#define do_something() usleep(rand() % 1000 + 1000)


struct handler_t {
    sem_t mutex;

    sem_t reset_s;
    sem_t procAoB_s;
    
    int reset_attesa, AoB_attesa;
    int reset_esecuzione, AoB_esecuzione;
} g_handler;


void handler_init(struct handler_t* handler)
{
    sem_init(&handler->mutex, 0, 1);

    sem_init(&handler->reset_s, 0, 0);
    sem_init(&handler->procAoB_s, 0, 0);

    handler->reset_attesa = handler->AoB_attesa = 0;
    handler->reset_esecuzione = handler->AoB_esecuzione = 0;
}

// ------------------------------------------------------------------------------------------------

void StartProcAOrB()
{
    sem_wait(&g_handler.mutex);

    //se ci sono processi reset o AoB in esecuzione mi blocco
    if (g_handler.reset_esecuzione || g_handler.AoB_esecuzione) {
        g_handler.AoB_attesa++;
        sem_post(&g_handler.mutex);
        sem_wait(&g_handler.procAoB_s);
        g_handler.AoB_attesa--;
    }
    else {
        g_handler.AoB_esecuzione++;
    }
    sem_post(&g_handler.mutex);
}

void EndProcAOrB()
{
    sem_wait(&g_handler.mutex);
    g_handler.AoB_esecuzione--;
    //se ci sono processi reset in attesa li sveglio
    if (g_handler.reset_attesa) {
        g_handler.reset_esecuzione++; 
        sem_post(&g_handler.reset_s);
    }
    else if (g_handler.AoB_attesa) {
        g_handler.AoB_esecuzione++; 
        sem_post(&g_handler.procAoB_s);
    }
    //facciamo un errore di proposito
   /* else {
        sem_post(&g_handler.mutex);
    }*/
}

void StartReset()
{
    sem_wait(&g_handler.mutex);

    //se ci sono processi AoB in esecuzione in questo momento mi blocco
    if (g_handler.AoB_esecuzione || g_handler.reset_esecuzione) {
        g_handler.reset_attesa++;
        sem_post(&g_handler.mutex);
        sem_wait(&g_handler.reset_s);
        g_handler.reset_attesa--;
    }
    else {
        g_handler.reset_esecuzione++;
    }
    sem_post(&g_handler.mutex);
}

void EndReset()
{
     sem_wait(&g_handler.mutex);

    g_handler.reset_esecuzione--;

    if (g_handler.reset_attesa) {
        g_handler.reset_esecuzione++; 
        sem_post(&g_handler.reset_s);
    }
    else if (g_handler.AoB_attesa) {
        g_handler.AoB_esecuzione++; 
        sem_post(&g_handler.procAoB_s);
    }
    else {
        sem_post(&g_handler.mutex);
    }
}

// ------------------------------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------------------------------

void ProcA(int thread_idx)
{
    StartProcAOrB();

    my_printf("<%dA", thread_idx); do_something(); my_printf("A%d>", thread_idx);

    EndProcAOrB();
}

void ProcB(int thread_idx)
{
    StartProcAOrB();

    my_printf("<%dB", thread_idx); do_something(); my_printf("B%d>", thread_idx);

    EndProcAOrB();
}

void Reset(int thread_idx)
{
    StartReset();

    my_printf("<%dR", thread_idx); do_something(); my_printf("R%d>", thread_idx);

    EndReset();
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

    handler_init(&g_handler);

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
}
