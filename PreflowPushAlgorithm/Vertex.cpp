#include "Vertex.h"

Vertex::Vertex() :internal_h(0), internal_e(0) {}

void Vertex::setHeight(int h_)
{
	internal_h = h_;
}

int Vertex::getHeight()
{
	return internal_h;
}
Vertex::Vertex(const Vertex & other):internal_e(other.internal_e.load()), internal_h(other.internal_h.load())
{

}

int Vertex::h()
{
	return internal_h;
}

std::atomic<int> & Vertex::e()
{
	return internal_e;
}