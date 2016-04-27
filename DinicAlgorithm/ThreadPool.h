#pragma once
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
class ThreadPool
{
public:

	void initThreads();
	//add a job that is run
	void addJob(std::function<void()> fn);
	//add a job that will be run when all jobs in the current stack are done
	void queueJob(std::function<void()> fn);
	void parallelRun(); //will run all jobs blocks until return
	void run(); // runs all the current jobs and the queue jobs
	~ThreadPool();
	bool isRunning();
	std::mutex runMutex;
	std::condition_variable runCV;

private:
	std::deque<std::function<void()>> jobQueue;
	std::deque<std::function<void()>> waitingQueue;
	//parallel
	void internal_loop();
	std::vector<std::thread> threads;
	std::mutex queueMutex;
	std::condition_variable queueCV;
	int active_threads; //covered by queueMutex
	std::atomic<bool> done;
};