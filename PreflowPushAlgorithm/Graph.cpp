#include "Graph.h"
#include <limits>
#include <cassert>
#include <iostream>
#include <thread>
#include <functional>
#include <algorithm>
using namespace std;

int min(int a, int b)
{
	return a < b ? a : b;
}

void Graph::computeMaxFlowParallel()
{
	//set up threads and go
	//threadCount = 0;
	done = false;
	vertices[0].setHeight(vertices.size());
	//implicit setting of height
	//implicit setting of flow
	//assign starting values of (s,u)
	for (unsigned i = 0; i != edges[0].size(); ++i)
	{
		int flow = edges[0][i].getCapacity();
		if (flow == 0) continue;
		edges[0][i].push(flow);
		edges[i][0].push(-flow);
		vertices[i].e() += flow;
		//{
		//	unique_lock<mutex> out(outputLock);
		//	//cout << "Pushed " << flow << " on edge " << 0 << " " << i << '\n';
		//}
		//liftableVertices.insert(i);

	}
	//init for each
	threadCount = vertices.size() - 2;
	threads.reserve(vertices.size());
	unique_lock<mutex> lk(endLock);
	//make the vectors
	//mutexs.swap(vector<mutex>(vertices.size()));
	//cvs.swap(vector<condition_variable>(vertices.size()));
	running.swap(vector<atomic<bool>>(vertices.size()));
	for (unsigned i = 1; i != vertices.size()-1; ++i)
	{
		threads.emplace_back(new thread(bind(&Graph::threadLoop, this, i)));
	}
	cv.wait(lk);
	assert(threadCount == 0);
	//flow = vertices.back().e();
}

int Graph::getParallelFlow()
{
	return vertices.back().e();
}

void Graph::threadLoop(int index)
{
	auto & u = vertices[index];
	running[index] = true;
	while (true)
	{
		if (u.e() > 0)
		{
			int e = u.e();
			int v = getVertexOfMinHeight(index);
			if (v == -1)
			{
				return;
			}
			if (u.h() > vertices[v].h())
			{
				int d = min(e, edges[index][v].getCapacity());
				edges[index][v].push(d);
				edges[v][index].push(-d);
				u.e() -= d;
				vertices[v].e() += d;
				//spawn the thread
				//adjust capactiy
				//cvs[v].notify_all();
				/*{
					unique_lock<mutex> out(outputLock);
					cout << "Pushing " << d << " on edge " << index << " " << v << "\n";
					out.unlock();
				}*/

			}
			else
			{

				u.setHeight(vertices[v].h() + 1);
				/*{
					unique_lock<mutex> out(outputLock);
					cout << "Lifting " << index << " to level " << u.getHeight() << '\n';
					out.unlock();
				}*/
			}
		}
		while (u.e() == 0)
		{
			//acquire own mutex and wait
			if(running[index] == true)
			--threadCount;
			running[index] = false;
			if (threadCount == 0)
			{
				//check all are 0
				//use some kind of shared mutex
				if (any_of(vertices.begin() + 1, vertices.end()-1, [](auto & v) {return v.e() != 0; }))
				{
					//one isn't done
				}
				else {
					done = true;
					cv.notify_all();
				}
			}
			if (done) return;
			this_thread::yield();
			if (u.e() > 0)
			{
				running[index] = true;
				++threadCount;
				break;
			}
			/*if (threadCount == 0) cv.notify_all();
			unique_lock<mutex> lk(mutexs[index]);
			cvs[index].wait(lk);
			++threadCount;*/
		}
	}
	//cout << " Thread Exited" << endl;
}

int Graph::computeMaxFlow()
{
	//Init
	vertices[0].setHeight(vertices.size());
	//implicit setting of height
	//implicit setting of flow
	//assign starting values of (s,u)
	for (unsigned i = 0; i != edges[0].size(); ++i)
	{
		int flow = edges[0][i].getCapacity();
		if (flow == 0) continue;
		edges[0][i].push(flow);
		edges[i][0].push(-flow);
		vertices[i].e() += flow;
		liftableVertices.insert(i);

	}
	bool done = true;
	while (done)
	{
		done = false;
		for (unsigned i = 1; i != vertices.size()-1; ++i)
		{
			if (vertices[i].e() != 0)
			{
				assert(vertices[i].e() > 0);
				done = true;
				int v = -1;
				bool val = getPushVertex(i, v);
				if (v != -1)
					push(i, v, min(vertices[i].e(), edges[i][v].getCapacity()));
				if (!val && v == -1)
					lift(i);
			}
		}
		if (done && vertices[0].e() > 0)
		{
			int v = -1;
			bool val = getPushVertex(0, v);
			if (v != -1)
				push(0, v, min(vertices[0].e(), edges[0][v].getCapacity()));
		}
	}
	//while (existsAPushLiftOperation())
	//{
	//	//new check
	//	
	//	auto pushVector = getPushes();
	//	for (auto it = pushVector.begin(); it != pushVector.end(); ++it)
	//	{
	//		push(*it);
	//	}
	//	auto liftVector = getLifts();
	//	for (auto it = liftVector.begin(); it != liftVector.end(); ++it)
	//	{
	//		lift(*it);
	//	}
	//}
	int maxFlow = 0;
	for (unsigned i = 0; i != edges[0].size(); ++i)
	{
		maxFlow += edges[0][i].getFlow();
	}
	return vertices.back().e();
	return maxFlow;
}

