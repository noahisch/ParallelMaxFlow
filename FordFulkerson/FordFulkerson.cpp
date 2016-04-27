#include "FordGraph.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
using namespace std;

void runTest(string name)
{
		ifstream ifs(name);
		assert(ifs.is_open());
		FordVertex::clearIndex();
		FordGraph graph;
		//graph.makeParallel();
		ifs >> graph;
		int flow = graph.computeMaxFlow();
		cout << "Found " << flow << " in " << name << '\n';
}
void runTestParallel(string name)
{
	ifstream ifs(name);
	assert(ifs.is_open());
	FordVertex::clearIndex();
	FordGraph graph;
	graph.makeParallel();
	ifs >> graph;
	int flow = graph.computeMaxFlow();
	cout << "Found " << flow << " in " << name << '\n';
}

void runRelease(bool parallel)
{
	FordGraph graph;
	if(parallel) graph.makeParallel();
	cin >> graph;
	int flow = graph.computeMaxFlow();
	cout << "Found " << flow << '\n';
}


int main()
{
	/*runRelease(true);
	return 0;
*/
	int testIndex = 0;
	vector<string> tests = { "sample.txt", "sample2.txt", "sample3.txt" };
	vector<int> answers = { 5,23,19 };
	for (auto & test : tests)
	{
			ifstream ifs(test);
			assert(ifs.is_open());
			FordVertex::clearIndex();
			FordGraph graph;
			//graph.makeParallel();
			ifs >> graph;
			int flow = graph.computeMaxFlow();
			cout << "Found " << flow << " in " << tests[testIndex] << '\n';
			assert(flow == answers[testIndex]);
			++testIndex;
	}
	
	runTest("maxFlow-25-0.50.txt");
	runTestParallel("maxFlow-25-0.50.txt");
}