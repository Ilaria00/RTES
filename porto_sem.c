#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define N 10 //numero posti barche
#define B 13 //numero barche

struct porto_t {
    sem_t mutex;

    sem_t barca_entra_s;
    sem_t barca_esce_s; 

    int posti_occupati;
    int barche_bloccate_in_entrata;
    int barche_bloccate_in_uscita;
    int attraversamenti;
} porto;

void init_porto (struct porto_t *porto) {
    
    sem_init(&porto->mutex, 0, 1);

    sem_init(&porto->barca_entra_s, 0, 0);
    sem_init(&porto->barca_esce_s, 0, 0);

    porto->posti_occupati = 0;
    porto->barche_bloccate_in_entrata = 0;
    porto->barche_bloccate_in_uscita = 0;
    porto->attraversamenti = 0;
}

void entrata_richiesta (struct porto_t *porto, int n) {
    sem_wait(&porto->mutex);
    printf("Barca %d chiede di entrare\n", n);
    if (porto->posti_occupati >= N || porto->attraversamenti >= 2) {
        //se i posti sono tutti occupati mi blocco
        printf("Barca %d in attesa per entrare - posti occupati: %d - attraversamenti: %d\n", n, porto->posti_occupati, porto->attraversamenti);
        porto->barche_bloccate_in_entrata++;
        sem_post(&porto->mutex);
        sem_wait(&porto->barca_entra_s);
        porto->barche_bloccate_in_entrata--;
    }
    porto->attraversamenti++;
    printf("Barca %d sta attraversando in entrata l'imboccatura\n", n);
    sem_post(&porto->mutex);
}

void entrata_ok (struct porto_t *porto, int n) {
    sem_wait(&porto->mutex);
    porto->posti_occupati++;
    porto->attraversamenti--;
    printf("Barca %d entrata\n", n);
    if (porto->attraversamenti < 2) {
        if (porto->barche_bloccate_in_uscita){
            sem_post(&porto->barca_esce_s);
        }
        else if (porto->barche_bloccate_in_entrata) {
            sem_post(&porto->barca_entra_s);
        }
    }
    sem_post(&porto->mutex);
}

void uscita_richiesta (struct porto_t *porto, int n) {
    sem_wait(&porto->mutex);
    printf("Barca %d chiede di uscire\n", n);
    if (porto->attraversamenti >= 2) {
        printf("Barca %d in attesa di uscire - attraversmenti: %d\n", n, porto->attraversamenti);
        porto->barche_bloccate_in_uscita++;
        sem_post(&porto->mutex);
        sem_wait(&porto->barca_esce_s);
        porto->barche_bloccate_in_uscita--;
    }
    porto->attraversamenti++;
    printf("Barca %d sta attraversando in uscita l'imboccatura\n", n);
    sem_post(&porto->mutex);
}

void uscita_ok (struct porto_t *porto, int n) {
    sem_wait(&porto->mutex);
    //libero un posto quindi sblocco una barca in attesa di entrare
    porto->posti_occupati--;
    porto->attraversamenti--;
    printf("Barca %d uscita\n", n);
    if (porto->attraversamenti < 2) {
        if (porto->barche_bloccate_in_uscita){
            sem_post(&porto->barca_esce_s);
        }
        else if (porto->barche_bloccate_in_entrata) {
            sem_post(&porto->barca_entra_s);
        }
    }
    sem_post(&porto->mutex);
}

void *barca (void *arg) { //sono B
    int numerobarca = *((int*)arg);
    //arriva all'imboccatura del porto
    entrata_richiesta(&porto, numerobarca); //bloccante
    //transito imboccatura
    sleep(1);
    entrata_ok(&porto, numerobarca); //non bloccante
    //staziona dentro il porto
    uscita_richiesta(&porto, numerobarca); //bloccante
    //transito imboccatura
    sleep(1);
    uscita_ok(&porto, numerobarca); //non bloccante
    //vai ad altro porto
}

int main (int argc, char*argv[]) {
    
    pthread_t tbarca[B];
    int numeri_barca[B];
    init_porto(&porto);

    for (int i=0; i<B; i++) {
        numeri_barca[i] = i+1;
        pthread_create(&tbarca[i], NULL, barca, &numeri_barca[i]);
    }

    for (int i=0; i<B; i++) {
        pthread_join(tbarca[i], NULL);
    }

    return 0;
}
