#pragma once
#include "DinicVertexLayer.h"
#include "DinicEdgeLayer.h"
#include "DinicEdge.h"
#include "Assign.h"
#include <iterator>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set>



class DinicGraph
{
public:
	friend std::istream& operator >> (std::istream & is, DinicGraph & graph);
	int getMaxFlow();
	int getMaxFlowParallel();
	int getMaxFlowParallelSchedule();

private:
	//generate layered networks
	bool generateLayeredNetwork();
	//data structures to use
	std::vector<std::vector<std::pair<int, int>>> edgeLayers;
	std::vector<std::vector<DinicEdge>> edges;

	void pushMax();
	void push(int u);
	void returnPush(int u);

	//parallel versions of above
	void pushMaxP();
	void pushP(int u);
	void pushPEdge(int u, int w, int j, const std::pair<int, int> & kp);
	void returnPushP(int u);
	void returnPushPEdge(int u, int v, int q, int j, const std::pair<int, int> & kp);

	//Assign functions
	void pushStep1(int v);
	void pushStep2(int v, int w, int j, const std::pair<int, int> & kp);
	void pushStep3(int v, int k);
	void pushStep4(int v);

	void returnStep1(int v);
	void returnStep2(int v, int u, int q, int j, const std::pair<int, int> & kp);
	void returnStep3(int v, int k);

	void cleanStep1(int v, int j);
	void cleanStep2(int v);

	std::deque<std::function<void()>> jobs;
	void addJobs();
	
	std::deque<int> unbalanced;
	std::set<int> inQueue;
	Assign assign;
	void addToQueue(int v);
	void addUnbalanced();

	//data to keep
	std::vector<DinicVertexLayer> network;
	std::vector<std::vector<DinicEdgeLayer>> layerEdges;
	std::deque<std::mutex> mutexs;

	//std::vector<ParallelLayeredVertex> parallelLayeredNetwork;
	//Parallel Stuff
	std::vector<DinicVertexLayerP> networkP;
};
