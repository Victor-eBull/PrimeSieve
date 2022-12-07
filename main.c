#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
// Testing:
#include <sched.h>

#define NTHREADS 1000
#define PER_THREAD 100000

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

// typedef struct ArrayList{
//     int *list;
//     size_t capacity;
//     size_t size;
//     pthread_mutex_t mutex;
// } ArrayList;

// ArrayList *ArrayList_init(int capacity){
//     ArrayList *l = malloc(sizeof(ArrayList));
//     l->list = calloc(capacity+1, sizeof(int));
//     l->capacity = capacity;
//     l->size = 0;
//     l->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
//     return l;
// }

// void ArrayList_add(ArrayList *list, int value){
//     pthread_mutex_lock(&list->mutex);
//     int s = list->size;
//     int c = list->capacity;
//     if(s >= c-1){
//         c *= 2;
//         list->list = (int *)realloc(list->list, list->capacity + 1);
//     }
//     ++s;
//     int ns = list->size;
//     int nc = list->capacity;
//     *(list->list + list->size) = value;
//     // list->list[list->size] = value;
//     int n_s = list->size;
//     int n_c = list->capacity;
//     pthread_mutex_unlock(&list->mutex);
// }

// void ArrayList_add(ArrayList *list, int value){
//     pthread_mutex_lock(&list->mutex);
//     int s = list->size;
//     int c = list->capacity;
//     int *set = NULL;
//     if(s >= c-1){
//         c *= 2;
//         set = (int *)realloc(list->list, c + 1);
//     }
//     ++s;
//     list->size = s;
//     list->capacity = c;
//     (set?:list->list)[s] = value;
//     pthread_mutex_unlock(&list->mutex);
// }

struct Iterator{
    int curr;
    int maxIndex;
    int *list;
};

struct Iterator* ArrayList_getIterator(ArrayList *list){
    struct Iterator *pt = malloc(sizeof(struct Iterator));
    pt->curr = 0;
    pt->maxIndex = list->size;
    pt->list = list->list;
    return pt;
}

void Iterator_forEach(struct Iterator* iter, void (fun_ptr)(int)){
    for(; iter->curr < iter->maxIndex; ++iter->curr)
        (*fun_ptr)(iter->list[iter->curr]);
}


// return range of primes with null terminator
// void *search_range(NumberRange *range);
void *search_range(void *arg){
    ArrayList *list = *(ArrayList **)arg;
    NumberRange *range = *(NumberRange **)(arg + sizeof(ArrayList *));
    void *thread_id = (void *)(uintptr_t)(arg + sizeof(ArrayList *) * 2);
    for(int i = range->start; i < range->end; i++){
        for(int j = 2; j <= (int)sqrt(i); j++){
            if(i % j == 0)
                goto NOT_PRIME;
        }
        ArrayList_add(list, i);
        NOT_PRIME:;
        // sched_yield();
    }
    return (void *)(uintptr_t)thread_id;
    // refactor to ensure correct order of numbers
}

void print(int n){
    printf("%d, ", n);
}

int main(void){
    printf("Checking for primes from 1-%d\nUsing %d threads\n", NTHREADS*PER_THREAD, NTHREADS);
    pthread_t threads[NTHREADS];
    ArrayList *primes = ArrayList_init(1000000);
    for(int i = 0; i < NTHREADS; i++){
        // Data packing :) :) 220 coming in clutch
        void *arg = malloc(sizeof(ArrayList *) + sizeof(NumberRange *));
        *(uintptr_t *) arg = (uintptr_t) primes;
        *(uintptr_t *) (arg+sizeof(ArrayList *)) = (uintptr_t) NumberRange_init(i*PER_THREAD?:2, (i+1)*PER_THREAD);
        *(int *) (arg+sizeof(ArrayList *) * 2) = i;
        // fprintf(stderr, "Thread %d created...\n", i);
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
    for(int i = 0; i < primes->size; i++){
        int length = (int)(ceil(log10(primes->list[i])));
        char *x = calloc(length + 2, sizeof(char));
        sprintf(x, "%d", primes->list[i]);
        *(x+length) = '\n';
        fwrite(x, sizeof(char), length + 1, fptr);
    }
    return 0;
}