#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "fib_heap.h"
#include "gtest/gtest.h"

#define UNIT_TESTING 0
#define NAIVE 0
#define CLASSIC 1
#define NDEBUG 1

using namespace fh;
using namespace std;

template<typename Out>
void split(const string &s, const char& delimiter, Out&& result) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delimiter)) {
		if (!item.empty())
			*(result++) = item;
	}
}

vector<string> split(const string &s, const char& delimiter) {
	vector<string> elems;
	split(s, delimiter, back_inserter(elems));
	return elems;
}

template<typename T>
void reset_vector(T& vec, const size_t size) {
	vec.clear();
	vec.reserve(size + 1);
	vec.insert(vec.end(), size + 1, nullptr);
}

int main(int argc, char* argv[]) {
#if UNIT_TESTING
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#else
	uint_least64_t delete_min_count = 0;
	uint_least64_t decrease_count = 0;
	uint_least64_t steps_delete_min_count = 0;
	uint_least64_t steps_decrease_count = 0;
	uint_least64_t naive_delete_min_count = 0;
	uint_least64_t naive_decrease_count = 0;
	uint_least64_t steps_naive_delete_min_count = 0;
	uint_least64_t steps_naive_decrease_count = 0;

	auto naive = false;
	auto fun = [&](const bool decrease, const bool operation_end) {
		if (decrease) {
			if (operation_end) {
				if (naive)
					++steps_naive_decrease_count;
				else
					++steps_decrease_count;
			}
			if (naive)
				++naive_decrease_count;
			else
				++decrease_count;
		}
		if (operation_end) {
			if (naive)
				++steps_naive_delete_min_count;
			else
				++steps_delete_min_count;
		}
		if (naive)
			++naive_delete_min_count;
		else
			++delete_min_count;
	};
	using heap_t = fibonacci_heap<int, decltype(fun)>;

	heap_t heap_classic(fun);
	heap_t heap_naive(fun);
	heap_naive.naive_implementation = true;

	string line;
	// Vector used for clarity and debugging instead of a pointer to a regularly
	// re-allocated array of node handles.
	vector<heap_t::node_t> remembered_nodes_naive;
	vector<heap_t::node_t> remembered_nodes_classic;
	size_t last_n = 0;
	auto first = true;
	ofstream ofs{ argv[1] };
	ifstream ifs{ "rand_uniform.err.txt" };

	while (!getline(ifs, line).eof()) {
#if NDEBUG
		//cout << line << endl;
#endif
		auto tokens = split(line, ' ');
		if (tokens[0] == string("#")) {
			if (!first) {
				ofs << last_n << " ";
#if CLASSIC
				ofs << float(delete_min_count) / steps_delete_min_count << " ";
				ofs << float(decrease_count) / steps_decrease_count << " ";
#endif
#if NAIVE
				ofs << float(naive_delete_min_count) / steps_naive_delete_min_count << " ";
				ofs << float(naive_decrease_count) / steps_decrease_count;
#endif
				ofs << endl;
				ofs << flush;
				cout << "FUCK THAT SHIT!" << endl;
			}
			// reset counters
			delete_min_count = 0;
			decrease_count = 0;
			naive_delete_min_count = 0;
			naive_decrease_count = 0;
			steps_delete_min_count = 0;
			steps_decrease_count = 0;
			steps_naive_delete_min_count = 0;
			steps_naive_decrease_count = 0;
			// prepare new run
			last_n = stoi(tokens[1]);
#if CLASSIC
			heap_classic.clear();
			reset_vector(remembered_nodes_classic, last_n);
#endif
#if NAIVE
			heap_naive.clear();
			reset_vector(remembered_nodes_naive, last_n);
#endif
			first = false;
		}
		else if (tokens[0] == string("I")) {
			const auto identification = stoi(tokens[1]);
			const auto priority = stoi(tokens[2]);
			heap_t::node_t handle;
#if CLASSIC
			naive = false;
			handle = heap_classic.insert(identification, priority);
			remembered_nodes_classic.at(identification) = handle;
#endif
#if NAIVE
			naive = true;
			handle = heap_naive.insert(identification, priority);
			remembered_nodes_naive.at(identification) = handle;
#endif
		}
		else if (tokens[0] == string("M")) {
			naive = false;
#if CLASSIC
			const auto min = heap_classic.delete_min();
			if (min) remembered_nodes_classic.at(min->value) = nullptr;
			delete min;
#endif
#if NAIVE
			naive = true;
			const auto min2 =  heap_naive.delete_min();
			if (min2) remembered_nodes_naive.at(min2->value) = nullptr;
			delete min2;
#endif
		}
		else { // if (tokens[0] == string("D"))
			const auto identification = stoi(tokens[1]);
			const auto priority = stoi(tokens[2]);
#if CLASSIC
			if (remembered_nodes_classic.at(identification) != nullptr &&
				remembered_nodes_classic.at(identification)->priority >= priority) {
				naive = false;
				heap_classic.decrease(remembered_nodes_classic.at(identification), priority);
			}
#endif
#if NAIVE
			if (remembered_nodes_naive.at(identification) != nullptr &&
				remembered_nodes_naive.at(identification)->priority >= priority) {
				naive = true;
				heap_naive.decrease(remembered_nodes_naive.at(identification), priority);
			}
#endif
		}
	}
	cout << "END OF SHIT" << endl;
#endif
}