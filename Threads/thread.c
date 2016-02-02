#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *howdy(void *arg)
{
	int tid;
	tid = (long)arg;
	printf("Hi %d\n",tid);
}

main()
{
#define NTHREADS 20
	long threadid;
	pthread_t threads[NTHREADS];

	for(threadid = 0; threadid < NTHREADS; threadid++) {
           pthread_create(&threads[threadid], NULL,
                          howdy, (void *)threadid);
	}
	pthread_exit(NULL);
}


