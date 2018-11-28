
#include <iostream>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include "matrix.h"
#include "transpositions.h"

using namespace mt;

double switch_count(const size_t n) {
	return double(n * n - n) / 2;
}

double std_err(std::vector<double>& v) {
	const auto sum = std::accumulate(v.begin(), v.end(), 0.0);
	auto mean = sum / v.size();
	std::vector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(), [mean](const double x) { return x - mean; });
	const auto sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	const auto std = std::sqrt(sq_sum / v.size());
	return std;
}

int main(int argc, char* argv[]) {
	const auto simulate = true;
	const auto debug = false;

	if (simulate) {
		for (size_t i = 54; i <= 120; i++) {
			const size_t dim = ceil(exp2(double(i) / 9));
			matrix<int32_t> m(dim, dim);
			transposition_naive<int32_t> tn(m);
			transposition_cache_oblivious<int32_t> to(m);
			const auto simulate_swap = [](const size_t h, const size_t w) {
				std::cout << "X " << h << " " << w << " " << w << " " << h << std::endl;
			};
			const auto do_swap = [data = m.to_array(), dim](const size_t h, const size_t w) {
				const auto temp = data[w * dim + h];
				data[w * dim + h] = data[h * dim + w];
				data[h * dim + w] = temp;
			};

			std::cout << "N " << dim << std::endl;
			to.transpose(simulate_swap);
			std::cout << "E" << std::endl;
		}
	}
	else {
			std::ofstream ofs{ "results.dat" };
			ofs << "k n naive naive_err oblivious oblivious_err" << std::endl << std::flush;

			constexpr size_t repetition_count = 10;

			auto start = std::chrono::steady_clock::now();
			auto end = std::chrono::steady_clock::now();
			std::vector<double> naive_values(repetition_count, 0.0);
			std::vector<double> oblivious_values(repetition_count, 0.0);

			for (size_t i = 54; /* Till RAM runs out. */ ; i++) {
				const size_t dim = ceil(exp2(double(i) / 9));
				matrix<int32_t> m(dim, dim);
				transposition_naive<int32_t> tn(m);
				transposition_cache_oblivious<int32_t> to(m);
				const auto simulate_swap = [](const size_t h, const size_t w) {
					std::cout << "X " << h << " " << w << " " << w << " " << h << std::endl;
				};
				const auto do_swap = [data = m.to_array(), dim](const size_t h, const size_t w) {
					const auto temp = data[w * dim + h];
					data[w * dim + h] = data[h * dim + w];
					data[h * dim + w] = temp;
				};

				if (debug) {
					for (size_t j = 0; j < dim * dim; j++) {
						m.to_array()[j] = j;
					}
				}

				for (size_t j = 0; j < repetition_count; j++) {
					start = std::chrono::steady_clock::now();
					//naive_transposition(m, std::cout);
					tn.transpose(do_swap);
					end = std::chrono::steady_clock::now();
					double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / switch_count(dim);
					naive_values[j] = time;

					if (debug) {
						for (size_t width = 0; width < dim; width++) {
							for (size_t height = 0; height < dim; height++) {
								if (size_t(m.at(height, width)) != width * dim + height) {
									std::cout << "Err naive: " << i << " at (" << height << width << ") is " << m.at(height, width) << "should be " << width * dim + height << std::endl;
								}
							}
						}
					}

					start = std::chrono::steady_clock::now();
					//cache_oblivious_transposition(m, std::cout);
					to.transpose(do_swap);
					end = std::chrono::steady_clock::now();
					time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / switch_count(dim);
					oblivious_values[j] = time;

					if (debug) {
						for (size_t height = 0; height < dim; height++) {
							for (size_t width = 0; width < dim; width++) {
								if (size_t(m.at(height, width)) != height * dim + width) {
									std::cout << "Err oblivious: " << i << " at (" << height << width << ") is " << m.at(height, width) << "should be " << width * dim + height << std::endl;
								}
							}
						}
					}
				}

				const auto naive_mean = std::accumulate(naive_values.begin(), naive_values.end(), 0.0) / repetition_count;
				const auto oblivious_mean = std::accumulate(oblivious_values.begin(), oblivious_values.end(), 0.0) / repetition_count;
				const auto naive_err = std_err(naive_values);
				const auto oblivious_err = std_err(oblivious_values);

				std::cout << "k: " << i << " N: " << dim << " Naive: " << naive_mean << " NaiveErr: " << naive_err;
				std::cout << " Oblivious: " << oblivious_mean << " ObliviousErr: " << oblivious_err << std::endl;
				ofs << i << " " << dim << " " << naive_mean << " " << naive_err << " " << oblivious_mean << " " << oblivious_err << std::endl << std::flush;
			}
	}
}