#pragma once
#include<stack>
#include<atomic>
#include<vector>
#include<utility>
#include<unordered_map>
#include<functional>
#include<mutex>
#include "PartialSumTree.h"
#include "DinicEdgeLayer.h"
struct flowQuantum
{
	flowQuantum() :e(-1), q(0) {}
	flowQuantum(int e_, int q_) :e(e_), q(q_) {}
	int e;
	int q;
};
class intWrapper;
class flowWrapper;
class DinicVertexLayer
{
public:
	DinicVertexLayer();
	//void pushMax();
	int& e();
	void addToStack(int e, int q);
	flowQuantum getTopOfStack(); //will remove from the stack
	bool isBlocked() const;
	void block();
	void unblock();//do i need this
private:
	int excess;
	std::stack<flowQuantum> Stack;
	bool blocked;
};

//parallel implementation
enum class Tree { Out, In, Access, Sum };
class DinicVertexLayerP
{
public:
	//graph operations
	DinicVertexLayerP(int self, std::vector<int> edgesOut, std::vector<int> edgesIn, int n);
	DinicVertexLayerP(const DinicVertexLayerP & other);
	void reset(const std::vector<DinicEdgeLayer>& capacities);
	//PSTree
	int root(Tree t);
	std::pair<int, int> find(int alpha, Tree t);
	void update(const int index, const int value, Tree t, const int edge = -1);
	int sum(int index, Tree t);
	void clear(int index, Tree t);
	//returns edges from k' to k
	std::vector<int> getRange(int k);
	int getFromOut(int j);
	flowQuantum getFromIn(int j);
	//Vertex
	int getE();
	void setE(int value);
	void increaseE(int value);
	std::atomic<int> & hd();
	std::atomic<int> & kPrime();
	int getIndex(int vertex, Tree t);
	std::vector<int> getEdgesIn();
	bool isBlocked() const;
	void block();
	void unblock();

	//Assign stuff
	std::vector<std::function<void()>> nextJobs();
	void addJob(std::function<void()> fn);
	void queueJob(std::function<void()> fn);
	void clearJobs(bool check = false);
	void clearJobs(std::unique_lock<std::mutex>&, bool check = false);
	void clearRunJobs();
	void runJob(int index);
	int size();
	//std::mutex& lock();
	//void unlock(std::unique_lock<mutex>&);

private:
	std::vector < std::function<void()>> jobs;
	std::vector <bool> runJobs;
	std::vector < std::function<void()>> queueJobs;
	std::atomic<int> queueTime;
	int self;
	std::atomic<int> excess;
	std::atomic<int> internal_hd;
	std::atomic<int> internal_kPrime;
	std::atomic<bool> blocked;
	//used with getIndex
	std::unordered_map<int, int> in;
	std::unordered_map<int, int> out;
	std::vector<int> edgesIn;
	std::vector<int> edgesOut;
	
	PartialSumTree<intWrapper> Tout;
	PartialSumTree<intWrapper> Taccess;
	PartialSumTree<intWrapper> Tsum;
	PartialSumTree<flowWrapper> Tin;
	std::mutex internal_mutex;
};

class flowWrapper
{
public:
	inline flowWrapper(int i) { assert(i == 0); }// fq.q = 0; fq.e = -1;}
	inline flowWrapper(flowQuantum fq_) : fq(fq_) {}
	inline operator int() const { return fq.q; }
	inline flowWrapper& operator= (int i)
	{
		fq.q = i;
		return *this;
	}
	//bool operator == (const flowWrapper& other) const
	flowQuantum fq;
};
class intWrapper
{
public:
	inline intWrapper(int i_) :i(i_) {}
	inline operator int() const { return i; };
	inline intWrapper& operator= (int i_)
	{
		i = i_;
		return *this;
	}
	/*bool operator == (const intWrapper& other) const
	{
		return i == other.i;
	}*/
private:
	int i;

};
