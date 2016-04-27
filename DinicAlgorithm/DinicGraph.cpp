#include "DinicGraph.h"
#include <cassert>
#include <queue>
#include <limits>
using namespace std;


inline int min(int a, int b)
{
	return a < b ? a : b;
}

istream& operator >> (istream & is, DinicGraph & graph)
{
	int nodes;
	is >> nodes;
	graph.edges.reserve(nodes);
	//read in the capacities
	for (int i = 0; i != nodes - 1; ++i)
	{
		graph.edges.push_back(vector<DinicEdge>());
		graph.edges[i].reserve(nodes);
		for (int j = 0; j != nodes; ++j)
		{
			int capacity;
			is >> capacity;
			graph.edges[i].emplace_back(capacity);
		}
	}
	graph.edges.push_back(vector<DinicEdge>());
	graph.edges.back().reserve(nodes);
	for (int j = 0; j != nodes; ++j)
	{
		graph.edges.back().emplace_back(0);
	}
	return is;
}

void DinicGraph::pushMax()
{
	for (int i = 1; i != layerEdges.size(); ++i)
	{
		int q = layerEdges[0][i].getCapacity();
		if (q == 0) continue;
		network[i].e() += q;
		network[i].addToStack(0, q);
		//cout << "Pushed " << q << " on edge " << 0 << " " << i << '\n';
		layerEdges[0][i].pushMax();
		addToQueue(i);
		/*unbalanced.push_back(i);
		inQueue.insert(i);*/
	}
}

void DinicGraph::pushMaxP()
{
	//networkP[0].e() =numeric_limits<int>::max();
	//pushP(0);
	auto & s = networkP[0];
	auto kp = s.find(s.root(Tree::Out), Tree::Out);
	auto ends = s.getRange(kp.first);
	for (int j = s.kPrime(); j <= kp.first; ++j)
	{
		pushPEdge(0, ends[j-s.kPrime()], j, kp);
	}
	addUnbalanced();
}

void DinicGraph::pushPEdge(int u, int w, int j, const pair<int, int> & kp)
{
	//setup
	auto & vertex = networkP[u];
	auto & vertexW = networkP[w];
	if (vertexW.isBlocked()) return;
	int r = vertexW.getIndex(u, Tree::Access); 

	vertexW.update(r, 1, Tree::Access); //2a
	int Sr = vertexW.sum(r, Tree::Access); //2b
	int q = vertex.getFromOut(j); //2c
	if (j == kp.first) q = kp.second;
	layerEdges[u][w].push(q); //2d
	//cout << "Pushed " << q << " on edge " << u << w << endl;
	int total = vertexW.root(Tree::In); //2e
	vertexW.update(vertexW.hd() + Sr, q, Tree::In, u); //2f
	vertex.update(j, vertex.getFromOut(j) - q, Tree::Out); //2g
	vertexW.hd() += vertexW.root(Tree::Access); //2h
	vertexW.clear(r, Tree::Access); //2i
	vertexW.increaseE(vertexW.root(Tree::In) - total); //2j
}

void DinicGraph::pushP(int v)
{
	auto & vertex = networkP[v];
	//Step 1
	int alpha = min(vertex.getE(), vertex.root(Tree::Out));
	if (alpha == 0) return;
	vertex.increaseE(-alpha);
	auto kp = vertex.find(alpha, Tree::Out);

	//Step 2
	auto ends = vertex.getRange(kp.first);
	for (int j = vertex.kPrime(); j <= kp.first; ++j)
	{
		pushPEdge(v, ends[j-vertex.kPrime()], j, kp);
	}
	//Step 3
	vertex.kPrime() = kp.first;

	//Step 4
	if (vertex.getE() > 0)
	{
		vertex.block();
		for (int u : vertex.getEdgesIn())
		{
			networkP[u].update(networkP[u].getIndex(v, Tree::Out), 0, Tree::Out);
		}
	}
}

