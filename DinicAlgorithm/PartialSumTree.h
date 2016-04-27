#ifndef PARTIAL_SUM_TREE_H
#define PARTIAL_SUM_TREE_H

#include <utility>
#include <cmath>
#include <vector>
#include <deque>
#include <exception>
#include <iostream>
#include <cassert>
#include <type_traits>

class PSTTest
{
public:
	PSTTest(double i_) :i(i_) {}
	operator int() const { return int(i); };
	bool operator == (const PSTTest& other) const
	{
		return i == other.i;
	}
private:
	double i;
};
template <typename T>
class PartialSumTree
{
public:
	PartialSumTree() {}
	PartialSumTree(std::vector<T> input);
	//PartialSumTree(std::deque<T> input);

	void clear(int i);
	void update(int i, int value);
	void updateValue(int i, T value);
	int sum(int i);
	std::pair<int, int> find(int alpha);
	std::size_t size();
	int root() { if (tree.size() == 0) return 0;  return tree[0]; } //will need to test this
	T operator[](int i) { assert(i <= size());  return getT(1, i).getT(); }
	friend std::ostream& operator << (std::ostream& os, const PartialSumTree& pst)
	{
		for (int j = 0; j <= pst.getIndex(1, int(pst.activeLeaves)); ++j)
		{
			os << pst.tree[j] << " ";
			int temp = log2(j + 1);
			if (pow(2, temp) == j + 1)
			{
				os << '\n';
			}
		}
		return os;
	}
	static void test()
	{
		std::vector<PSTTest> v = { 5,2,4,7,1,6,3 };
		PartialSumTree<PSTTest> pst(v);
		std::vector<int> answer = { 28,18,10,7,11,7,3,5,2,4,7,1,6,3,0 };
		verify(answer, pst.tree);
		pst.clear(2);
		answer = { 0,0,10,0,11,7,3,5,0,4,7,1,6,3,0 };
		verify(answer, pst.tree);
		assert(pst.root() == 0);
		pst.update(1, 6);
		answer = { 27, 17, 10, 6, 11, 7, 3, 6, 0, 4, 7, 1, 6, 3, 0 };
		assert(pst.root() == 27);
		verify(answer, pst.tree);
		pst.update(4, 12);
		answer = { 32, 22, 10, 6, 16, 7, 3, 6, 0, 4, 12, 1, 6, 3, 0 };
		verify(answer, pst.tree);
		pst.clear(4);
		pst.update(4, 7);
		assert(pst[4] == PSTTest(7));
		pst.update(2, 2);
		assert(pst[2] == PSTTest(2));
		pst.clear(1);
		pst.update(1, 5);
		answer = { 28,18,10,7,11,7,3,5,2,4,7,1,6,3,0 };
		verify(answer, pst.tree);
		assert(pst.sum(4) == 18);
		assert(pst.find(8) == std::make_pair(3, 1));
		assert(pst.find(19) == std::make_pair(5, 1));
		std::vector<PSTTest> v2 = { 1 };
		PartialSumTree<PSTTest> pst2(v2);
		assert(pst2.treeHeight == 2);
	}

private:
	std::size_t activeLeaves;
	int treeHeight;
public:
	class Node
	{
	public:
		Node() : data_int(0), type(Type::Int) {}
		Node(T t) :data_type(t), type(Type::Typename) {}
		Node(int i) :data_int(i), type(Type::Int) {}
		Node& operator=(T& other)
		{
			type = Type::Typename;
			data_type = other;
			return *this;
		}
		Node& operator=(Node& other)
		{
			type = other.type;
			switch (other.type)
			{
			case Type::Int:
				data_int = other.data_int;
				break;
			case Type::Typename:
				data_type = other.data_type;
				break;
			}
			return *this;
		}
		Node& operator=(int other)
		{
			switch (type)
			{
			case Type::Int:
				data_int = other;
				break;
			case Type::Typename:
				data_type = other;
				break;
			}
			return *this;
		}
		Node(Node& other) :type(other.type)
		{
			switch (type)
			{
			case Type::Int:
				data_int = other.data_int;
				break;
			case Type::Typename:
				data_type = other.data_type;
				break;
			}
		}
		operator int() const
		{
			switch (type)
			{
			case Type::Int:
				return data_int;
				break;
			case Type::Typename:
				return data_type;
				break;
			}
		}
		T getT() const
		{
			assert(type == Type::Typename);
			return data_type;
		}
		friend int operator+(Node& lhs, Node& rhs)
		{
			int i = lhs;
			int i_2 = rhs;
			return i + i_2;
		}
		~Node()
		{
			if (type == Type::Typename) data_type.~T();
		}
	private:
		union
		{
			int data_int;
			T data_type;
		};
		enum class Type { Int, Typename };
		Type type;
		
		
	};
	private:
	std::vector<Node> tree;
	int getIndex(int height, int index) const;
	const Node& getT(int height, int index);
	Node& setT(int height, int index);
	void updateElement(int index);
	static bool verify(std::vector<int> a, std::vector<Node> b)
	{
		assert(a.size() == b.size());
		for (unsigned i = 0; i != a.size(); ++i)
		{
			if (a[i] != int(b[i])) return false;
		}
		return true;
	}

};







