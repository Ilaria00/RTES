#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

sem_t m;
sem_t priv_AB;
sem_t priv_Reset;
int c_AB, c_Reset;          // conta le istanze in esecuzione
int b_AB, b_Reset;          // conta il numero dei bloccati

void myInit(void)
{
  sem_init(&m,0,1);
  sem_init(&priv_AB,0,0);
  sem_init(&priv_Reset,0,0);

  c_AB = c_Reset = b_AB = b_Reset = 0;
}

void StartProcA(void)
{
  sem_wait(&m);

  if (c_Reset || b_Reset) {
    b_AB++;
  }
  else {
    sem_post(&priv_AB);
    c_AB++;
  }
  sem_post(&m);
  sem_wait(&priv_AB);
}

void EndProcA(void)
{
  sem_wait(&m);

  c_AB--;
  if (b_Reset && !c_AB) {
    c_Reset++;
    b_Reset--;
    sem_post(&priv_Reset);
  }

  sem_post(&m);
}


// le procedure di B si comportano in modo identico a quelle di A
void StartProcB(void)
{
  StartProcA();
}

void EndProcB(void)
{
  EndProcA();
}

void StartReset(void)
{
  sem_wait(&m);
  if (c_AB) {
    b_Reset++;
  }
  else {
    sem_post(&priv_Reset);
    c_Reset++;
  }
  sem_post(&m);
  sem_wait(&priv_Reset);
}

void EndReset(void)
{
  sem_wait(&m);

  c_Reset--;
  while (b_AB) {
    sem_post(&priv_AB);
    b_AB--;
    c_AB++;
  }

  sem_post(&m);
}
#endif

/* ------------------------------------------------------------------------ */

/* alla fine di ogni ciclo ogni thread aspetta un po'.
   Cosa succede se tolgo questa nanosleep? 
   di fatto solo i thread di tipo B riescono ad entrare --> starvation!!!!
   (provare per credere)
*/
void pausetta(void)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*1000000;
  nanosleep(&t,NULL);
}

/* ------------------------------------------------------------------------ */


void ProcA(void)
{
  printf("Eseguo A\n");
}
void ProcB(void)
{
  printf("Eseguo B\n");
}

void Reset(void)
{
  printf("Eseguo R\n");
}


void *PA(void *arg)
{
    printf("Inizio A\n");
    StartProcA();
    ProcA();
    EndProcA();
    printf("Fine A\n");
    return 0;
}

void *PB(void *arg)
{
    printf("Inizio B\n");
    StartProcB();
    ProcB();
    EndProcB();
    printf("Fine B\n");
    return 0;
}

void *PR(void *arg)
{
    printf("Inizio R\n");
    StartReset();
    Reset();
    EndReset();
    printf("Fine R\n");
    pausetta();
    return 0;
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv)
{
  pthread_attr_t a;
  pthread_t p;
  
  /* inizializzo il sistema */
  myInit();

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