bool Graph::existsAPushLiftOperation()
{
	return !pushableVertices.empty() || !liftableVertices.empty();
}

vector<int> Graph::getPushes()
{
	vector<int> v(pushableVertices.begin(), pushableVertices.end());
	pushableVertices.clear();
	return v;
}

vector<int> Graph::getLifts()
{
	vector<int> v(liftableVertices.begin(), liftableVertices.end());
	liftableVertices.clear();
	return v;
}

void Graph::push(int index)
{
	if (vertices[index].e() == 0) return;
	int v = 0;
	bool assertb = false;
	if (!getPushVertex(index, v))
	{
		liftableVertices.insert(index);
		assertb = true;
	}
	if (v != -1)
	{
		int d = min(vertices[index].e(), edges[index][v].getCapacity());
		push(index, v, d);
		if (!assertb)
		{
			if (vertices[index].e() > 0) pushableVertices.insert(index);
		}
	}
	if(assertb)assert(pushableVertices.count(index) == 0);
	/*if (vertices[v].getHeight() >= vertices[index].getHeight())
	{
		liftableVertices.insert(index);
		return;
	}
	int d = min(vertices[index].e(), edges[index][v].getCapacity());
	push(index, v, d);*/
}

void Graph::push(int u, int v, int amount)
{
	edges[u][v].push(amount);
	edges[v][u].push(-amount);
	vertices[u].e() -= amount;
	vertices[v].e() += amount;
	assert(vertices[u].e() >= 0);
	assert(vertices[v].e() >= 0);
	//add to pushable objects
	if (vertices[v].e() > 0) addToPushLift(v);
}

void Graph::lift(int u)
{
	if (vertices[u].e() == 0) return;
	vertices[u].setHeight( vertices[getVertexOfMinHeight(u)].h() + 1);
	pushableVertices.insert(u);
}

void Graph::addToPushLift(int u)
{
	if (u == vertices.size() - 1) return;
	int v = -1;
	if(!getPushVertex(u, v) && v == -1 && u != 0)
		liftableVertices.insert(u);
	if( v != -1)
		pushableVertices.insert(u);
	return;
	if (vertices[v].getHeight() < vertices[u].getHeight())
	{
		pushableVertices.insert(u);
	}
	else 
	{
		liftableVertices.insert(u);
	}
}

bool Graph::getPushVertex(int u, int &v)
{
	v = -1;
	int hu = vertices[u].getHeight();
	bool returnValue = false;
	for (unsigned i = 0; i != edges[u].size(); ++i)
	{
		int hv = vertices[i].getHeight();
		if (edges[u][i].getCapacity() > 0)
		{
			if (hv + 1 == hu)
			{
				v = i;
				if (returnValue) return returnValue;
			}
			if (hv < hu)
			{
				returnValue = true;
				if (v != -1) return returnValue;
			}
		}
	}
	return false;
}

int Graph::getVertexOfMinHeight(int u)
{
	int min = numeric_limits<int>::max();
	int index = -1;
	for (unsigned i = 0; i != edges[u].size(); ++i)
	{
		int hv = vertices[i].getHeight();
		if (edges[u][i].getCapacity() > 0 && hv < min)
		{
			min = hv;
			index = i;
		}
	}
	return index;
}

std::istream& operator >> (std::istream & is, Graph & graph)
{
	int nodes = 0;
	is >> nodes;
	assert(is);
	//init data
	graph.vertices.resize(nodes);
	graph.edges.resize(nodes);
	for (int i = 0; i != nodes - 1; ++i)
	{
		graph.edges[i].reserve(nodes);
		for (int j = 0; j != nodes; ++j)
		{
			int capacity;
			is >> capacity;
			graph.edges[i].emplace_back(capacity);
		}
	}
	graph.edges.back().resize(nodes, 0);
	return is;
}