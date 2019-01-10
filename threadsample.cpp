//***************************************************
// Sample threading application.
//
// Must be compiled and linked with -pthread

#include <pthread.h>
#include <cstdio>
#include <stdlib.h>

typedef struct
{
    pthread_t id;
    int param1;
    double param2;
} thread_arg_t;

void *ThreadFunc(void *a)
{
    thread_arg_t *args = static_cast<thread_arg_t*>(a);

    printf("Thread %ld: Params: %d %f\n",
            pthread_self(), args->param1, args->param2);

    return nullptr;
}

int main(int argc, char **argv)
{
    int n_threads = 1;
    thread_arg_t *args;

    if (argc > 1) n_threads = atoi(argv[1]);

    args = new thread_arg_t[n_threads];

    for (int ii=0; ii<n_threads; ii++)
    {
        args[ii].param1 = ii;
        args[ii].param2 = ii*4.5;

        pthread_create(&args[ii].id, nullptr, ThreadFunc, &args[ii]);
    }

    for (int ii=0; ii<n_threads; ii++)
    {
        pthread_join(args[ii].id, nullptr);
    }

    delete args;

    printf("Add done\n");

    return 0;
}
