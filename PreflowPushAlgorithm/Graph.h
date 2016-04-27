#pragma once
#include "Vertex.h"
#include "Edge.h"
#include <vector>
#include <set>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>


class Graph
{
public:
	//Graph();
	void computeMaxFlowParallel();
	int getParallelFlow();
	int computeMaxFlow();
	friend std::istream& operator >> (std::istream & is, Graph & graph);

private:
	std::vector<int> getPushes();
	std::vector<int> getLifts();
	void threadLoop(int index);

	bool existsAPushLiftOperation();
	void push(int index);
	void push(int u, int v, int amount);
	void lift(int u);

	std::vector<Vertex> vertices;
	std::vector<std::vector<Edge>> edges;

	void addToPushLift(int index);

	int getVertexOfMinHeight(int u);
	bool getPushVertex(int u, int&v);

	//Sets
	std::set<int> pushableVertices;
	std::set<int> liftableVertices;
	std::atomic<bool> done;
	//debugging
	std::vector<std::atomic<bool>> running;

	std::mutex outputLock;

	std::mutex endLock;
	std::condition_variable cv;
	//std::vector<std::mutex> mutexs;
	//std::vector<std::condition_variable> cvs;
	std::atomic<int> threadCount;
	std::vector<std::thread*> threads;
};