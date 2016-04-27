#include "Edge.h"
#include <cassert>
#include <iostream>

Edge::Edge(int capacity_) :capacity(capacity_), currentCapacity(capacity_) {}

int Edge::pushMax()
{
	int flow = currentCapacity;
	currentCapacity = 0;
	return flow;
}

Edge::Edge(const Edge & other): capacity(other.capacity), currentCapacity(other.currentCapacity.load())
{
}

void Edge::push(int val)
{
	currentCapacity -= val;
	assert(currentCapacity >= 0);
}

int Edge::getFlow()
{
	return capacity - currentCapacity;
}

int Edge::getCapacity()
{
	return currentCapacity;
}