void DinicGraph::returnPushPEdge(int v, int u, int q, int j, const std::pair<int, int> & kp)
{
	auto & vertex = networkP[v];
	auto & vertexU = networkP[u];
	//2a
	int d = q;
	if (j == kp.first) d = q - kp.second; //ABSTACT THIS OUT
	if (d == 0) return;
	//2b
	if (j == kp.first) vertex.update(j, kp.second, Tree::In, u);
	else vertex.update(j, 0, Tree::In, u);
	//2c
	//cout << "Reversing " << d << " on edge" << u << v << endl;
	layerEdges[u][v].push(-d);
	//2d
	//2e do I need to do the edge stuff
	vertexU.update(vertexU.getIndex(v, Tree::Sum), d, Tree::Sum);
	//2f
	vertexU.increaseE(vertexU.root(Tree::Sum));
	//2g
	//2h
	vertexU.clear(vertexU.getIndex(v, Tree::Sum), Tree::Sum);
	
}

void DinicGraph::returnPushP(int v)
{
	auto & vertex = networkP[v];
	//Step 1
	auto kp = vertex.find(vertex.root(Tree::In) - vertex.getE(), Tree::In);
	vertex.setE(0);
	//Step 2
	for (int j = kp.first; j <= vertex.hd(); ++j)
	{
		auto Q = vertex.getFromIn(j);
		returnPushPEdge(v, Q.e, Q.q, j, kp);
	}
	//Step 3
	vertex.hd() = kp.first;
}

void DinicGraph::addJobs()
{
	for (int i = 0; i != unbalanced.size(); i++)
	{
		auto vJobs = networkP[unbalanced[i]].nextJobs();
		for (int j = 0; j < vJobs.size(); ++j)
		{
			jobs.push_back(vJobs[j]);
		}
	}
}

int DinicGraph::getMaxFlowParallelSchedule()
{
	network = vector<DinicVertexLayer>(edges.size()); //for auxilarry use
	networkP.clear();
	networkP.reserve(edges.size());
	for (int i = 0; i != edges.size(); ++i)
	{
		std::vector<int> EdgeIn;
		std::vector<int> EdgeOut;
		for (int j = 0; j != edges.size(); ++j)
		{
			if (i == j) continue;
			if (edges[i][j].getCapacity() > 0) EdgeOut.push_back(j);
			if (edges[j][i].getCapacity() > 0) EdgeIn.push_back(j);
		}
		networkP.emplace_back(i, EdgeOut, EdgeIn, edges.size());
	}
		/*assign = Assign(networkP.size());
		assign.updateNetwork(networkP);*/
		mutexs.resize(networkP.size());
		assign.init(networkP.size(), networkP);
		while (generateLayeredNetwork())
		{

			unbalanced.clear();
			inQueue.clear();
			//initialize step
			//also takes care of previous clear
			for (int i = 0; i != networkP.size(); ++i)
			{
				networkP[i].reset(layerEdges[i]);
			}
			//push
			pushMaxP();
			//loop
			while (!unbalanced.empty())
			{
				//PUSH
				for (int i = 0; i != unbalanced.size(); ++i)
				{
					networkP[unbalanced[i]].clearJobs(true);
					networkP[unbalanced[i]].addJob(bind(&DinicGraph::pushStep1, this, unbalanced[i]));
				}

				assign.run(unbalanced, 4);
				//clear all the jobs
				//RETURN
				for (int i = 0; i != unbalanced.size(); ++i)
				{
					networkP[unbalanced[i]].clearJobs(true);
					networkP[unbalanced[i]].addJob(bind(&DinicGraph::returnStep1, this, unbalanced[i]));
				}
				assign.run(unbalanced, 3);
				for (int i = 0; i != unbalanced.size(); ++i)
				{
					networkP[unbalanced[i]].clearJobs(true);//clean up jobs
				}
				unbalanced.clear();
				addUnbalanced();
			}
			//update the edges
			for (int i = 0; i != edges.size(); ++i)
			{
				for (int j = 0; j != edges.size(); ++j)
				{
					edges[i][j].push(layerEdges[i][j].getFlow());
				}
			}

		}

	int flow = 0;
	for (int i = 1; i != edges.size(); ++i)
	{
		flow += edges[0][i].getFlow();
	}
	return flow;
}

