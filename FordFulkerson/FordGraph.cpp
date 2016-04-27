#include "FordGraph.h"
#include "FordVertex.h"
#include <algorithm>
#include <limits>
#include <iterator>
#include <cassert>


using namespace std;

//int NO_EDGE = numeric_limits<int>::min();

ThreadPool::ThreadPool()
{
	done = false;
	callbackBool = true;
	threads.reserve(int(thread::hardware_concurrency()));
	for (int i = 0; i < thread::hardware_concurrency(); ++i)
	{
		threads.emplace_back(bind(&ThreadPool::loop_function, this));
	}
}
void ThreadPool::AddJob(std::function<void()> job)
{
	{
		unique_lock<mutex> lk(WaitingQueueMutex);
		WaitingQueue.push(job);
	}
	//QueueCV.notify_one();
}

bool ThreadPool::jobs()
{
	return Queue.empty() && WaitingQueue.empty(); //do I need to get the lock?
}

void ThreadPool::activateJobs(std::function<void()> callback_)
{
	assert(Queue.empty());
	{
		unique_lock<mutex> lk(WaitingQueueMutex);
		unique_lock<mutex> lk2(QueueMutex);
		callbackBool = false;
		Queue = WaitingQueue;
		WaitingQueue = queue<function<void()>>();
		callback = callback_;
	}
	QueueCV.notify_all();
}

void ThreadPool::clearJobs()
{
	Queue = queue<function<void()>>();
	WaitingQueue = Queue;
}

void ThreadPool::loop_function()
{
	while (true)
	{
		{
			if (done) return;
			if (running == 0 && !callbackBool)
			{
				unique_lock<mutex> lk2(callBackLock);
				unique_lock<mutex> lk(QueueMutex);
				if (Queue.empty() && !callbackBool)
				{
					callbackBool = true;
					lk2.unlock();
					lk.unlock();
					Queue.push(callback);
					QueueCV.notify_one();
				}
			}
			unique_lock<mutex> lk(QueueMutex);
			QueueCV.wait(lk, [this]() {return !Queue.empty() || done == true; });
			if (done) 
			{
				lk.unlock();
				return;
			}
			auto job = Queue.front();
			Queue.pop();
			lk.unlock();
			++running;
			job();
			--running;
		}
	}
}

void ThreadPool::end()
{
	done = true;
	clearJobs();
	QueueCV.notify_all();
	for (auto & t : threads)
	{
		t.join();
	}
	//clear the threads
}

