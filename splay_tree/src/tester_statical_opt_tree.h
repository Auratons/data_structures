#ifndef TESTER_STATICAL_OPT_TREE_HPP
#define TESTER_STATICAL_OPT_TREE_HPP

#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <utility>

#include "tester.h"

template<typename T>
class statical_tree_tester : public tree_tester<T> {
using pair_t = std::pair<size_t, size_t>;
private:
	using node_val = size_t;
	using node_val_cnt = size_t;
	using bounds = size_t;
	using root_pos = size_t;

	struct pair_hash {
		template<typename TT>
		size_t operator() (const std::pair<TT, TT>& p) const {
			auto h1 = std::hash<TT>{}(p.first);
			auto h2 = std::hash<TT>{}(p.second);
			return h1 ^ h2;
		}
	};

	std::unordered_map<node_val, node_val_cnt> val_counts;
	std::string line;
	size_t overall_length;
	size_t find_count;
	size_t tree_size;
	std::unordered_map<std::pair<bounds, bounds>, root_pos, pair_hash> roots;
	bool first_file_line;

public:
	statical_tree_tester() : tree_tester<T>(), overall_length(0), find_count(0), tree_size(0), first_file_line(true) {
	}

private:
	void process_line(const std::string& line) override {
		std::istringstream iss{ line };
		std::vector<std::string> cmd(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
		
		const size_t value = std::stoi(cmd[1]);
		if (cmd[0] == std::string("F")) {
			val_counts[value]++;
			return;
		}
		if (cmd[0] == std::string("I")) {
			val_counts[value] = 0;
			return;
		}
		if (cmd[0] == std::string("#")) {
			if (!first_file_line) {
				write_out_tree_run();
				// Reset
				overall_length = 0;
				find_count = 0;
			}
			tree_size = std::stoi(cmd[1]);
			first_file_line = false;
		}
	}

	void postprocess_last_line() override {
		write_out_tree_run();
	}

	void write_out_tree_run() {
		std::vector<node_val> numbers;
		roots = find_statically_optimal_roots(numbers);
		for (auto&& number : numbers) {
			overall_length += find(numbers, number) * val_counts[number];
			find_count += val_counts[number];
		}
		tree_tester<T>::ofile << tree_size << ' ' << double(overall_length) / find_count << std::endl;
	}

	// Return length of the search path to val in the tree.
	size_t find(std::vector<node_val>& numbers, T value) {
		size_t begin = 0;
		size_t end = numbers.size() - 1;
		size_t ptr = numbers[roots[pair_t(begin, end)]];
		size_t path_length = 0;
		while (ptr != value) {
			if (begin == end) {
				break; // The value is not in the tree.
			}
			if (value > ptr) {
				begin = roots.at(pair_t(begin, end)) + 1;
			}
			else {
				end = roots.at(pair_t(begin, end)) - 1;
			}
			ptr = numbers.at(roots[pair_t(begin, end)]);
			path_length++;
		}
		return path_length;
	}

	auto find_statically_optimal_roots(std::vector<node_val>& numbers) {
		std::unordered_map<std::pair<bounds, bounds>, root_pos, pair_hash> expected_values;
		numbers.reserve(val_counts.size());
		numbers.insert(numbers.begin(), val_counts.size(), 0);
		std::transform(val_counts.begin(), val_counts.end(), numbers.begin(), [](auto pair) { return pair.first; });
		std::sort(numbers.begin(), numbers.end()); // Ascending order.

		// Dynamic programming algorithm initialization.
		for (size_t i = 0; i < numbers.size(); i++) {
			expected_values[std::pair<size_t, size_t>(i, i)] = val_counts[numbers[i]];
			roots[std::pair<size_t, size_t>(i, i)] = i;
		}	

		for (size_t window_size = 2; window_size <= numbers.size(); window_size++)
		{
			for (size_t window_shift = 0; window_shift <= numbers.size() - window_size; window_shift++)
			{
				const auto begin = window_shift;
				const auto end = begin + window_size - 1;
				auto min_expected_value = std::numeric_limits<size_t>::max();
				size_t cnt_sum = 0;
				// Weight of the whole window-defined tree.
				for (size_t i = 0; i < window_size; i++)
					cnt_sum += val_counts.at(numbers[window_shift + i]);
				// Testing all possible root candidates.
				for (size_t root_candidate = roots[pair_t(begin, end - 1)]; root_candidate <= roots[pair_t(begin + 1, end)]; root_candidate++)
				{
					size_t el = 0;
					size_t er = 0;
					if (root_candidate != 0)
						el = expected_values[pair_t(begin, root_candidate - 1)];
					if (root_candidate + 1 < numbers.size())
						er = expected_values[pair_t(root_candidate + 1, end)];
					auto expected_val = el + er + cnt_sum;
					if (expected_val < min_expected_value) {
						roots[pair_t(begin, end)] = root_candidate;
						expected_values[pair_t(begin, end)] = expected_val;
						min_expected_value = expected_val;
					}
				}
			}
		}
		return roots;
	}
};

#endif // TESTER_STATICAL_OPT_TREE_HPP