int DinicGraph::getMaxFlowParallel()
{
	//deque<int> unbalance;
	network = vector<DinicVertexLayer>(edges.size()); //for auxilarry use
	networkP.clear();
	networkP.reserve(edges.size());
	for (int i = 0; i != edges.size(); ++i)
	{
		std::vector<int> EdgeIn;
		std::vector<int> EdgeOut;
		for (int j = 0; j != edges.size(); ++j)
		{
			if (i == j) continue;
			if (edges[i][j].getCapacity() > 0) EdgeOut.push_back(j);
			if (edges[j][i].getCapacity() > 0) EdgeIn.push_back(j);
		}
		networkP.emplace_back(i, EdgeOut, EdgeIn, edges.size());
	}
	//while loop
	while (generateLayeredNetwork())
	{
		unbalanced.clear();
		inQueue.clear();
		//initialize step
		//also takes care of previous clear
		for (int i = 0; i != networkP.size(); ++i)
		{
			networkP[i].reset(layerEdges[i]);
		}
		//push
		pushMaxP();
		//loop
		while (!unbalanced.empty())
		{
			int size = unbalanced.size();
			for (int v = 0; v != size; ++v)
			{
				pushP(unbalanced[v]);
			}
			for (int v = 0; v != size; ++v)
			{
				returnPushP(unbalanced[v]);
			}
			unbalanced.clear();
			addUnbalanced();
		}
		//update the edges
		for (int i = 0; i != edges.size(); ++i)
		{
			for (int j = 0; j != edges.size(); ++j)
			{
				edges[i][j].push(layerEdges[i][j].getFlow());
			}
		}

	}
	int flow = 0;
	for (int i = 1; i != edges.size(); ++i)
	{
		flow += edges[0][i].getFlow();
	}
	return flow;
}

int DinicGraph::getMaxFlow()
{
	//deque<int> unbalance;
	network = vector<DinicVertexLayer>(edges.size());
	//Non-parallel
	while (generateLayeredNetwork())
	{
		unbalanced.clear();
		inQueue.clear();
		network = vector<DinicVertexLayer>(edges.size());
		pushMax();
		while (!unbalanced.empty())
		{
			int x = unbalanced.front();
			unbalanced.pop_front();
			inQueue.erase(x);
			push(x);
			returnPush(x);
			assert(network[x].e() == 0);

		}
		//restore edges
		for (int i = 0; i != edges.size(); ++i)
		{
			for (int j = 0; j != edges.size(); ++j)
			{
				edges[i][j].push(layerEdges[i][j].getFlow());
			}
		}
	}
	int flow = 0;
	for (int i = 1; i != edges.size(); ++i)
	{
		flow += edges[0][i].getFlow();
	}
	return flow;
}

void DinicGraph::addUnbalanced()
{
	for (int v = 1; v != networkP.size() - 1; ++v)
	{
		if (networkP[v].getE() > 0)
			unbalanced.push_back(v);
	}
}

void DinicGraph::addToQueue(int v)
{
	if (inQueue.count(v) == 0 && v != edges.size() - 1 && v != 0)
	{
		inQueue.insert(v);
		unbalanced.push_back(v);
	}
}

void DinicGraph::push(int u)
{
	auto & vertex = network[u];
	for (int i = 0; i != network.size(); ++i)
	{
		if (vertex.e() == 0) return;
		if (i == u) continue;
		if (layerEdges[u][i].getCapacity() > 0 && !network[i].isBlocked())
		{
			int q = min(layerEdges[u][i].getCapacity(), vertex.e());
			network[i].addToStack(u, q);
			layerEdges[u][i].push(q);
			vertex.e() -= q;
			network[i].e() += q;
			//cout << "Pushed " << q << " on edge " << u << " " << i << '\n';
			addToQueue(i);
		}
	}
	if (vertex.e() != 0)
	{
		vertex.block();
	}
}

void DinicGraph::returnPush(int u)
{
	assert(u != network.size() - 1);
	auto & vertex = network[u];
	while (vertex.e() > 0)
	{
		auto flow = vertex.getTopOfStack();
		int q = min(flow.q, vertex.e());
		layerEdges[flow.e][u].push(-q);
		//cout << "Reversed " << q << " on edge " << flow.e << " " << u << '\n';
		vertex.e() -= q;
		network[flow.e].e() += q;
		/*if (inQueue.count(flow.e) == 0 && flow.e != 0)
		{
			inQueue.insert(flow.e);
			unbalanced.push_back(flow.e);
		}*/
		addToQueue(flow.e);
		if (q != flow.q)
		{
			vertex.addToStack(flow.e, flow.q - q);
		}
	}
}

