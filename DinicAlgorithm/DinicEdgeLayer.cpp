#include "DinicEdgeLayer.h"
#include <cassert>
DinicEdgeLayer::DinicEdgeLayer(int capacity_) :capacity(capacity_), currentyCapacity(capacity_) {}
void DinicEdgeLayer::setCapacity(int capacity_)
{
	assert(getFlow() == 0);
	capacity = capacity_;
	currentyCapacity = capacity_;
}
void DinicEdgeLayer::pushMax()
{
	currentyCapacity = 0;
}
void DinicEdgeLayer::push(int val)
{
	currentyCapacity -= val;
}
	int DinicEdgeLayer::getFlow() const
	{
		return capacity - currentyCapacity;
	}
	int DinicEdgeLayer::getCapacity() const
	{
		return currentyCapacity;
	}