#pragma once
#include <atomic>
class Vertex
{
public:
	Vertex();
	Vertex(const Vertex & other);
	void setHeight(int h_);
	//void push();
	//void lift();
	int getHeight();
	int h();
	std::atomic<int> & e();

private:
	//int h;
	std::atomic<int> internal_h;
	//int internal_e;
	std::atomic<int> internal_e;

};