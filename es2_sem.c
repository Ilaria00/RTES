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

    //se ci sono processi reset o AoB in esecuzione mi blocco
    if (g->reset_esecuzione || g->AoB_esecuzione) {
        g->AoB_attesa++;
        sem_post(&g->mutex);
        sem_wait(&g->procAoB_s);
        g->AoB_attesa--;
    }
    else {
        g->AoB_esecuzione++;
    }
    sem_post(&g->mutex);
}

void EndProcAorProcB (struct gestore_t *g) {

    sem_wait(&g->mutex);
    g->AoB_esecuzione--;
    //se ci sono processi reset in attesa li sveglio
    if (g->reset_attesa) {
        g->reset_esecuzione++; 
        sem_post(&g->reset_s);
    }
    else if (g->AoB_attesa) {
        g->AoB_esecuzione++; 
        sem_post(&g->procAoB_s);
    }
    else {
        sem_post(&g->mutex);
    }
   
}

void StartReset (struct gestore_t *g) {

    sem_wait(&g->mutex);

    //se ci sono processi AoB in esecuzione in questo momento mi blocco
    if (g->AoB_esecuzione || g->reset_esecuzione) {
        g->reset_attesa++;
        sem_post(&g->mutex);
        sem_wait(&g->reset_s);
        g->reset_attesa--;
    }
    else {
        g->reset_esecuzione++;
    }
    sem_post(&g->mutex);
}

void EndReset (struct gestore_t *g) {

    sem_wait(&g->mutex);

    g->reset_esecuzione--;

    if (g->reset_attesa) {
        g->reset_esecuzione++; 
        sem_post(&g->reset_s);
    }
    else if (g->AoB_attesa) {
        g->AoB_esecuzione++; 
        sem_post(&g->procAoB_s);
    }
    else {
        sem_post(&g->mutex);
    }
}

void pausetta(void)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*1000000;
  nanosleep(&t,NULL);
}

int id_A = 0;
int id_B = 0;
int id_R = 0;


void ProcA(void)
{
  printf("Eseguo A-%d", id_A);
}

void ProcB(void)
{
  printf("Eseguo B-%d", id_B);
}

void Reset(void)
{
  printf("Eseguo R-%d", id_R);
}


void *PA(void *arg)
{
    id_A++;
    printf("Inizio A-%d", id_A);
    StartProcAorProcB(&gestore);
    ProcA();
    EndProcAorProcB(&gestore);
    printf("Fine A-%d", id_A);
    return 0;
}

void *PB(void *arg)
{
    id_B++;
    printf("Inizio B-%d", id_B);
    StartProcAorProcB(&gestore);
    ProcB();
    EndProcAorProcB(&gestore);
    printf("Fine B-%d", id_B);
    return 0;
}

void *PR(void *arg)
{
    id_R++;
    printf("Inizio R-%d", id_R);
    StartReset(&gestore);
    Reset();
    EndReset(&gestore);
    printf("Fine R-%d", id_R);
    pausetta();
    return 0;
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv)
{
  pthread_attr_t a;
  pthread_t p;
  
  /* inizializzo il sistema */
  gestore_init(&gestore);

  /* inizializzo i numeri casuali, usati nella funzione pausetta */
  srand(555);

  pthread_attr_init(&a);

  /* non ho voglia di scrivere 10000 volte join! */
  pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

  pthread_create(&p, &a, PA, NULL);
  pthread_create(&p, &a, PB, NULL);
  pthread_create(&p, &a, PR, NULL);

  pthread_attr_destroy(&a);

  /* aspetto 10 secondi prima di terminare tutti quanti */
  sleep(5);

  return 0;
}