bool DinicGraph::generateLayeredNetwork()
{
	//cout << "Generating Layered Network\n";
	bool gotToEnd = false;
	edgeLayers.clear();
	queue<pair<unsigned, unsigned>> q;
	vector<bool> examined(network.size(), false);
	vector<unsigned> toExamine;
	toExamine.resize(edges.size(), edges.size());
	q.push(make_pair(0, 0));
	while (!q.empty())
	{
		auto top = q.front();
		q.pop();
		if (examined[top.first]) continue; //already found
		examined[top.first] = true;
		for (unsigned i = 0; i != edges.size(); ++i)
		{
			if (toExamine[i] < (top.second + 1))
			{
				continue;
			}
			if (edges[top.first][i].getCapacity() > 0.0)
			{
				if (edgeLayers.size() <= top.second)
				{
					edgeLayers.resize(top.second + 1);
				}
				edgeLayers[top.second].push_back(make_pair(top.first, i));
				q.push(make_pair(i, top.second + 1));
				toExamine[i] = top.second + 1;
				if (i == edges.size() - 1)
				{
					gotToEnd = true;
				}
			}
		}
	}
	if (gotToEnd)
	{
		//remove all nodes that don't belong
		unsigned end = network.size() - 1;
		auto it = remove_if(edgeLayers.back().begin(), edgeLayers.back().end(), [end](auto e) {return e.second != end; });
		if (it != edgeLayers.back().end())
		{
			edgeLayers.back().erase(it, edgeLayers.back().end());
		}
		layerEdges = vector<vector<DinicEdgeLayer>>(edges.size(), vector<DinicEdgeLayer>(edges.size(), { 0 }));
		for (size_t i = 0; i != edgeLayers.size(); ++i)
		{
			for (size_t j = 0; j != edgeLayers[i].size(); ++j)
			{
				pair<int, int> key = edgeLayers[i][j];
				layerEdges[key.first][key.second].setCapacity(edges[key.first][key.second].getCapacity());
			}
		}

	}
	/*for (unsigned i = 0; i != edgeLayers.size(); ++i)
	{
		cout << "Layer " << i << "\n";
		for (unsigned j = 0; j != edgeLayers[i].size(); ++j)
		{
			cout << edgeLayers[i][j].first << " " << edgeLayers[i][j].second << '\n';
		}
	}*/
	return gotToEnd;
}

//Assign functions
void DinicGraph::pushStep1(int v)
{
	auto & vertex = networkP[v];
	int alpha = min(vertex.getE(), vertex.root(Tree::Out));
	if (alpha == 0) return;
	vertex.increaseE(-alpha);
	auto kp = vertex.find(alpha, Tree::Out);
	auto ends = vertex.getRange(kp.first);
	for (int j = vertex.kPrime(); j <= kp.first; ++j)
	{
		vertex.addJob(bind(&DinicGraph::pushStep2, this, v, ends[j - vertex.kPrime()], j, kp));
	}
	vertex.queueJob(bind(&DinicGraph::pushStep3, this, v, kp.first));
}
void DinicGraph::pushStep2(int v, int w, int j, const std::pair<int, int> & kp)
{
	auto & vertex = networkP[v];
	auto & vertexW = networkP[w];
	lock(mutexs[v], mutexs[w]);
	if (vertexW.isBlocked())
	{
		mutexs[v].unlock();
		mutexs[w].unlock();
		return;
	}
	int r = vertexW.getIndex(v, Tree::Access);

	vertexW.update(r, 1, Tree::Access); //2a
	int Sr = vertexW.sum(r, Tree::Access); //2b
	int q = vertex.getFromOut(j); //2c
	if (j == kp.first) q = kp.second;
	assert(q >= 0);
	layerEdges[v][w].push(q); //2d
	//cout << "Pushed " << q << " on edge " << v << w << endl;
	//unique_lock<mutex> lk(vertexW.lock());
	int total = vertexW.root(Tree::In); //2e
	vertexW.update(vertexW.hd() + Sr, q, Tree::In, v); //2f
	vertex.update(j, vertex.getFromOut(j) - q, Tree::Out); //2g
	vertexW.hd() += vertexW.root(Tree::Access); //2h
	vertexW.clear(r, Tree::Access); //2i
	//lk.unlock();
	vertexW.increaseE( vertexW.root(Tree::In) - total); //2j
	mutexs[v].unlock();
	mutexs[w].unlock();
}
void DinicGraph::pushStep3(int v, int k)
{
	networkP[v].kPrime() = k;
	if (networkP[v].getE() > 0)
		networkP[v].addJob(bind(&DinicGraph::pushStep4, this, v));
}
void DinicGraph::pushStep4(int v)
{
    networkP[v].block();
	for (int u : networkP[v].getEdgesIn())
	{
		networkP[u].update(networkP[u].getIndex(v, Tree::Out), 0, Tree::Out);
	}
}

