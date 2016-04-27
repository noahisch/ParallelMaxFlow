#pragma once
#include "Assign.h"
#include <cassert>
using namespace std;

Assign::Assign():network(nullptr) {}

//Assign::Assign(int n) : Tassign(vector<intWrapper>(n, { 0 })) {}
void Assign::init(int n, std::vector<DinicVertexLayerP> & networkObj)
{
	Tassign = PartialSumTree<intWrapper>(vector<intWrapper>(n, { 0 }));
	network = &networkObj;
	for (int j = 1; j <= Tassign.size(); ++j)
	{
		Tassign.update(j, (*network)[j - 1].size());
	}
	pool.initThreads();
}

void Assign::run(const deque<int>& unbalanced, int steps)
{
	assert(network != nullptr);
	assert(!(*network).empty());
	jobQueue.clear();
	for (int i = 0; i != steps; ++i) {
		for (int j = 0; j != (*network).size(); ++j)
		{
			pool.addJob(bind(&Assign::processorSetup, this, j));
		}
		pool.parallelRun();
		pool.parallelRun();
		assert(pool.isRunning() == false);
	}
}

void Assign::processorSetup(int j)
{

	(*network)[j].clearRunJobs(); //updates the size
	Tassign.update(j+1, (*network)[j].size());
	int k = 1;
	pool.queueJob(bind(&Assign::processorLoop, this, j+1, k));


}

void Assign::processorLoop(int j, int k)
{
	int n = (*network).size();
	while (j + (k - 1)*n <= Tassign.root())
	{
		int g1 = j + (k - 1) * n;
		auto it = Tassign.find(g1);
		if (it.first - 1 >= (*network).size()) continue;
		(*network)[it.first-1].runJob(it.second-1);
		++k;
		//might push back onto the stack???
	}
	

}