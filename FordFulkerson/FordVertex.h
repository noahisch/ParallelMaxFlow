#pragma once
#include<vector>
#include<cassert>

class FordVertex
{
public:
	inline FordVertex() :index(global_index++), isLabeled(false) {}
	inline int get_index() {return index; }
	inline void label(int source_, int value_) { isLabeled = true; source = source_; value = value_; }
	inline void pLabel(int source_, int value_) { source = source_; value = value_; }
	inline int get_pLabel() { return value; }
	inline int get_pSource() { return source; }
	inline int get_label() { assert(isLabeled); return value; }
	inline int get_source() { assert(isLabeled); return source; }
	inline bool is_labled() {return isLabeled; }
	inline void pLabel() { isLabeled = true; }
	inline void clear() { isLabeled = false; }
	static void clearIndex() { global_index = 0; }
private:
	static int global_index; //used for generating index
	int index;
	int source;
	int value;
	bool isLabeled;
};