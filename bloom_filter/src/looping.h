#ifndef LOOPING_HPP
#define LOOPING_HPP

#include <fstream>
#include "bloom_filter.h"

#define STUDENT_NUMBER 95
#define PREALLOCATED 15
namespace bf {

	constexpr const int max_steps = 600000;
	constexpr const int min_steps = 10000;
	constexpr const int step_size = 10000;
	constexpr const int f_cnt = 6;
	constexpr const int functions[f_cnt] = { 1, 2, 3, 4, 10, 15 };

	template<int Size, int FCnt>
	struct f_loop {
		template <typename F>
		constexpr void operator()(F& f, std::ifstream& ifs, std::ofstream& ofs) {
			f(bloom_filter<Size, functions[FCnt - 1]>(), ifs, ofs, FCnt);
			std::cout << "(" << Size << ", " << functions[FCnt - 1] << ")" << std::endl;
			f_loop<Size, FCnt + 1>()(f, ifs, ofs);
		}
	};

	template<int Size>
	struct f_loop<Size, f_cnt> {
		template <typename F>
		constexpr void operator()(F& f, std::ifstream& ifs, std::ofstream& ofs) {
			f(bloom_filter<Size, functions[f_cnt - 1]>(), ifs, ofs, f_cnt);
			std::cout << "(" << Size << ", " << functions[f_cnt - 1] << ")" << std::endl;
		}
	};

	template <int Size>
	struct looper {
		template <typename F>
		constexpr void operator()(F& f, std::ifstream& ifs, std::ofstream& ofs) {
			ofs << Size << " ";
			f_loop<Size, 1>()(f, ifs, ofs);
			looper<Size - step_size>()(f, ifs, ofs);
		}
	};

	template <>
	struct looper<min_steps> {
		template <typename F>
		constexpr void operator()(F& f, std::ifstream& ifs, std::ofstream& ofs) {
			ofs << min_steps << " ";
			f_loop<min_steps, 1>()(f, ifs, ofs);
		}
	};

}
#endif // LOOPING_HPP