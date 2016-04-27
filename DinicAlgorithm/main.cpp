#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <fstream>
#include "PartialSumTree.h"
#include "DinicGraph.h"
using namespace std;
void runDinicTest(string test, int answer = -1)
{
	ifstream ifs(test);
	assert(ifs.is_open());
	DinicGraph graph;
	ifs >> graph;
	int flow = graph.getMaxFlow();
	if(answer != -1)assert(flow == answer);
	cout << "Found " << flow << " in " << test << endl;
}

void runDinicParallelAssignTest(string test, int answer=-1)
{
	ifstream ifs(test);
	assert(ifs.is_open());
	DinicGraph graph;
	ifs >> graph;
	int flow = graph.getMaxFlowParallelSchedule();
	if(answer != -1)assert(flow == answer);
	cout << "Found " << flow << " in " << test << endl;

}

int main(int argc, char** argv)
{
	//USED FOR RUNNING COMMAND PROMPT

	//DinicGraph graph;
	//cin >> graph;
	//if (argc > 1 && string(argv[1]) == "s")
	//{
	//	//runDinicTest("maxFlow-500-0.82.txt");
	//	cout << "Found " << graph.getMaxFlow() << endl;
	//}
	//else
	//{
	//	//runDinicParallelAssignTest("maxFlow-50-0.20-16.txt");
	//	cout << "Found " << graph.getMaxFlowParallelSchedule() << endl;
	//}
	//return 0;
	//PartialSumTree<PSTTest>::test();
	//return 0;

	//USED FOR TEST
	int testIndex = 0;
	vector<string> tests = { "sample.txt", "sample2.txt", "sample3.txt", "sample4.txt", "maxFlow-20-0.05.txt" , "maxFlow-25-0.50.txt", "maxFlow-50-0.20-16.txt" };
	vector<int> answers = { 5,23,19,3, 48, 72, -1};// , 48};
	for (auto & test : tests)
	{
		//serial
		runDinicTest(tests[testIndex], answers[testIndex]);
		runDinicParallelAssignTest(tests[testIndex], answers[testIndex]);
		++testIndex;
	}
	return 0;
}
 