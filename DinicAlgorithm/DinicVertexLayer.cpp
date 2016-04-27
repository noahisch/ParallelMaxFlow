#include "DinicVertexLayer.h"
#include <cassert>
using namespace std;
//SERIAL
DinicVertexLayer::DinicVertexLayer() : excess(0), blocked(false) {}
int& DinicVertexLayer::e() { return excess; }
void DinicVertexLayer::addToStack(int e, int q)
{
	Stack.push({ e, q });
}
flowQuantum DinicVertexLayer::getTopOfStack()
{
	assert(!Stack.empty());
	auto temp = Stack.top();
	Stack.pop();
	return temp;
}
bool DinicVertexLayer::isBlocked() const
{
	return blocked;
}
void DinicVertexLayer::block()
{
	blocked = true;
}
void DinicVertexLayer::unblock()
{
	blocked = false;
}

//PARALLEL

//graph operations
DinicVertexLayerP::DinicVertexLayerP(int self_, std::vector<int> edgesOut_, std::vector<int> edgesIn_, int n)
	:self(self_), edgesOut(edgesOut_), edgesIn(edgesIn_), blocked(false), excess(0), queueTime(-1),
	Tout	(vector<intWrapper>(edgesOut.size(), { 0 })),
	Taccess	(vector<intWrapper>(edgesIn.size(), { 0 })),
	Tsum	(vector<intWrapper>(edgesOut.size(), { 0 })),
	Tin(vector<flowWrapper>(edgesIn.size() * 2 * n, { 0 }))

{
	for (int i = 0; i != edgesOut.size(); ++i)
	{
		out[edgesOut[i]] = i+1;
	}
	for (int i = 0; i != edgesIn.size(); ++i)
	{
		in[edgesIn[i]] = i+1;
	}
	internal_hd = 0;
	internal_kPrime = 1;
}
DinicVertexLayerP::DinicVertexLayerP(const DinicVertexLayerP & other)
:self(other.self), excess(other.excess.load()), internal_hd(other.internal_hd.load()),
internal_kPrime(other.internal_kPrime.load()), blocked (other.blocked.load()), in(other.in),
edgesIn(other.edgesIn), edgesOut(other.edgesOut), queueTime(-1)
{
	Tout = other.Tout;
	Taccess = other.Taccess;
	Tin = other.Tin;
	Tsum = other.Tsum;
	internal_hd = 0;
	internal_kPrime = 1;
}
void DinicVertexLayerP::reset(const std::vector<DinicEdgeLayer>& capacities)
{
	if(self !=0 && self != capacities.size()-1)assert(excess == 0);
	for (int i = 1; i <= internal_hd; ++i)
	{
		Tin.clear(i);
	}
	assert(Tin.root() == 0);
	for (int i = 0; i != capacities.size(); ++i)
	{
		if(out.find(i) != out.end())
		Tout.update(out[i], capacities[i].getCapacity());
	}
	internal_hd = 0;
	internal_kPrime = 1;
	unblock();
}
//PSTree
int DinicVertexLayerP::root(Tree t)
{
	switch (t)
	{
	case Tree::Out:
		return Tout.root();
	case Tree::In:
		return Tin.root();
	case Tree::Access:
		return Taccess.root();
	case Tree::Sum:
		return Tsum.root();

	}
}
std::pair<int, int> DinicVertexLayerP::find(int alpha, Tree t)
{
	switch (t)
	{
	case Tree::Out:
		return Tout.find(alpha);
	case Tree::In:
		return Tin.find(alpha);
	case Tree::Access:
		return Taccess.find(alpha);
	case Tree::Sum:
		return Tsum.find(alpha);

	}
}
void DinicVertexLayerP::update(const int index, const int value, Tree t, const int e)
{
	switch (t)
	{
	case Tree::Out:
		return Tout.update(index, value);
	case Tree::In:
		assert(e > -1);
		return Tin.updateValue(index, flowQuantum( e, value ));
	case Tree::Access:
		return Taccess.update(index, value);
	case Tree::Sum:
		return Tsum.update(index, value);

	}
}
int DinicVertexLayerP::sum(int index, Tree t)
{
	switch (t)
	{
	case Tree::Out:
		return Tout.sum(index);
	case Tree::In:
		return Tin.sum(index);
	case Tree::Access:
		return Taccess.sum(index);
	case Tree::Sum:
		return Tsum.sum(index);

	}
}
void DinicVertexLayerP::clear(int index, Tree t)
{
	switch (t)
	{
	case Tree::Out:
		return Tout.clear(index);
	case Tree::In:
		return Tin.clear(index);
	case Tree::Access:
		return Taccess.clear(index);
	case Tree::Sum:
		return Tsum.clear(index);

	}
}
//returns edges from k' to k
std::vector<int> DinicVertexLayerP::getRange(int k)
{
	assert(internal_kPrime > 0);
	return vector<int>(edgesOut.begin() + internal_kPrime -1, edgesOut.begin() + k);
}
int DinicVertexLayerP::getFromOut(int j)
{
	return Tout[j];
}
flowQuantum DinicVertexLayerP::getFromIn(int j)
{
	return Tin[j].fq;
}
//Vertex
void DinicVertexLayerP::increaseE(int value)
{
	unique_lock<mutex> lk(internal_mutex);
	excess += value;
}
int DinicVertexLayerP::getE()
{
	unique_lock<mutex> lk(internal_mutex);
	return excess;
}

