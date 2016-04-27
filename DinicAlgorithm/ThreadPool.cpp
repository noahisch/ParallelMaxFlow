#include "ThreadPool.h"
#include <cassert>
using namespace std;

void ThreadPool::addJob(std::function<void()> fn)
{
	{
		unique_lock<mutex> lk(queueMutex);
		jobQueue.push_back(fn);
	}
	//queueCV.notify_one();
}
//add a job that will be run when all jobs in the current stack are done
void ThreadPool::queueJob(std::function<void()> fn)
{
	{
		unique_lock<mutex> lk(queueMutex);
		waitingQueue.push_back(fn);
	}
}

void ThreadPool::run()
{
	while (!jobQueue.empty())
	{
		while (!jobQueue.empty())
		{
			jobQueue.front()();
			jobQueue.pop_front();
		}
		jobQueue = waitingQueue; //change to swap
		waitingQueue.clear();
	}
}

void ThreadPool::initThreads()
{
	active_threads = 0;
	done = false;
	int amount = int(thread::hardware_concurrency());
	threads.reserve(amount);
	active_threads = amount;
	unique_lock<mutex> lk(runMutex);
	jobQueue.clear();
	waitingQueue.clear();
	for (int i = 0; i < amount; ++i)
	{
		threads.emplace_back(bind(&ThreadPool::internal_loop, this));
	}
	runCV.wait(lk, [this]() { return active_threads == 0; });
}
void ThreadPool::parallelRun()
{
	{
		unique_lock<mutex> lk2(queueMutex); //wait for last thread to finish
		assert(!jobQueue.empty() || !waitingQueue.empty());
		unique_lock<mutex> lk(runMutex);
		if (jobQueue.empty() && !waitingQueue.empty())
		{
			jobQueue.swap(waitingQueue);
		}
		lk2.unlock();
		queueCV.notify_all(); // wake up all the threads
		runCV.wait(lk, [this]() {return jobQueue.empty(); });
		assert(active_threads == 0);
	}
}
void ThreadPool::internal_loop()
{
	while (true)
	{
		{
			unique_lock<mutex> lk(queueMutex);
			--active_threads;
			if (active_threads == 0 && jobQueue.empty())
			{
				runCV.notify_one();
				////swap in the waiting queue
				//jobQueue.swap(waitingQueue);
				//assert(waitingQueue.empty());
				//if (jobQueue.empty())
				//{
				//	runCV.notify_one(); //wake up run thread
				//	//this thread will fall asleep since the queue is empty
				//}
				//else
				//{
				//	queueCV.notify_all(); //wake up all threads to run
				//}
			}
			while (jobQueue.empty())
			{
				if (done) return;
				queueCV.wait(lk);
			}
			if (done) return;
			++active_threads;
			auto job = jobQueue.front();
			jobQueue.pop_front();
			lk.unlock();
			job();
		}
	}
}

ThreadPool::~ThreadPool()
{
	done = true;
	queueCV.notify_all();
	for (auto & t : threads)
	{
		t.join();
	}
}

bool ThreadPool::isRunning()
{
	unique_lock<mutex> lk(queueMutex);
	return active_threads > 0;
}