void FordGraph::makeParallel()
{
	isParallel = true;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

int FordGraph::computeMaxFlow()
{
	if (!isParallel) pool.end();
	mutexs.swap(vector<mutex>(vertices.size()));
	maxFlow = getMaxFlow();
	while (true)
	{
		step1();
		if (isParallel)
		{
			if (better_parallel_step2())
				return maxFlow - getMaxFlow();
		}
		else{
			if(step2())
			return maxFlow - getMaxFlow();
		}
		step3();
	}
	return -1;
}


void FordGraph::from_label_vertex_spawn_threads(int u)
{
	//check every edge at once
	for (size_t i = 0; i < start.size(); ++i)
	{
		for (size_t j = 0; j < start[i].size(); ++j)
		{
			if (current[i][j] == start[i][j]) continue; //NO FLOW on edge
			parallel_edge(i, j);

		}
	}
	//end
}

void FordGraph::pLabel(int index, int source, int amount)
{
	unique_lock<mutex> lock(labelLock);
	vertices[index]->pLabel(source, amount);
	lock.unlock();
	move_to_labeled_and_unscanned.insert(index);
	move_to_labeled_and_scanned.insert(source);
}

void FordGraph::parallel_edge(int u, int v)
{
	if (vertices[u]->is_labled() && labeled_and_scanned.count(u) == 0 && !vertices[v]->is_labled())
	{
		if (current[u][v] != 0)
		{
			pLabel(v, u, min(vertices[u]->get_label(), current[u][v]));
		}
	}
	if (!vertices[u]->is_labled() && labeled_and_scanned.count(v) == 0 && vertices[v]->is_labled())
	{
		if (getFlow(u, v))
		{
			pLabel(u, v, -min(vertices[v]->get_label(), getFlow(u, v)));
		}
	}
}

void FordGraph::getLocks(int u, int v)
{
	assert(u != v);
	if (u < v)//prevent deadlock
	{
		mutexs[u].lock();
		mutexs[v].lock();
	}
	else
	{
		mutexs[v].lock();
		mutexs[u].lock();
	}
}

void FordGraph::unlockLocks(int u, int v)
{
	mutexs[v].unlock();
	mutexs[u].unlock();
}

//need 2O(m)

//pushes flow from v to u
void FordGraph::parallelEdgeBackward(int u_, int v_)
{
	auto & u = vertices[u_];
	auto & v = vertices[v_];
	if (!u->is_labled() && !scanned[v_] && v->is_labled() && getFlow(u_, v_))
	{
		getLocks(u_, v_);
		if (!u->is_labled() && !scanned[v_] && v->is_labled() && getFlow(u_, v_)) //check twice
		{
			//cout << "Pushing backwards on " << u_ << " " << v_ << endl;
			was_scanned[v_] = true;
			++scannedVertices;
			u->label(v_, -min(v->get_label(), getFlow(u_, v_)));
			++labeledVertices;
			unlockLocks(u_, v_);
			
			//create Jobs
			for (int i = 0; i < vertices.size(); ++i)
			{
				if (current[v_][i] != 0 && scanned[i] == false) {
					jobs.insert(make_pair(v_, i));
					pool.AddJob(bind(&FordGraph::parallelEdgeForward, this, v_, i));
				}
				if (getFlow(i, v_) > 0 && scanned[i] == false) {
					jobs.insert(make_pair(v_, i));
					pool.AddJob(bind(&FordGraph::parallelEdgeBackward, this, v_, i));
				}
				//if (current[v_][i] != 0)
					//pool.AddJob(bind(&FordGraph::parallelEdgeForward, this, v_, i));
				//if (getFlow(v_, i) > 0)
					//pool.AddJob(bind(&FordGraph::parallelEdgeBackward, this, v_, i));
			}
			return;
			
		}
		unlockLocks(u_, v_);
	}
	return;
	//while (!parallelReturn)
	//{
	//	//wait
	//	if (!u->is_labled() && !scanned[v_] && v->is_labled() && getFlow(u_, v_))
	//	{
	//		getLocks(u_, v_);
	//		if (!u->is_labled() && !scanned[v_] && v->is_labled() && getFlow(u_, v_)) //check twice
	//		{
	//			scanned[v_] = true;
	//			++scannedVertices;
	//			u->label(v_, -min(v->get_label(), getFlow(u_, v_)));
	//			//wake up the u threads
	//			++labeledVertices;
	//			if (scannedVertices == labeledVertices)
	//			{
	//				step2Cv.notify_all();
	//			}
	//			cvs[u_].notify_all();
	//		}
	//		unlockLocks(u_, v_);
	//	}
	//	//wait on v
	//	unique_lock<mutex> lk(mutexs[v_]);
	//	cvs[v_].wait(lk, [this, v_]() {return scanned[v_] == false && vertices.back()->is_labled() == false; });
	//}
}

//pushes flow from v to u
void FordGraph::parallelEdgeForward(int u_, int v_)
{
	//uses waiting
	//cout << "staring a foward edge call on edge " << u_ << " " << v_ << endl;
	auto & u = vertices[u_];
	auto & v = vertices[v_];
	if (u->is_labled() && !scanned[u_] && !v->is_labled() && current[u_][v_] != 0)
	{
		getLocks(u_, v_);
		if (u->is_labled() && !scanned[u_] && !v->is_labled() && current[u_][v_] != 0) //check twice
		{
			//cout << "Pushing forwards on " << u_ << " " << v_ << endl;
			was_scanned[u_] = true;
			++scannedVertices;
			v->label(u_, min(u->get_label(), current[u_][v_]));
			++labeledVertices;
			unlockLocks(u_, v_);
			for (int i = 0; i < vertices.size(); ++i)
			{
				if (current[v_][i] != 0) {
					//jobs.insert(make_pair(v_, i));
					pool.AddJob(bind(&FordGraph::parallelEdgeForward, this, v_, i));
				}
				if (getFlow(i, v_) > 0) {
					//jobs.insert(make_pair(v_, i));
					pool.AddJob(bind(&FordGraph::parallelEdgeBackward, this, v_, i));
				}
			}
			return;
		}
		unlockLocks(u_, v_);
	}
	return;

}

void FordGraph::better_parallel_step2_helper()
{
	//commit changes
	for (unsigned i = 0; i != scanned.size(); ++i)
	{
		if (was_scanned[i] == true) scanned[i] = true;
	}
	was_scanned.swap(vector<atomic<bool>>(vertices.size()));
	jobs.clear();
	//check end
	if (vertices.back() -> is_labled() || pool.jobs())
	{
		step2Cv.notify_one();
		return;
	}
	//add self as callback
	pool.activateJobs(bind(&FordGraph::better_parallel_step2_helper, this));
}


bool FordGraph::better_parallel_step2()
{
	//lets go
	pool.clearJobs();
	labeledVertices = 1;
	scannedVertices = 0;
	scanned.swap(vector<atomic<bool>>(vertices.size()));
	was_scanned.swap(vector<atomic<bool>>(vertices.size()));
	bool start = false;
	for (int i = 1; i < vertices.size(); ++i)
	{
		if (current[0][i] != 0)
		{
			start = true;
			jobs.insert(make_pair(0, i));
			pool.AddJob(bind(&FordGraph::parallelEdgeForward, this, 0, i));
		}
	}
	if (!start)
	{
		pool.end();
		return true;
	}
	//Run jobs
	pool.activateJobs(bind(&FordGraph::better_parallel_step2_helper, this));
	unique_lock<mutex> lk(step2Mutex);
	step2Cv.wait(lk);
	//cout << "Ended a step 2" << endl;
	if (vertices.back()->is_labled() == true)
		return false;
	pool.end();
	return true;
}

bool FordGraph::parallel_step2()
{
	move_to_labeled_and_unscanned.clear();
	move_to_labeled_and_scanned.clear();
	if (labeled_and_unscanned.empty())
	{
		return true;
	}
	vector<thread> threads;
	for (size_t i = 0; i < start.size(); ++i)
	{
		for (size_t j = 0; j < start[i].size(); ++j)
		{
			if (start[i][j] != 0)
			{
				threads.emplace_back(&FordGraph::parallel_edge, this, i, j);
			}

		}
	}
	for (auto & threadObj : threads)
	{
		if (threadObj.joinable())
			threadObj.join();
	}
	for (int x : move_to_labeled_and_unscanned) //parallel this
	{
		assert(unlabeled.count(x) == 1);
		unlabeled.erase(x);
		vertices[x]->pLabel();
		labeled_and_unscanned.insert(x);

	}
	for (int x : move_to_labeled_and_scanned) //and this
	{
		assert(labeled_and_unscanned.count(x) == 1);
		labeled_and_unscanned.erase(x);
		labeled_and_scanned.insert(x);
	}
	if (vertices.back()->is_labled()) return false;
	if (move_to_labeled_and_unscanned.empty() && move_to_labeled_and_scanned.empty()) return true;
	return parallel_step2();

}

std::istream& operator >> (std::istream & is, FordGraph & graph)
{
	int nodes;
	is >> nodes;
	assert(is);
	//init data
	graph.vertices.reserve(nodes);
	for (unsigned i = 0; i != nodes; ++i)
	{
		graph.vertices.emplace_back(new FordVertex());
		graph.starter.insert(i);
		
	}
	graph.current = vector<vector<int>>(nodes, vector<int>(nodes, 0));
	graph.start = vector<vector<int>>(nodes, vector<int>(nodes, 0));

	for (int i = 0; i != nodes - 1; ++i)
	{
		for (int j = 0; j != nodes; ++j)
		{
			int capacity;
			is >> capacity;
			graph.current[i][j] = capacity;
			graph.start[i][j] = capacity;
		}
	}
	return is;
}

int FordGraph::getMaxFlow()
{
	int sum = 0;
	for (size_t i = 0; i != vertices.size(); ++i)
	{
			sum += current[0][i];
	}
	return sum;
}

void FordGraph::label(int index, int source, int value)
{
	unique_lock<mutex> lock(labelLock);
	//lock.lock();
	assert(value != 0);
	unlabeled.erase(index);
	labeled_and_unscanned.insert(index);
	vertices[index]->label(source, value);
	
}

void FordGraph::step1()
{
	//does this happen here or outside?
	unlabeled = starter;
	for (int x : unlabeled)
	{
		vertices[x]->clear();
	}
	labeled_and_scanned.clear();
	labeled_and_unscanned.clear();
	//label S
	label(0, 0, numeric_limits<int>::max());
}




bool FordGraph::step2()
{
	if (labeled_and_unscanned.empty()) return true;
	
	//graph random vertex
	auto it = labeled_and_unscanned.begin();
	int u = vertices[*it]->get_index();
	assert(u == *it);
	labeled_and_unscanned.erase(it);
	auto forward_neighbors = forwardNeighbors(u);
	for (auto v : forward_neighbors)
	{
		if (current[u][v] > 0)
		{
			label(v, u, min(vertices[u]->get_label(), current[u][v]));
		}
	}
	for (auto v : backwardNeighbors(forward_neighbors, u))
	{
		label(v, u, -min(vertices[u]->get_label(), getFlow(v, u)));
	}
	if (vertices.back()->is_labled())
	{
		return false; //go to step 3
	}
	return step2();//tail recursion
}





vector<int> FordGraph::forwardNeighbors(int index)
{
	vector<int> N;
	for (size_t i = 0; i != vertices.size(); ++i)
	{
		if (current[index][i] > 0 && !vertices[i]->is_labled())
			N.push_back(i);
	}
	return N;
}

vector<int> FordGraph::backwardNeighbors(vector<int> candidates, int index)
{
	vector<int> N;
	for(int i : candidates)
	{
		if (getFlow(i, index) > 0 && !vertices[i]->is_labled())
			N.push_back(i);
	}
	return N;
}
void FordGraph::step3()
{
	int x = vertices.size() - 1;
	//cout << "Pushed " << vertices.back()->get_label() << " flow" << endl;
	while (x != 0)
	{
		int y = vertices[x]->get_source();
		int labelValue = vertices[x]->get_label();
		if (labelValue > 0)
		{
			updateFlow(y, x, vertices.back()->get_label());
		}
		else
		{
			updateFlow(x, y, -(vertices.back()->get_label()));
		}
		x = y;
	}
}

int FordGraph::getFlow(int u, int v)
{
	return start[u][v] - current[u][v];
}
void FordGraph::updateFlow(int u, int v, int amount)
{
	current[u][v] -= amount;
	assert(current[u][v] >= 0);
	assert(current[u][v] <= start[u][v]);
	assert(start[u][v] != 0);
	//cout << "Pushed " << amount << " from " << u << " to " << v << '\n';
}