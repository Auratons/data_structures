#ifndef TESTER_SPLAY_TREE_HPP
#define TESTER_SPLAY_TREE_HPP

#include <string>
#include <exception>
#include <vector>
#include <iterator>
#include <sstream>
#include <cassert>
#include <unordered_map>

#include "tester.h"
#include "splay.h"
#include "splay_std.h"
#include "splay_naive.h"


template<typename T>
class splay_tree_tester : public tree_tester<T> {
using node_t = tp::node<T>;
private:
	size_t overall_length;
	size_t find_count;
	size_t tree_size;
	tp::splay_tree<T>* tree;
	bool first_file_line;
	bool is_naive;
	std::unordered_map<size_t, size_t> lengths;

public:
	explicit splay_tree_tester(const std::string& splay_type) : tree_tester<T>(),
																overall_length(0), find_count(0), tree_size(0), tree(nullptr),
																first_file_line(true), is_naive(true) {
		if (splay_type == std::string("std")) {
			tree = new tp::splay_std<T>();
			is_naive = false;
		}
		else {
			tree = new tp::splay_naive<T>();
		}
	}

private:
	// If the input line contains planting a new tree, the method writes statistics for the
	// previous tree into the ofile file. If inserting, it only inserts into the current tree
	// that is building and if finding, it uses private find method that counts the length of
	// given search path.
	void process_line(const std::string& line) override {
		std::istringstream iss{ line };
		std::vector<std::string> cmd(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

		const size_t value = std::stoi(cmd[1]);
		if (cmd[0] == std::string("F")) {
			// possible speedup to remember the found lengths and reuse them, not finding every time
			if (lengths.find(value) == lengths.end()) {
				auto length = find(value);
				lengths[value] = length;
				overall_length += length;
			}
			else {
				overall_length += lengths[value];
			}
			find_count++;
			return;
		}
		if (cmd[0] == std::string("I")) {
			tree->insert(value);
			return;
		}
		if (cmd[0] == std::string("#")) {
			if (!first_file_line)
				write_out_tree_run();
			reset_tree_counting(value);
			first_file_line = false;
			return;
		}
		throw std::exception("Weird cmd.");
	}

	void postprocess_last_line() override {
		write_out_tree_run();
	}

	void write_out_tree_run() {
		tree_tester<T>::ofile << tree_size << ' ' << double(overall_length) / find_count << std::endl;
	}

	// Delete the old tree hanging under fst_node, reset counters.
	void reset_tree_counting(size_t tree_sz) noexcept {
		delete tree;
		if (is_naive)
			tree = new tp::splay_naive<T>();
		else
			tree = new tp::splay_std<T>();
		overall_length = 0;
		find_count = 0;
		tree_size = tree_sz;
	}

	// Return length of the search path to val in the tree.
	size_t find(T val) {
		node_t* ptr = tree->fst_node;
		size_t length = 0;
		while (ptr != nullptr) {
			const auto& current_data = ptr->data;
			if (current_data == val) {
				return length;
			}
			if (current_data < val) {
				ptr = ptr->right;
			}
			else {
				ptr = ptr->left;
			}
			length++;
		}
		return length;
	}

};

#endif // TESTER_SPLAY_TREE_HPP
