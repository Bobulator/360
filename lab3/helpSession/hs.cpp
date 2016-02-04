#include <iostream>
#include <queue>
#include <pthread.h>
#include <sstream>
#include <semaphore.h>
#include <string>

using namespace std;

queue<int> work;
sem_t work_to_do, space_on_q, mutex;

struct thread_params
{
	long thread_id;
	string dir;
};

void *serve(void *arg)
{	
	//long thread_id = (long) arg;
	struct thread_params *tp = (struct thread_params*) arg; 
	
	int val;
	for (;;)
	{
		stringstream ss;
		ss << "I'm thread " << tp->thread_id;

		sem_wait(&work_to_do);
		sem_wait(&mutex);
		val = work.front();
		work.pop();
		sem_post(&mutex);
		sem_post(&space_on_q);
		
		ss << " working on " << tp->dir << endl;
		cout << ss.str();
	}
}

int main(int argc, char* argv[])
{
	int num_threads = 10;
	cout << "threads hello!" << endl;
	pthread_t threads[num_threads];
	
	string dir = "something/";
	for (long i = 1; i <= num_threads; ++i)
	{
		struct thread_params tp;
		tp.thread_id = i;
		tp.dir = dir;
		pthread_create(&threads[i], NULL, serve, (void *) &tp);
		
	}

	int queue_size = 20;
	sem_init(&space_on_q, 0, queue_size);
	sem_init(&work_to_do, 0, 0);
	sem_init(&mutex, 0, 1);

	for (int i = 0;; i++)
	{
		sem_wait(&space_on_q);
		sem_wait(&mutex);
		work.push(i);
		sem_post(&mutex);
		sem_post(&work_to_do);
	}


	return 0;
}
