#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct bandierine_t {
    sem_t mutex;

    sem_t giocatore_s;
    sem_t giudice_s;
    sem_t giudice_fine_s;

    int b_giocatore; //mi dice quanti giocatori sono in attesa
    int num_giocatori;
    int giudice_arrivato;
    int vincitore; //-1
    bool gara_conclusa;
    
} bandierine;

void init_bandierine(struct bandierine_t *b) {
    sem_init(&b->mutex, 0, 1);

    sem_init(&b->giocatore_s, 0, 0);
    sem_init(&b->giudice_s, 0, 0);
    sem_init(&b->giudice_fine_s, 0, 0);

    b->b_giocatore= 0;
    b->num_giocatori = 0;
    b->giudice_arrivato = 0;
    b->vincitore = -1;
    b->gara_conclusa = false;
}

void attendi_giocatori (struct bandierine_t *b) {
    
    sem_wait(&b->mutex);
    b->giudice_arrivato = 1;
    printf("Il giudice e' arrivato\n");
    /*il giudice si blocca se non sono arrivati entambi i giocatori*/
    if (b->b_giocatore < 2) {
        sem_post(&b->mutex);
        printf("Giudice: aspetto i giocatori");
        sem_wait(&b->giudice_s);
    }
    sem_post(&b->mutex);
}

void via(struct bandierine_t *b) {

    sem_wait(&b->mutex);
    printf("Giudice: via\n");
    for (int i=0; i < 2; i++) {
        sem_post(&b->giocatore_s);
    }
    b->b_giocatore = 0;
    sem_post(&b->mutex);
}

void attendi_il_via (struct bandierine_t *b, int n) {

    sem_wait(&b->mutex);
    b->b_giocatore++;
    printf("Il giocatore %d attende il via\n", n);
    /*se sono il secondo giocatore e il giudice è arrivato sveglio il giudice*/
    if (b->b_giocatore == 2 && b->giudice_arrivato == 1) {
        sem_post(&b->giudice_s);
    }
    sem_post(&b->mutex);
    sem_wait(&b->giocatore_s);
}

int bandierina_presa (struct bandierine_t *b, int n) {
    sem_wait(&b->mutex);
    b->num_giocatori++;
    int r;
    //se è il primo giocatore allora torna 1
    if (b->num_giocatori == 1){
        r = 1;
        printf("Il giocatore %d ha preso la bandierina\n", n);
    }
    else {
        r = 0;
    }
    sem_post(&b->mutex);
    return r;
}

int sono_salvo (struct bandierine_t *b, int n) {
    sem_wait(&b->mutex);
    int r;
    if (b->vincitore == -1) {
        printf("Il giocatore %d e' salvo\n", n);
        b->vincitore = n;
        r = 1;
        b->gara_conclusa = true;
        sem_post(&b->giudice_fine_s);
    }
    else {
        r = 0;
    }
    sem_post(&b->mutex);
    return r;
}

int ti_ho_preso (struct bandierine_t *b, int n) {
    sem_wait(&b->mutex);
    int r;
    if (b->vincitore == -1) {
        printf("Il giocatore %d ha preso l'avversario\n", n);
        b->vincitore = n;
        r = 1;
        b->gara_conclusa = true;
        sem_post(&b->giudice_fine_s);
    }
    else {
        r = 0;
    }
    sem_post(&b->mutex);
    return r;
}

int risultato_gioco (struct bandierine_t *b) {
    sem_wait(&b->mutex);

    //se i giocatori non hanno ancora concluso la gara mi blocco
    if (b->gara_conclusa == false) {
        sem_post(&b->mutex);
        sem_wait(&b->giudice_fine_s);
    }

    sem_post(&b->mutex);
    return b->vincitore;
}

/*MAIN*/

void *giocatore (void *arg) {
    int numerogiocatore = *((int*)arg);
    attendi_il_via(&bandierine, numerogiocatore);
    //corri e tenta di prendere la bandierina
    if(bandierina_presa(&bandierine, numerogiocatore)) {
        //corri alla base
        if(sono_salvo(&bandierine, numerogiocatore)) {
            printf("Salvo\n");
        }
    }
    else {
        //cerca di prendere l'altro giocatore
        if(ti_ho_preso(&bandierine, numerogiocatore)){
            printf("Preso\n");
        }
    }
    return 0;
}

void *giudice (void *arg) {
    attendi_giocatori(&bandierine);
    //pronti, attenti...
    via(&bandierine);
    printf("Il vincitore e': %d\n", risultato_gioco(&bandierine));
    return 0;
}

int main (int argc, char* argv[]) {
    pthread_t tgiocatore1, tgiocatore2, tgiudice;

    init_bandierine(&bandierine);

    int n1 = 0, n2 = 1;

    pthread_create(&tgiocatore1, NULL, giocatore, &n1);
    pthread_create(&tgiocatore2, NULL, giocatore, &n2);
    pthread_create(&tgiudice, NULL, giudice, NULL);

    pthread_join(tgiocatore1, NULL);
    pthread_join(tgiocatore2, NULL);
    pthread_join(tgiudice, NULL);

    return 0;
}
