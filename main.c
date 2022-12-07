#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
// Testing:
#include <sched.h>

#define NTHREADS 1000
#define PER_THREAD 100000

#define PTR_SIZE sizeof(uintptr_t)

typedef struct NumberRange {
    int start;
    int end;
} NumberRange;

NumberRange *NumberRange_init(int start, int end){
    NumberRange *range = malloc(sizeof(NumberRange));
    range->start = start;
    range->end = end;
    return range;
}

typedef struct LinkedNode{
    int value;
    struct LinkedNode *next;
} LinkedNode;

typedef struct LinkedList{
    LinkedNode *head;
    LinkedNode *tail;
    pthread_mutex_t mutex;
} LinkedList;

LinkedList *LinkedList_init(void){
    LinkedList *l = malloc(sizeof(LinkedList));
    l->head = l->tail = NULL;
    l->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
}

void LinkedList_add(LinkedList *l, int value){
    // fprintf(stderr, "%p, %d\n", l, value);
    pthread_mutex_lock(&l->mutex);
    LinkedNode *node = malloc(sizeof(LinkedNode));
    node->value = value;
    if(!l->head)
        l->head = node;
    if(l->tail)
        l->tail->next = node;
    l->tail = node;
    pthread_mutex_unlock(&l->mutex);
}

// TODO: add Producer consumer
// Search_range creates producer (or is producer) and consumer threads add values to the list

void *search_range(void *arg){
    LinkedList *list = (LinkedList *)(*(uintptr_t *)arg);
    // fprintf(stderr, "List: %p\n", arg);
    NumberRange *range = *(NumberRange **)(arg + PTR_SIZE);
    void *thread_id = (void *)(uintptr_t)(arg + PTR_SIZE * 2);
    for(int i = range->start; i < range->end; i++){
        for(int j = 2; j <= (int)sqrt(i); j++){
            if(i % j == 0)
                goto NOT_PRIME;
        }
        LinkedList_add(list, i);
        NOT_PRIME:;
    }
    return thread_id;
    // refactor to ensure correct order of numbers
}

void print(int n){
    printf("%d, ", n);
}

int main(void){
    printf("Checking for primes from 1-%d\nUsing %d threads\n", NTHREADS*PER_THREAD, NTHREADS);
    pthread_t threads[NTHREADS];
    LinkedList *primes = LinkedList_init();
    for(int i = 0; i < NTHREADS; i++){
        // Data packing :) :) 220 coming in clutch
        void *arg = malloc(PTR_SIZE * 2);
        *(uintptr_t *) arg = (uintptr_t) primes;
        *(uintptr_t *) (arg+PTR_SIZE) = (uintptr_t) NumberRange_init(i*PER_THREAD?:2, (i+1)*PER_THREAD);
        *(int *) (arg+PTR_SIZE * 2) = i;
        fprintf(stderr, "Thread %d created...\n", i);
        pthread_create(&threads[i], NULL, search_range, arg);
    }
    for(int i = 0; i < NTHREADS; i++){
        fprintf(stderr, "Thread %d joined\n", i);
        pthread_join(threads[i], NULL);
    }

    // struct Iterator *iter = ArrayList_getIterator(primes);
    // Iterator_forEach(iter, print);
    // ^ print to console

    // write to file:
    FILE *fptr = fopen("out.txt", "w");
    for(LinkedNode *node = primes->head; node; node = node->next){
        int length = (int)(ceil(log10(node->value)));
        char *x = calloc(length + 2, sizeof(char));
        sprintf(x, "%d", node->value);
        *(x+length) = '\n';
        fwrite(x, sizeof(char), length + 1, fptr);
    }
    return 0;
}