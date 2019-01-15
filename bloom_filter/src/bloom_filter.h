#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <random>

#define STUDENT_NUMBER 95
#define PREALLOCATED 15
//#define NDEBUG

namespace bf {

	template<std::size_t BitArraySize, std::size_t HashFunctionCount>
	class bloom_filter {
		const std::size_t bit_array_size_ = (BitArraySize + 7) / 8;
		const std::size_t prime_number_ = 27644437;
		uint8_t* bit_array_ = nullptr;
		std::size_t* hash_param_a_ = nullptr;
		std::size_t* hash_param_b_ = nullptr;
		std::size_t* hash_param_c_ = nullptr;
		uint_least64_t** a_powers_ = nullptr;
		std::size_t precomputed_number_ = 0;
		std::size_t currently_allocated_ = 100;
	public:
		explicit bloom_filter() :
			bit_array_(new uint8_t[bit_array_size_]),
			hash_param_a_(new std::size_t[HashFunctionCount]),
			hash_param_b_(new std::size_t[HashFunctionCount]),
			hash_param_c_(new std::size_t[HashFunctionCount]),
			a_powers_(new uint_least64_t*[HashFunctionCount]) {
			std::random_device rd;  //Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
			std::uniform_int_distribution<> dis(0, prime_number_ - 1);
			std::memset(bit_array_, 0, bit_array_size_);
			for (std::size_t i = 0; i < HashFunctionCount; ++i) {
				hash_param_a_[i] = dis(gen) % prime_number_;
				hash_param_b_[i] = dis(gen) % prime_number_;
				hash_param_c_[i] = dis(gen) % prime_number_;
				a_powers_[i] = new uint_least64_t[currently_allocated_];
				a_powers_[i][0] = 1;
			}
			precomputed_number_ = 1;
		}
		bloom_filter(const bloom_filter& other) = delete;
		bloom_filter(bloom_filter&& other) noexcept = default;
		bloom_filter& operator=(const bloom_filter& other) = delete;
		bloom_filter& operator=(bloom_filter&& other) noexcept = default;
		~bloom_filter() {
			delete[] bit_array_;
			delete[] hash_param_a_;
			delete[] hash_param_b_;
			delete[] hash_param_c_;
			for (std::size_t i = 0; i < HashFunctionCount; ++i) {
				delete[] a_powers_[i];
			}
			delete[] a_powers_;
		}

		bool insert(const void* value, const uint_least64_t value_size) {
			auto inserted = false;
			for (std::size_t hash_fun = 0; hash_fun < HashFunctionCount; ++hash_fun) {
				const auto index = compute_hash(hash_fun, value, value_size);
				if (!get_bit(index)) {
					inserted = true;
					set_bit(index);
				}
			}
			return inserted;
		}

	private:

		uint_least64_t compute_hash(const std::size_t function_index, const void* value, const uint_least64_t value_byte_size) {
			assert(function_index < HashFunctionCount);
			uint_least64_t counter = 0;
			const auto poly_base = hash_param_a_[function_index];
			const auto multiplicative = hash_param_b_[function_index];
			const auto additive = hash_param_c_[function_index];

			if (value_byte_size > precomputed_number_) {
				while (value_byte_size >= currently_allocated_)
					a_powers_[function_index] = double_array(a_powers_[function_index], &currently_allocated_);
				for (std::size_t i = 0; i < value_byte_size - precomputed_number_; ++i) {
					assert(precomputed_number_ < currently_allocated_);
					a_powers_[function_index][precomputed_number_] = (a_powers_[function_index][precomputed_number_ - 1] * poly_base) % prime_number_;
					++precomputed_number_;
				}
			}

			const auto val = reinterpret_cast<const uint8_t*>(value);
			for (std::size_t i = 0; i < value_byte_size; ++i) {
				counter += (multiplicative * val[i] * a_powers_[function_index][i]) % prime_number_;
			}
			return (additive + counter % prime_number_) % BitArraySize;
		}

		bool get_bit(const uint_least64_t index) const {
			assert(index < BitArraySize);
			return bit_array_[index / 8] & (1 << (index % 8));
		}

		void set_bit(const uint_least64_t index) {
			assert(index < BitArraySize);
			bit_array_[index / 8] |= (1 << (index % 8));
		}

		// Copy array to memory block twice the array_size which is in-out parameter
		// set to the doubled size. Delete array, return pointer to the reallocated memory block.
		template<typename ArrayType>
		static ArrayType* double_array(ArrayType* arr, std::size_t* array_size) {
			const auto next_size = (*array_size) * 2;
			auto temp = new ArrayType[next_size];
			std::memcpy(temp, arr, (*array_size) * sizeof(ArrayType*) / sizeof(unsigned char));
			*array_size = next_size;
			delete[] arr;
			return temp;
		}
	};

}
#endif // BLOOM_FILTER_HPP