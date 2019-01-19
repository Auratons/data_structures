#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include "bloom_filter.h"
#include "looping.h"

using namespace std;
using namespace bf;

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

int main(int, char** argv) {
	constexpr const auto step_size = 10000;
	vector<int> functions{ 1, 2, 3, 4, 10, 15 };
	vector<int> sizes(600000 / step_size);
	generate(begin(sizes), end(sizes), [=, i = 0]() mutable { return ++i * step_size; });

	const auto output_file_name = string(argv[1]);
	ofstream ofs{ output_file_name };
	if (!ofs.is_open())
		throw invalid_argument("File " + output_file_name + " could not be opened for writing.");
	
	const auto input_file_name = string("ascii_shakespeare_squeezed_whitespaces.txt");
	ifstream ifs{ input_file_name };
	if (!ifs.is_open())
		throw invalid_argument("File " + input_file_name + " could not be opened for reading.");

	const auto f = [](auto&& bf, ifstream& ifs, ofstream& ofs, const uint32_t i) {
		string line;
		int_least64_t insert_counter = 0;
		ifs.clear();            // clear fail and eof bits
		ifs.seekg(0, ios::beg); // back to the start!
		while (getline(ifs, line)) {
			auto tokens = split(line, ' ');
			for (auto&& token : tokens) {
				if (bf.insert(token.c_str(), token.length()))
					++insert_counter;
			}
		}
		ofs << insert_counter << " ";
		ofs << bf.utilization() << " ";
		if (i == f_cnt) {
			ofs << endl;
			ofs << flush;
		}
	};
	ofs << "bit_arr_size f1 f1util f2 f2util f3 f3util f4 f4util f10 f10util f15 f15util" << endl;
	//f(bloom_filter<60000, 10>(), ifs, ofs, 0);
	//looper<max_steps>()(f, ifs, ofs);
	const auto start = std::chrono::high_resolution_clock::now();
	//test(f, ifs, ofs);
	looper<max_steps>()(f, ifs, ofs);
	const auto end = std::chrono::high_resolution_clock::now();

	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	cout << "Done in " << milliseconds / 1000 << " seconds." << endl;
}