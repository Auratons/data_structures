#ifndef TRANSPOSITIONS_HPP
#define TRANSPOSITIONS_HPP

#include <fstream>
#include "matrix.h"

#define CHECK_IF_SQUARE(matrix) \
if (matrix.width() != matrix.height()) { \
	throw std::exception("Input matrix is not square."); \
}


namespace mt {

	template<typename T>
	class transposition_naive {
	public:
		matrix<T>&m;
		explicit transposition_naive(matrix<T>& mm) : m(mm) {
			CHECK_IF_SQUARE(m);
		}

		// Lambdas serve as a way how to split work with simulation and real switching at once.
		template<typename Lambda>
		void transpose(Lambda f) {
			for (size_t h = 1; h < m.height(); h++) {
				for (size_t w = 0; w < h; w++) {
					f(h, w);
				}
			}
		}
	};

	template<typename T>
	class transposition_cache_oblivious {
	public:
		matrix<T>& m;
		explicit transposition_cache_oblivious(matrix<T>& mm) : m(mm) {
			CHECK_IF_SQUARE(m);
		}

		// Lambdas serve as a way how to split work with simulation and real switching at once.
		template<typename Lambda>
		void transpose(Lambda f) {
			transpose_on_diagonal(0, 0, m.height(), m.width(), f);
		}
	private:
		// If the matrix is small enough we use naive algorithm, otherwise
		// we use recursion.
		template<typename Lambda>
		matrix<T>& transpose_on_diagonal(size_t rb, size_t cb, size_t re, size_t ce, Lambda f) {
			const auto r = re - rb, c = ce - cb;
			if (r <= 16 && c <= 16) {
				for (auto h = rb; h < re; h++) {
					for (auto w = cb; w < h; w++) {
						f(h, w);
					}
				}
			}
			else {
				transpose_on_diagonal(rb, cb, rb + r / 2, cb + c / 2, f);
				transpose_on_diagonal(rb + r / 2, cb + c / 2, re, ce, f);
				transpose_and_swap(rb + r / 2, cb, re, cb + c / 2, f);
			}
			return m;
		}

		// We transpose and swap when the submatrices are small enough and we
		// do it simultaneously by going though rows of the first matrix and
		// columns of the second matrix, which coordinates we compute from the
		// coordinates of the first one.
		template<typename Lambda>
		matrix<T>& transpose_and_swap(size_t arb, size_t acb, size_t are, size_t ace, Lambda f) {
			const auto ar = are - arb, ac = ace - acb;
			if (ar <= 16 || ac <= 16) {
				for (size_t h = 0; h < ar; h++) {
					for (size_t w = 0; w < ac; w++) {
						f(arb + h, acb + w);
					}
				}
			}
			else {
				transpose_and_swap(arb, acb, arb + ar / 2, acb + ac / 2, f); // left top and  left top
				transpose_and_swap(arb, acb + ac / 2, arb + ar / 2, ace, f); // right top and left bottom
				transpose_and_swap(arb + ar / 2, acb, are, acb + ac / 2, f); // left bottom and right top
				transpose_and_swap(arb + ar / 2, acb + ac / 2, are, ace, f); // right bottom and right bottom
			}
			return m;
		}
	};
}

#endif /* TRANSPOSITIONS_HPP */