struct TreeConstructorError : std::exception {
	const char* what() const noexcept { return "Error trying to access an edge that does not exist"; }
};



//IMPLEMENTATION
using std::pair;
using std::make_pair;

//template <typename T>
//PartialSumTree<T>::PartialSumTree(std::deque<T> input)
//{
//	bool test = std::is_convertible<T, int>::value;
//	assert(test);
//	auto size = activeLeaves;
//	//set the height
//	treeHeight = ceil(log2(size)) + 1;
//	//active leaves size
//	tree.resize(pow(2, int(ceil(log2(size)) + 1)) - 1);
//	//insert all values
//	int offset = pow(2, int(ceil(log2(size)))) - 1;
//	for (unsigned i = 0; i != size; ++i)
//	{
//		tree[i + offset] = input[i];
//		//if (first == last) throw TreeConstructorError();
//	}
//	//update all values above
//	--offset;
//	for (; offset >= 0; --offset)
//	{
//		updateElement(offset);
//	}
//}

template <typename T>
PartialSumTree<T>::PartialSumTree(std::vector<T> input) : activeLeaves(input.size())
{
	if (input.size() == 0)
	{
		//no in/out edges
		treeHeight = 0;
		return;
	}
	bool test = std::is_convertible<T, int>::value;
	assert(test);
	auto size = activeLeaves;
	//set the height
	treeHeight = ceil(log2(size)) + 1;
	if (size == 1) treeHeight = 2;
	//active leaves size
	tree.resize(pow(2, int(ceil(log2(size)) + 1)) - 1);
	if (size == 1) tree.resize(3);
	//insert all values
	int offset = pow(2, int(ceil(log2(size)))) - 1;
	if (size == 1) offset = 1;
	for (unsigned i = 0; i != size; ++i)
	{
		assert(int(input[i]) >= 0);
		tree[i + offset] = input[i];
		//if (first == last) throw TreeConstructorError();
	}
	//update all values above
	--offset;
	for (; offset >= 0; --offset)
	{
		updateElement(offset);
	}
}


template <typename T>
void PartialSumTree<T>::clear(int i)
{
	int j = 1;
	while (j <= treeHeight)
	{
		setT(j, ceil(i / pow(2, j - 1))) = 0;
		assert(getT(j, ceil(i / pow(2, j - 1))) >= 0);
		++j;
	}
}
template <typename T>
void PartialSumTree<T>::updateValue(int i, T value)
{
	setT(1, i) = value;
	assert(getT(1, i) >= 0);
	int j = 2;
	while (j <= treeHeight)
	{
		int pos = ceil(i / pow(2, j - 1));
		setT(j, pos) = getT(j - 1, 2 * pos - 1) + getT(j - 1, 2 * pos);
		assert(getT(j, pos) >= 0);
		++j;
	}
}

template <typename T>
void PartialSumTree<T>::update(int i, int value)
{
	setT(1, i) = value;
	assert(getT(1, i) >= 0);
	int j = 2;
	while (j <= treeHeight)
	{
		int pos = ceil(i / pow(2, j - 1));
		setT(j, pos) = getT(j - 1, 2 * pos - 1) + getT(j - 1, 2 * pos);
		assert(getT(j, pos) >= 0);
		++j;
	}
}
template <typename T>
int PartialSumTree<T>::sum(int i)
{
	int Si = getT(1, i);
	int j = 2;
	while (j <= treeHeight)
	{
		int pos = ceil(i / pow(2, j - 2));
		if (2 * ceil(i / pow(2, j - 1)) == pos)
		{
			Si += getT(j - 1, pos - 1);
		}
		++j;
	}
	return Si;
}
template <typename T>
std::pair<int, int> PartialSumTree<T>::find(int alpha)
{
	int j = treeHeight;
	int k = 1;
	int p = alpha;
	while (j > 1)
	{
		if (p > int(getT(j - 1, 2 * k - 1)))
		{
			p -= getT(j - 1, 2 * k - 1);
			assert(getT(j - 1, 2 * k - 1) >= 0);
			k *= 2;
		}
		else
		{
			k *= 2;
			--k;
		}
		--j;
	}
	return make_pair(k, p);
}
template <typename T>
std::size_t PartialSumTree<T>::size()
{
	return activeLeaves;
}
template <typename T>
int PartialSumTree<T>::getIndex(int height, int index) const
{
	//calculate index
	int tempHeight = treeHeight - height;
	int level = pow(2, tempHeight) - 1;
	return level + index - 1;
	//get offset

}
template <typename T>
typename const PartialSumTree<T>::Node& PartialSumTree<T>::getT(int height, int index)
{
	return setT(height, index);
}
template <typename T>
typename PartialSumTree<T>::Node& PartialSumTree<T>::setT(int height, int index)
{
	assert(index > 0);
	assert(index <= size()+1);
	return tree[getIndex(height, index)];
}
template <typename T>
void PartialSumTree<T>::updateElement(int index)
{
	assert(index < (tree.size() + 1 / 2));
	tree[index] = tree[2 * (index + 1)] + tree[2 * (index + 1) - 1]; //this is probably wrong

}



#endif