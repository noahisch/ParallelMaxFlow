#pragma once
class DinicEdge
{
public:
	DinicEdge(int capacity_);
	int getCapacity() const;
	int getFlow() const;
	void push(int flow);
private:
	int capacity;
	int currentCapacity;
};