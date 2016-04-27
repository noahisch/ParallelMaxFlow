#pragma once
#include <iostream>
#include <set>
#include <vector>
#include "FordVertex.h"
#include <memory>

//Parallel Libraries
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
class ThreadPool
{
public:
	ThreadPool();
	void AddJob(std::function<void()>);
	void end();
	void clearJobs();
	void activateJobs(std::function<void()> callback_);
	bool jobs();
private:
	void loop_function();
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> Queue;
	std::queue<std::function<void()>> WaitingQueue;
	std::mutex QueueMutex;
	std::mutex WaitingQueueMutex;
	std::condition_variable QueueCV;
	std::atomic<bool> done;
	std::atomic<int> running;

	std::function<void()> callback;
	std::atomic<bool> callbackBool;
	std::mutex callBackLock;
};


class FordGraph
{
public:
	int computeMaxFlow();
	friend std::istream& operator >> (std::istream & is, FordGraph & graph);
	void makeParallel();
private:
	std::set<int> starter;
	std::set<int> unlabeled;
	std::set<int> labeled_and_scanned;
	std::set<int> labeled_and_unscanned;
	
	std::vector<std::unique_ptr<FordVertex>> vertices;
	std::vector<std::vector<int>> start; //adjaceny matrix
	std::vector<std::vector<int>> current;
	int maxFlow;
	std::vector<int> forwardNeighbors(int index);
	std::vector<int> backwardNeighbors(std:: vector<int> candidates, int index);
	void step1();
	bool step2();
	void step3();

	void label(int index, int source, int value);
	int getMaxFlow();
	int getFlow(int u, int v);

	void updateFlow(int u, int v, int amount);

	void parallelEdgeForward(int u, int v);

	void parallelEdgeBackward(int u, int v);

	bool isParallel = false;
	std::atomic<bool> parallelReturn = false;

	std::vector<std::mutex> mutexs;
	//std::vector<std::condition_variable> cvs;
	std::vector<std::atomic<bool>> scanned;
	std::vector<std::atomic<bool>> was_scanned;

	std::mutex step2Mutex;
	std::condition_variable step2Cv;

	void getLocks(int u, int v);
	void unlockLocks(int u, int v);

	std::atomic<int> scannedVertices;
	std::atomic<int> labeledVertices;
	//parallel
	std::set<std::pair<int, int>> jobs;
	bool better_parallel_step2();
	void better_parallel_step2_helper();
	bool parallel_step2();
	void parallel_step2_helper();
	void from_label_vertex_spawn_threads(int u);
	void parallel_edge(int u, int v);
	void pLabel(int index, int source, int value);
	std::set<int> move_to_labeled_and_unscanned;
	std::set<int> move_to_labeled_and_scanned;
	//void parallel_edge_forward(int u, int v);
	//void parallel_edge_backward(int u, int v);

	//Parallel Implementation with cv's
	ThreadPool pool;

	std::atomic<int> count;
	std::mutex labelLock;



};