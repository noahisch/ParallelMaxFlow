// pushRelabel.cpp : Defines the entry point for the console application.
//

#include "Graph.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <thread>
#include <functional>
using namespace std;

void runTest(string name)
{
	ifstream ifs(name);
	assert(ifs.is_open());
	Graph graph;
	ifs >> graph;
	int flow = 0;
	flow = graph.computeMaxFlow(); 
	cout << "Found " << flow << " in " << name << '\n';
}

void runTestParallel(string name)
{
	ifstream ifs(name);
	assert(ifs.is_open());
	Graph graph;
	ifs >> graph;
	int flow = 0;
	thread mainThread(bind(&Graph::computeMaxFlowParallel, &graph));
	mainThread.join();
	flow = graph.getParallelFlow();
	cout << "Found " << flow << " in " << name << '\n';
}

void releaseTest(bool parallel)
{
	Graph graph;
	cin >> graph;
	int flow = 0;
	if (parallel)
	{
		thread mainThread(bind(&Graph::computeMaxFlowParallel, &graph));
		mainThread.join();
		flow = graph.getParallelFlow();
	}
	else
	{
		flow = graph.computeMaxFlow();
	}

	cout << "Found " << flow << '\n';
}

int main()
{
	//RELEASE
	//runTest("maxFlow-100-0.05.txt");
	releaseTest(false);
	//runTestParallel("maxFlow-3000-0.80.txt");
	return 0;
	using namespace std::placeholders;
	int testIndex = 0;
	vector<string> tests = { "sample.txt", "sample2.txt", "sample3.txt" };
	vector<int> answers = { 5,23,19 };
	for (auto & test : tests)
	{
		ifstream ifs(tests[testIndex]);
		assert(ifs.is_open());
		Graph graph;
		ifs >> graph;
		int flow = 0;
		//serial
		flow = graph.computeMaxFlow(); 
		//parallel
		//thread mainThread(bind(&Graph::computeMaxFlowParallel, &graph));
		//mainThread.join();
		//flow = graph.getParallelFlow();
		cout << "Found " << flow << " in " << tests[testIndex] << '\n';
		assert(flow == answers[testIndex++]);
	}
	runTest("maxFlow-25-0.50.txt");
	runTestParallel("maxFlow-25-0.50.txt");
	runTest("maxFlow-100-0.05.txt");
}