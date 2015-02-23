#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
//#include <semaphore.h>
#define N 4

int buf[N];
int first_occupied_slot = 0;
int first_empty_slot = N;


pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;;

typedef struct {
    int count;
    int pending_posts;
} sem_t;


sem_t sem_prod;
sem_t sem_cons;

void sem_init(sem_t *sem, int new_count) { 
    pthread_mutex_lock(&mut);
    sem->count = new_count;
    sem->pending_posts = new_count;
    pthread_mutex_unlock(&mut);
}

void sem_post(sem_t *sem) { 
    pthread_mutex_lock(&mut);
    sem->count = sem->count + 1;
    sem->pending_posts = sem->pending_posts + 1;
    pthread_mutex_unlock(&mut);
}

void sem_wait(sem_t *sem) {
    printf("In sem_wait...\n");
    int done;
    pthread_mutex_lock(&mut);
    sem_t mysem = *(sem);  // Save the *sem I saw on entry.                    
    sem->count = sem->count - 1;
    if (sem->count < 0) {                                                      
        SLEEP:
        pthread_mutex_unlock(&mut);
                                                                               // Make pending posts = 0? (class notes)
        while (sem->pending_posts <= 0) { printf(""); }                                    // Comparing count or pending_pots?
        
        // When we return from while:  *sem >= mysem 
        // In this case, we are guaranteed that we don't need to wait any more.
        pthread_mutex_lock(&mut);
        
        //if there are no more available, go back to sleep
        if (sem->pending_posts <= 0) { goto SLEEP; }
    }

    //reduce pending_post if there is one (not zero)
    if (sem->pending_posts > 0) { 
            sem->pending_posts = sem->pending_posts - 1;
    }
    else { goto SLEEP; }

    pthread_mutex_unlock(&mut);
    return;
}

void add(int val) {
    if (first_empty_slot >= N) { first_empty_slot = 0; }
    buf[first_empty_slot] = val;
    first_empty_slot++;
    if (first_empty_slot >= N) { first_empty_slot = 0; }
    return;
}

int rem() {
    if (first_occupied_slot >= N) { first_occupied_slot = 0; }
    int val = buf[first_occupied_slot];
    first_occupied_slot++;
    if (first_occupied_slot >= N) { first_occupied_slot = 0; }
    return val;
}

void *consumer(void *arg) {
    int i ;
    sem_wait(&sem_cons);
    i = rem();
    printf("CONSUMER REMOVED %d\n", i);
    sleep( rand() % 5 );
    sem_post(&sem_prod);
}

void *producer(void *arg) {
    int i;
    for(i = 0; i < 4; i++) {
        sem_wait(&sem_prod);
        printf("ADDING %d!!!\n",i);
        add(i);
        sleep( rand() % 5 );
        sem_post(&sem_cons);
    }
}

int main() {

    sem_init(&sem_prod, N);
    sem_init(&sem_cons, 0);

    pthread_t producer_t;
    pthread_t consumer_t;

    pthread_create(&producer_t, NULL, producer, NULL);
    pthread_create(&consumer_t, NULL, consumer, NULL);

    pthread_join(producer_t, NULL);
    pthread_join(consumer_t, NULL);

    //How do we do this actually?
    int i = 0;

    return 0;
}