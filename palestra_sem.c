#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define N 5 //numero tipologie di attrezzi
/*bilanciere
cyclette
tapis roulant
panca
pesi*/

#define M 4 //numero di attrezzi per tipologia

#define P 10 //persone in palestra

#define E 3 //esercizi che fa ciascuna persona

struct semaforoprivato_t {
    sem_t s;
    int c_attesa;
    int c_in_uso;
    int c_prenotati;
};

struct palestra_t {
    sem_t mutex;

    struct semaforoprivato_t s_uso_attrezzo[N];

    int prenotazioni[P];
    /*prenotazioni[0] = 5 se la persona 0 ha prenotato l'attrezzo 5*/

} palestra;

void semaforoprivato_init (struct semaforoprivato_t *s) {
    sem_init(&s->s, 0, 0);
    s->c_attesa = 0; //thread che si bloccano sul semaforo
    s->c_in_uso = 0; //numero di attrezzi in uso 
    s->c_prenotati = 0; //numero attrezzi prenotati
}

void init_palestra (struct palestra_t *p) {
    sem_init(&p->mutex, 0, 1);

    for (int i=0; i<N; i++) {
    semaforoprivato_init(&p->s_uso_attrezzo[i]);
    }
}

void usaattrezzo (struct palestra_t *p, int numeropersona, int tipoattrezzo) {
    sem_wait(&p->mutex);
    /*se si tratta di un attrezzo che ho precedentemente prenotato ok*/
    if (p->prenotazioni[numeropersona] == tipoattrezzo) {
        printf("La persona %d sta utilizzando l'attrezzo %d che aveva precedentemente prenotato\n", numeropersona, tipoattrezzo);
        p->s_uso_attrezzo[tipoattrezzo].c_in_uso++;
        p->s_uso_attrezzo[tipoattrezzo].c_prenotati--;
        p->prenotazioni[numeropersona] = -1; //cancello al prenotazione siccome lo sto usando
    }

    /*se si tratta di un attrezzo che non ho prenotato ma è libero ok*/
    else if ((p->s_uso_attrezzo[tipoattrezzo].c_in_uso + p->s_uso_attrezzo[tipoattrezzo].c_prenotati) < M){
        printf("La persona %d sta utilizzando l'attrezzo %d che non aveva prenotato ma che ha trovato libero\n", numeropersona, tipoattrezzo);
        p->s_uso_attrezzo[tipoattrezzo].c_in_uso++;
    }

    /*se si tratta di un attrezzo occupato aspetto*/
    else {
        printf("La persona %d attende di usare l'attrezzo %d ma non l'aveva prenotato e non e' libero\n", numeropersona, tipoattrezzo);
        p->s_uso_attrezzo[tipoattrezzo].c_attesa++;
        sem_post(&p->mutex);
        sem_wait(&p->s_uso_attrezzo[tipoattrezzo].s);
        p->s_uso_attrezzo[tipoattrezzo].c_attesa--;
        p->s_uso_attrezzo[tipoattrezzo].c_in_uso++;
    }
    sem_post(&p->mutex);
}

void prenota (struct palestra_t *p, int numeropersona, int tipoattrezzo) {
    sem_wait(&p->mutex);
    /*se l'attrezzo da prenotare è libero (non in uso e non prenotato) lo prenoto*/
    if ((p->s_uso_attrezzo[tipoattrezzo].c_in_uso + p->s_uso_attrezzo[tipoattrezzo].c_prenotati) < M) {
        printf("La persona %d ha prenotato l'attrezzo %d\n", numeropersona, tipoattrezzo);
        p->prenotazioni[numeropersona] = tipoattrezzo;
        p->s_uso_attrezzo[tipoattrezzo].c_prenotati++;
    }
    else {
    /*altrimenti non faccio la prenotazione*/
        printf("La persona %d non ha effettuato la prenotazione perche' l'attrezzo %d non e' libero\n", numeropersona, tipoattrezzo);
    }
    sem_post(&p->mutex);
}

void fineuso (struct palestra_t *p, int numeropersona, int tipoattrezzo) {
    sem_wait(&p->mutex);
    /**/
    p->s_uso_attrezzo[tipoattrezzo].c_in_uso--;
    printf("La persona %d ha terminato di usare l'attrezzo %d\n", numeropersona, tipoattrezzo);
    if (p->s_uso_attrezzo[tipoattrezzo].c_attesa) {
        sem_post(&p->s_uso_attrezzo[tipoattrezzo].s);
    }
    sem_post(&p->mutex);
}

void *persona (void *arg) {
    int numeropersona = *((int*)arg);
    int attrezzocorrente = rand()%N;
    int prossimoattrezzo = rand()%N;
    for (int i=E; i>0; i--) {
        usaattrezzo(&palestra, numeropersona, attrezzocorrente);
        if (i != 0) {
            prenota(&palestra, numeropersona, prossimoattrezzo);
        }
        fineuso(&palestra, numeropersona, attrezzocorrente);
        if (i != 0) {
            attrezzocorrente = prossimoattrezzo;
            prossimoattrezzo = rand()%N;
        }
    }
}

int main (int argc, char *argv[]) {
    pthread_t tpersona[P];
    int numeri_persone[P];
    init_palestra(&palestra);

    for (int i=0; i<P; i++){
        numeri_persone[i] = i;
        pthread_create(&tpersona[i], NULL, persona, &numeri_persone[i]);
    }

    for (int i=0; i<P; i++) {
        pthread_join(tpersona[i], NULL);
    }
    
    return 0;
}
