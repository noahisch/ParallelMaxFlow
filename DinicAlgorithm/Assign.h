#pragma once
#include "PartialSumTree.h"
#include "DinicVertexLayer.h"
#include "ThreadPool.h"
#include <vector>
#include <deque>
class Assign //used to simulate 
{
public:
	Assign();
	//Assign(int n);
	void init(int n, std::vector<DinicVertexLayerP> & network);
	//void updateNetwork(std::vector<DinicVertexLayerP> & network);
	//executes all jobs in the simulation
	void run(const std::deque<int>& unbalanced, int steps);

	//assign push /return jobs
	/*void assignPush(std::vector<std::function<void()>>& jobs);
	void assignReturn(std::vector<std::function<void()>>& jobs);*/
	//don't need becuase nodes grab it straight from the network

private:
	void processorSetup(int j);
	void processorLoop(int j, int k);
	PartialSumTree<intWrapper> Tassign;
	std::vector<DinicVertexLayerP> *network;
	//Assign Node holds jobs
	ThreadPool pool;
	std::vector<std::function<void()>> jobQueue;
};