void DinicGraph::returnStep1(int v)
{
	auto & vertex = networkP[v];
	int tempExcess = vertex.getE();
	auto kp = vertex.find(vertex.root(Tree::In) - vertex.getE(), Tree::In);
	vertex.setE(0);
	for (int j = kp.first; j <= vertex.hd(); ++j)
	{
		auto Q = vertex.getFromIn(j);
		if (Q.e == -1)
		{
			cout << "STOP ME" << endl;
		}
		//if (Q.q == 0) return;
		vertex.addJob(bind(&DinicGraph::returnStep2, this, v, Q.e, Q.q, j, kp));
	}
	vertex.queueJob(bind(&DinicGraph::returnStep3, this, v, kp.first));
}
void DinicGraph::returnStep2(int v, int u, int q, int j, const std::pair<int, int> & kp)
{
	auto & vertex = networkP[v];
	//2a
	int d = q;
	if (j == kp.first) d = q - kp.second; //ABSTACT THIS OUT
	if (d == 0) return;
	auto & vertexU = networkP[u];
	lock(mutexs[v], mutexs[u]);
	//2b
	if (j == kp.first) vertex.update(j, kp.second, Tree::In, u);
	else vertex.update(j, 0, Tree::In, u);
	//2c
	//cout << "Reversing " << d << " on edge" << u << v << endl;
	layerEdges[u][v].push(-d);
	//2d
	//2e do I need to do the edge stuff
	vertexU.update(vertexU.getIndex(v, Tree::Sum), d, Tree::Sum);
	//2f
	vertexU.increaseE(vertexU.root(Tree::Sum));
	//2g
	//2h
	vertexU.clear(vertexU.getIndex(v, Tree::Sum), Tree::Sum);
	mutexs[v].unlock();
	mutexs[u].unlock();
}
void DinicGraph::returnStep3(int v, int k)
{
	networkP[v].hd() = k;
}

void DinicGraph::cleanStep1(int v, int j)
{
	assert(false);
}
void DinicGraph::cleanStep2(int v)
{
	assert(false);
}

//else {
//	while (generateLayeredNetwork())
//	{
//
//		unbalanced.clear();
//		inQueue.clear();
//		//initialize step
//		//also takes care of previous clear
//		for (int i = 0; i != networkP.size(); ++i)
//		{
//			networkP[i].reset(layerEdges[i]);
//		}
//		//push
//		pushMaxP();
//		//loop
//		while (!unbalanced.empty())
//		{
//			//PUSH
//			for (int i = 0; i != unbalanced.size(); ++i)
//			{
//				networkP[unbalanced[i]].addJob(bind(&DinicGraph::pushStep1, this, unbalanced[i]));
//			}
//
//			addJobs();
//			while (!jobs.empty())
//			{
//				while (!jobs.empty())
//				{
//					//do the jobs
//					jobs.front()();
//					jobs.pop_front();
//				}
//				addJobs();
//			}
//			//RETURN
//			for (int i = 0; i != unbalanced.size(); ++i)
//			{
//				networkP[unbalanced[i]].addJob(bind(&DinicGraph::returnStep1, this, unbalanced[i]));
//			}
//			addJobs();
//			while (!jobs.empty())
//			{
//				while (!jobs.empty())
//				{
//					jobs.front()();
//					jobs.pop_front();
//				}
//				addJobs();
//			}
//			unbalanced.clear();
//			addUnbalanced();
//		}
//		//update the edges
//		for (int i = 0; i != edges.size(); ++i)
//		{
//			for (int j = 0; j != edges.size(); ++j)
//			{
//				edges[i][j].push(layerEdges[i][j].getFlow());
//			}
//		}
//	}