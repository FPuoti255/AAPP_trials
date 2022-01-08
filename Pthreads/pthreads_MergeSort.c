#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

unsigned int array_size = 0, num_threads = 0, spawned = 0;
int *data;

typedef struct
{
    int start;
    int end;
} Boundaries;

typedef struct //TODO implement the system for balancing the tree of threads
{
    pthread_t id;
    pthread_t *left;
    pthread_t *right;
} ThreadNode;

pthread_attr_t attr;
pthread_barrier_t barr;
pthread_mutex_t mut;

void *randomInitialization(void *thread_index)
{
    unsigned int id = *(unsigned int *)thread_index;

    for (unsigned int i = id; i < array_size; i += num_threads)
    {
        data[i] = rand() % 100;
    }

    pthread_barrier_wait(&barr);
    return 0;
}

void mergesub(int start, int mid, int end)
{
    int *left = malloc((mid - start + 1) * sizeof(int));
    int *right = malloc((end - mid) * sizeof(int));

    // n1 is size of left part and n2 is size of right part
    int n1 = mid - start + 1, n2 = end - mid, i, j;

    // storing values in left part
    for (i = 0; i < n1; i++)
        left[i] = data[i + start];

    // storing values in right part
    for (i = 0; i < n2; i++)
        right[i] = data[i + mid + 1];

    int k = start;
    i = j = 0;

    // merge left and right in ascending order
    while (i < n1 && j < n2)
    {
        if (left[i] <= right[j])
            data[k++] = left[i++];
        else
            data[k++] = right[j++];
    }

    // insert remaining values from left
    while (i < n1)
    {
        data[k++] = left[i++];
    }

    // insert remaining values from right
    while (j < n2)
    {
        data[k++] = right[j++];
    }
}

void *mergesort(void *myBounds)
{
    int start = ((Boundaries *)myBounds)->start;
    int end = ((Boundaries *)myBounds)->end;

    if (start == end)
        return 0;

    pthread_mutex_lock(&mut);

    int still_spawnable = num_threads - spawned;
    int num = (still_spawnable > 2 ? 2 : still_spawnable);
    if (still_spawnable > 0)
    {
        spawned += num;
    }
    pthread_mutex_unlock(&mut);

    int mid = start + (end - start) / 2;

    Boundaries *bounds1 = malloc(sizeof(Boundaries));
    bounds1->start = start;
    bounds1->end = mid;

    Boundaries *bounds2 = malloc(sizeof(Boundaries));
    bounds2->start = mid + 1;
    bounds2->end = end;

    pthread_t threads[num];

    switch (num)
    {
    case 2:
        pthread_create(&threads[0], &attr, *mergesort, (void *)bounds1);
        pthread_create(&threads[1], &attr, *mergesort, (void *)bounds2);

        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);

        mergesub(start, mid, end);

        break;

    case 1:
        pthread_create(&threads[0], &attr, *mergesort, (void *)bounds1);
        mergesort(bounds2);

        mergesub(start, mid, end);

        break;

    case 0:

        mergesort(bounds1);
        mergesort(bounds2);

        mergesub(start, mid, end);

        break;

    default:
        break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Wrong number of parameters");
        return 0;
    }

    array_size = (unsigned int)atoi(argv[1]);
    num_threads = (unsigned int)atoi(argv[2]);
    data = malloc(array_size * sizeof(unsigned int));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    /* notice that alternatively to set the barrier count to num_thread+1 I could have: 
    -> set barrier_count = num_threads and made the main to 'Join' one of the threads */
    pthread_barrier_init(&barr, NULL, num_threads + 1);
    pthread_mutex_init(&mut, NULL);

    unsigned int indexes[num_threads];
    pthread_t threads[num_threads];

    //CREATING RANDOM DATA ARRAY
    time_t tm;
    srand((unsigned)time(&tm));
    for (int i = 0; i < num_threads; i++)
    {
        indexes[i] = i;
        pthread_create(&threads[i], &attr, randomInitialization, (void *)&indexes[i]);
    }
    pthread_barrier_wait(&barr);
    printf("Created random input data:\n");
    for (int i = 0; i < array_size; i++)
    {
        printf("%d ", data[i]);
    }

    //DIVIDE AND CONQUER
    Boundaries *bounds = malloc(sizeof(Boundaries));
    bounds->start = 0;
    bounds->end = array_size - 1;
    mergesort(bounds);

    printf("\nThe sorted array:\n");
    for (int i = 0; i < array_size; i++)
    {
        printf("%d ", data[i]);
    }

    pthread_barrier_destroy(&barr);
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mut);
    return 0;
}