#pragma once
class DinicEdgeLayer
{
public:
	DinicEdgeLayer(int capacity_);
	void setCapacity(int capacity_);
	void pushMax();
	void push(int val);
	int getFlow() const;
	int getCapacity() const;
private:
	int capacity;
	int currentyCapacity;
};