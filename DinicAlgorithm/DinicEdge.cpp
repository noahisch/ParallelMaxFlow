#include "DinicEdge.h"
DinicEdge::DinicEdge(int capacity_): capacity(capacity_), currentCapacity(capacity_)
{

}
int DinicEdge::getCapacity() const
{
	return currentCapacity;
}
void DinicEdge::push(int flow)
{
	currentCapacity -= flow;
}
int DinicEdge::getFlow()const
{
	return capacity - currentCapacity;
}