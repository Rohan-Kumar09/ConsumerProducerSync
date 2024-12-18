/* 
compile: gcc RKprog4.c -o RKprog4 -lpthread
run: ./RKprog4 30 5 5
comment: 30 is time in seconds, 5 is number of producers, 5 is number of consumers
*/

# include <pthread.h> /* for pthread functions */
# include <stdio.h>
# include <unistd.h> /* for sleep() */
# include <stdlib.h> /* for rand() */
# include <semaphore.h> /* for semaphores */

typedef int buffer_item; /* to distinguish regular integers from buffer_item integers */
# define BUFFER_SIZE 5 /* changable buffer size */
# define EMPTY -1 /* -1 = empty slot */

buffer_item buffer[BUFFER_SIZE];
int in = 0; /* insert index */
int out = 0; /* remove index */

pthread_mutex_t mutex; /* mutex lock: one accessor allowed */
sem_t empty; /* available slots = empty slots */
sem_t full; /* taken slots = full slots */

void* producer(void *param); /* produce item calls insert_item */
void* consumer(void *param); /* consume item calls remove_item */
int insert_item(buffer_item item); /* insert item into buffer at IN */
int remove_item(buffer_item *item); /* remove item from buffer at OUT */

int main(int argc, char *argv[]){
    /* get arguments */
    if (argc != 4){
        printf("Usage: %s <time> <numOfProducers> <numOfConsumers>\n", argv[0]);
        exit(1);
    }
    int time = atoi(argv[1]);
    int numOfProducers = atoi(argv[2]);
    int numOfConsumers = atoi(argv[3]);

    int localProducer_tid[numOfProducers]; /* local producer id */
    int localConsumer_tid[numOfConsumers]; /* local consumer id */

    pthread_t producer_tid[numOfProducers]; /* producer thread id */
    pthread_t consumer_tid[numOfConsumers]; /* consumer thread id */
    pthread_attr_t attr; /* set of thread attributes */
    pthread_attr_init(&attr); /* get default attributes */

    /* initialize buffer, set all slots to available/empty */
    int i;
    for (i = 0; i < BUFFER_SIZE; i++){
        buffer[i] = EMPTY;
    }
    sem_init(&empty, 0, BUFFER_SIZE); /* empty = buffer_size */
    sem_init(&full, 0, 0); /* full = 0 */
    pthread_mutex_init(&mutex, NULL); /* initialize mutex */
    printf("Main thread beginning\n");
    /* Create producer threads */
    for (i = 0; i < numOfProducers; i++){
        localProducer_tid[i] = i;
        pthread_create(&producer_tid[i], &attr, producer, (void *) &localProducer_tid[i]);
        printf("Creating producer thread with id %lu\n", producer_tid[i]);
    }
    /* Create consumer threads */
    for (i = 0; i < numOfConsumers; i++){
        localConsumer_tid[i] = i;
        pthread_create(&consumer_tid[i], &attr, consumer, (void *) &localConsumer_tid[i]);
        printf("Creating consumer thread with id %lu\n", consumer_tid[i]);
    }
    /* Sleep */
    printf("Main thread sleeping for %i seconds\n", time);
    sleep(time);
    /* Main thread exits*/
    printf("Main thread exiting\n");
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    exit(0);
    return 0;
}

void* producer(void *tid) {
    int local_tid = *((int *) tid);
    buffer_item item;
    while(1) {
        int num = (rand() % 4) + 1; /* random number from 1 to 4 */
        printf("Producer thread %i sleeping for %d seconds\n", local_tid, num);
        sleep(num); /* sleep for random amount of time */

        item = (rand() % 50) + 1; /* generate random number */
        sem_wait(&empty); /* only add if there is/are space(s) */
        pthread_mutex_lock(&mutex); /* only add if mutex is acquired */
        insert_item(item);
        pthread_mutex_unlock(&mutex);
        sem_post(&full); /* signal that there is an/are item(s) in the buffer */
    }
}

void* consumer(void *tid) {
    int local_tid = *((int *) tid);
    buffer_item item;
    while(1) {
        int num = (rand() % 4) + 1; /* random number from 1 to 4 */
        printf("Consumer thread %i sleeping for %d seconds\n", local_tid, num);
        sleep(num); /* sleep for random amount of time */

        sem_wait(&full); /* only remove if there is an/are item(s) */
        pthread_mutex_lock(&mutex); /* only remove if mutex is acquired */
        remove_item(&item);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty); /* signal that there is an/are empty space(s) in the buffer */
    }
}

int insert_item(buffer_item item){
    /* insert item into buffer */
    // if (buffer[in] == EMPTY){ /* unnessasary check to make sure buffer slot is empty (semaphores do this job in producer)*/
        printf("insert_item inserted item %d at position %d\n", item, in);
        buffer[in] = item; /* insert item */
        in = (in + 1) % BUFFER_SIZE; /* increment in */
        /* print buffer */
        int i;
        printf("Buffer = {");
        for (i = 0; i < BUFFER_SIZE; i++){
            if (i != BUFFER_SIZE - 1){
                printf("%d, ", buffer[i]);
            }
            else {
                printf("%d", buffer[i]);
            }
        }
        printf("}\n");
        return 0; /* success */
    // }
    // return -1; /* fail */
}

int remove_item(buffer_item *item){
    /* remove item from buffer */
    // if (buffer[out] != EMPTY){ /* unnessasary check to make sure buffer slot is full (semaphores do this job in consumer)*/
        *item = buffer[out]; /* item to be removed */
        printf("remove_item removed item %d from position %d\n", *item, out);
        buffer[out] = EMPTY; /* set slot to empty */
        out = (out + 1) % BUFFER_SIZE; /* increment out */
        /* print buffer */
        int i;
        printf("Buffer = {");
        for (i = 0; i < BUFFER_SIZE; i++){
            if (i != BUFFER_SIZE - 1){
                printf("%d, ", buffer[i]);
            }
            else {
                printf("%d", buffer[i]);
            }
        }
        printf("}\n");
        return 0; /* success */
    // }
    // return -1; /* fail */
}