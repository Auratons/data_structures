#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <exception>
#include "fib_heap.h"

// NDEBUG is (not) defined in fib_heap.h, if not defined, debug configuration is
// built including many additional integrity checks (several times slower).

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
void reset_vector(T& vec, unsigned long size);

template<typename T>
void reset_vector(T& vec, const unsigned long size) {
	vec.clear();
	vec.reserve(size + 1);
	vec.insert(vec.end(), size + 1, nullptr);
}

int main(int argc, char* argv[]) {
	bool only_standard = false;
	if (argc < 2) {
		throw runtime_error("One argument -- output file name.");
	}
	if (argc == 3) {
		// For uniform and nonuniform tests.
		only_standard = true;
	}
	uint_least64_t delete_min_count = 0;
	uint_least64_t decrease_count = 0;
	uint_least64_t steps_delete_min_count = 0;
	uint_least64_t steps_decrease_count = 0;
	uint_least64_t naive_delete_min_count = 0;
	uint_least64_t naive_decrease_count = 0;
	uint_least64_t steps_naive_delete_min_count = 0;
	uint_least64_t steps_naive_decrease_count = 0;

	auto reset_counters = [&]() {
        delete_min_count = 0;
        decrease_count = 0;
        naive_delete_min_count = 0;
        naive_decrease_count = 0;
        steps_delete_min_count = 0;
        steps_decrease_count = 0;
        steps_naive_delete_min_count = 0;
        steps_naive_decrease_count = 0;
	};

	auto naive = false;
	auto fun = [&](const bool decrease, const bool operation_end) {
		if (decrease) {
			if (operation_end) {
				if (naive)
					++steps_naive_decrease_count;
				else
					++steps_decrease_count;
			}
			else {
				if (naive)
					++naive_decrease_count;
				else
					++decrease_count;
			}
		}
		else {
			if (operation_end) {
				if (naive)
					++steps_naive_delete_min_count;
				else
					++steps_delete_min_count;
			}
			else {
				if (naive)
					++naive_delete_min_count;
				else
					++delete_min_count;
			}
		}
	};
	using heap_t = fibonacci_heap<unsigned long, decltype(fun)>;

	heap_t heap_classic(fun);
	heap_t heap_naive(fun);
	heap_naive.naive_implementation = true;

	string line;
	// Vector used for clarity and debugging instead of a pointer to a regularly
	// re-allocated array of node handles.
	vector<heap_t::node_t> remembered_nodes_naive;
	vector<heap_t::node_t> remembered_nodes_classic;
					#ifndef NDEBUG
					vector<heap_t::node_t> stl_heap;
					struct cmp{
						bool operator()(const heap_t::node_t a, const heap_t::node_t b) const {
							return -a->priority < -b->priority;
						}
					};
					#endif
    size_t last_n = 0;
	volatile auto first = true;
	ofstream ofs{ argv[1] };
	ifstream ifs{ "/Users/kremel/school/data_structures_muj_pc/fibonacci_heap/src/deep.txt" };
	//bool yet = false;

	while (!getline(ifs, line).eof()) {
//		if (!yet || line != string("# 472392")){
//			yet = true;
//			continue;
//		}
        //cout << line << endl;
		auto tokens = split(line, ' ');
		if (tokens[0] == string("#")) {

			//cout <<"line " << line << "N " << size_t(stoi(tokens[1])) << "   ";
			if (!first) {
				ofs << last_n << " ";
				ofs << float(delete_min_count) / steps_delete_min_count << " ";
				ofs << float(decrease_count) / steps_decrease_count << " ";
				if (!only_standard) {
					ofs << float(naive_delete_min_count) / steps_naive_delete_min_count << " ";
					ofs << float(naive_decrease_count) / steps_decrease_count;
				}
				ofs << endl;
				ofs << flush;
				cout << "TREE FINISHED!" << endl;
			}
			// reset counters
			reset_counters();
			// prepare next run
			last_n = stoul(tokens[1]);
			heap_classic.clear();
			reset_vector(remembered_nodes_classic, last_n);
			if (!only_standard) {
				heap_naive.clear();
				reset_vector(remembered_nodes_naive, last_n);
			}
					#ifndef NDEBUG
					stl_heap.clear();
					#endif
			first = false;
		}
		else if (tokens[0] == string("I")) {
			const auto identification = stoul(tokens[1]);
			const auto priority = stoi(tokens[2]);
			heap_t::node_t handle;
					#ifndef NDEBUG
					auto x = new heap_t::node(identification, priority);
					stl_heap.push_back(x);
					std::push_heap(stl_heap.begin(), stl_heap.end(), cmp());
					#endif
			naive = false;
			handle = heap_classic.insert(identification, priority);
			remembered_nodes_classic.at(identification) = handle;
			if (!only_standard) {
				naive = true;
				handle = heap_naive.insert(identification, priority);
				remembered_nodes_naive.at(identification) = handle;
			}
		}
		else if (tokens[0] == string("M")) {
					#ifndef NDEBUG
					std::pop_heap(stl_heap.begin(), stl_heap.end(), cmp());
					auto ground = stl_heap.back();
					stl_heap.pop_back();
					#endif

            naive = false;
			const auto min = heap_classic.delete_min();
			if (min) remembered_nodes_classic.at(min->value) = nullptr;

					#ifndef NDEBUG
					if (min && min->priority != ground->priority)
						throw runtime_error("Classic minimum differs from STL.");
					#endif

			delete min;

        	if (!only_standard) {
				naive = true;
				const auto min2 = heap_naive.delete_min();
				if (min2) remembered_nodes_naive.at(min2->value) = nullptr;

					#ifndef NDEBUG
					if (min2 && min2->priority != ground->priority)
						throw runtime_error("Naive minimum differs from STL.");
					#endif

				delete min2;
			}

					#ifndef NDEBUG
					delete ground;
					#endif
		}
		else { // if (tokens[0] == string("D"))
			const auto identification = stoul(tokens[1]);
			const auto priority = stoi(tokens[2]);
					#ifndef NDEBUG
					auto to_lower = std::find_if(stl_heap.begin(), stl_heap.end(), [identification](auto it) {
						return it->value == identification;
					});
					if (to_lower != stl_heap.end()){
						(*to_lower)->priority = priority;
						std::make_heap(stl_heap.begin(), stl_heap.end(), cmp());
					}
					#endif
			if (remembered_nodes_classic.at(identification) &&
				remembered_nodes_classic.at(identification)->priority >= priority) {
				naive = false;
				heap_classic.decrease(remembered_nodes_classic.at(identification), priority);
			}
			if (!only_standard) {
				if (remembered_nodes_naive.at(identification) &&
					remembered_nodes_naive.at(identification)->priority >= priority) {
					naive = true;
					heap_naive.decrease(remembered_nodes_naive.at(identification), priority);
				}
			}
		}
	}
	cout << "END OF INPUT" << endl;
}