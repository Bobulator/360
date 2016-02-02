#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <iostream>
#include <semaphore.h>
#include <sstream>

sem_t empty, full, mutex;

class MyQueue
{
	std::queue <int> stlqueue;
public:
	void push(int sock)
	{
		sem_wait(&empty);
		sem_wait(&mutex);
		stlqueue.push(sock);
		sem_post(&mutex);
		sem_post(&full);
	}
	int pop()
	{
		sem_wait(&full);
		sem_wait(&mutex);
		int rval = stlqueue.front();
		stlqueue.pop();
		sem_post(&mutex);
		sem_post(&empty);
		return rval;
	}
} sockqueue;

void *howdy(void *arg)
{
	std::stringstream ss;
	for(;;)
	{
		ss.str("");

		ss << (long) arg << " GOT " << sockqueue.pop() << std::endl;
		std::cout << ss.str();
	}
}



main()
{
#define NTHREADS 10
#define NQUEUE 20
	long number;
	pthread_t threads[NTHREADS];
	sem_init(&mutex, PTHREAD_PROCESS_PRIVATE, 1);
	sem_init(&full, PTHREAD_PROCESS_PRIVATE, 0);
	sem_init(&empty, PTHREAD_PROCESS_PRIVATE, NQUEUE);

	for (number = 1; number <= NTHREADS; number++)
	{
		pthread_create(&threads[number], NULL,
			       howdy, (void *) number);
	}
	for (int i = 0; i < NQUEUE; i++)
	{
		sockqueue.push(i);
	}
	pthread_exit(NULL);
}


