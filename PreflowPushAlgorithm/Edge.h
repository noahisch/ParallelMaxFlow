#pragma once
#include <atomic>
class Edge
{
public:
	Edge(int capacity_);
	Edge(const Edge & other);
	int pushMax();
	void push(int val);
	int getFlow();
	int getCapacity();
private:
	int capacity;
	//int currentCapacity;
	std::atomic<int> currentCapacity;
};