void DinicVertexLayerP::setE(int value)
{
	unique_lock<mutex> lk(internal_mutex);
	excess = value;
}

atomic<int>  & DinicVertexLayerP::hd()
{
	return internal_hd;
}
atomic<int>  & DinicVertexLayerP::kPrime()
{
	return internal_kPrime;
}
int DinicVertexLayerP::getIndex(int vertex, Tree t)
{
	switch (t)
	{
	case Tree::Out:
	case Tree::Sum:
		return out[vertex];
	case Tree::Access:
		return in[vertex];
	default:
		assert(false);
		return -1;
	}
}
std::vector<int> DinicVertexLayerP::getEdgesIn()
{
	return edgesIn;
}
bool DinicVertexLayerP::isBlocked() const
{
	return blocked == true;
}
void DinicVertexLayerP::block()
{
	blocked = true;
}
void DinicVertexLayerP::unblock()
{
	blocked = false;
}

//Assign functions

std::vector<std::function<void()>> DinicVertexLayerP::nextJobs()
{
	unique_lock<mutex> lk(internal_mutex);
	assert(queueJobs.size() < 2);
	vector<function<void()>> output;
	output.swap(jobs);
	clearJobs();
	/*assert(jobs.empty());
	jobs.swap(queueJobs);
	assert(queueJobs.empty());*/
	lk.unlock();
	return output;
}
void DinicVertexLayerP::addJob(std::function<void()> fn)
{
	unique_lock<mutex> lk(internal_mutex);
	jobs.push_back(fn);
	runJobs.push_back(false);
	
}
void DinicVertexLayerP::queueJob(std::function<void()> fn)
{
	unique_lock<mutex> lk(internal_mutex);
	queueJobs.push_back(fn);
	assert(queueTime == -1);
	queueTime = 0;
	if (queueJobs.size() > 1)
	{
		cout << "I SHOULD NOT HAPPEN" << endl;
		assert(false);
	}
	
}

//std::mutex& DinicVertexLayerP::lock()
//{
//	return internal_mutex;
//}
//void DinicVertexLayerP::unlock(std::unique_lock<mutex>& lk)
//{
//	assert(lk.mutex() == &internal_mutex);
//	lk.unlock();
//}

void DinicVertexLayerP::clearJobs(bool check)
{
	unique_lock<mutex> lk(internal_mutex);
	clearJobs(lk, check);
}

void DinicVertexLayerP::clearJobs(std::unique_lock<std::mutex>& lock, bool check)
{
	if (!lock.owns_lock())
	{
		lock.lock();
	}
	jobs.clear();
	jobs.swap(queueJobs);
	runJobs = vector<bool>(jobs.size(), false);
	assert(queueJobs.empty());
	if (check) assert(jobs.empty());
}

void DinicVertexLayerP::runJob(int index)
{
	//unique_lock<mutex> lk(internal_mutex);
	if (index >= jobs.size()) return;
	assert(runJobs[index] == false);
	//lk.unlock();
	jobs[index]();
	//lk.lock();
	runJobs[index] = true;
	//lk.unlock();
}

int DinicVertexLayerP::size()
{
	return jobs.size();
}

void DinicVertexLayerP::clearRunJobs()
{
	unique_lock<mutex> lk(internal_mutex);
	int i = 0;
	for (; i != jobs.size(); ++i)
	{
		if (runJobs[i] == false) break;
	}
	jobs.erase(jobs.begin(), jobs.begin() + i);
	if (queueTime != -1) ++queueTime;
	if (queueTime == 2)
	{
		queueTime = -1;
		assert(jobs.empty()); //no jobs left to run
		clearJobs(lk);
	}
	for (; i < jobs.size(); ++i)
	{
		assert(runJobs[i] == false);
	}
	runJobs = vector<bool>(jobs.size(), false);
	lk.unlock();
}