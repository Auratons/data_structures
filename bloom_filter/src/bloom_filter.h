#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <random>


#define STUDENT_NUMBER 95
#define NDEBUG
//#define USE_STL_GENERATOR

#define GET_PARAM_A(func_idx) (a_powers_[func_idx][1])
#define BYTE_SIZE(bit_count) (((bit_count) + 7) / 8)

#ifndef USE_STL_GENERATOR
#include <time.h>
#include <stdlib.h>
#endif

namespace bf {

	template<std::size_t BitArraySize, uint32_t HashFunctionCount>
	class bloom_filter {
		const uint32_t prime_number_ = 27644437u;
		uint8_t bit_array_[BYTE_SIZE(BitArraySize)]{};
		std::size_t hash_param_b_[HashFunctionCount]{};
		std::size_t hash_param_c_[HashFunctionCount]{};
		uint_least64_t** a_powers_ = nullptr;
		std::size_t precomputed_number_ = 0;
		std::size_t currently_allocated_ = 35;
	public:
		bloom_filter() {
			a_powers_ = new uint_least64_t*[HashFunctionCount];
#ifdef USE_STL_GENERATOR
			std::random_device rd;  //Will be used to obtain a seed for the random number engine
			std::mt19937 gen{ rd() }; //Standard mersenne_twister_engine seeded with rd()
			std::uniform_int_distribution<> dis{ 0, int32_t(prime_number_ - 1) };
#else
			srand(time(nullptr));
			auto r = random(uint32_t(rand()));
#endif
			precomputed_number_ = 35;
			for (uint32_t i = 0; i < HashFunctionCount; ++i) {
				a_powers_[i] = new uint_least64_t[currently_allocated_];
				a_powers_[i][0] = 1;
#ifdef USE_STL_GENERATOR
				a_powers_[i][1] = dis(gen);
				hash_param_b_[i] = dis(gen);
				hash_param_c_[i] = dis(gen);
#else
				a_powers_[i][1] = r.next_u32() % prime_number_;
				hash_param_b_[i] = r.next_u32() % prime_number_;
				hash_param_c_[i] = r.next_u32() % prime_number_;
#endif
				for (size_t j = 1; j < precomputed_number_ - 1; ++j) {
					a_powers_[i][j] = (a_powers_[i][j - 1] * GET_PARAM_A(i)) % prime_number_;
				}
			}
		}
		bloom_filter(const bloom_filter& other) = delete;
		bloom_filter(bloom_filter&& other) noexcept = default;
		bloom_filter& operator=(const bloom_filter& other) = delete;
		bloom_filter& operator=(bloom_filter&& other) noexcept = default;
		~bloom_filter() {
			for (uint32_t i = 0; i < HashFunctionCount; ++i) {
				delete[] a_powers_[i];
			}
			delete[] a_powers_;
		}

		bool insert(const void* value, const uint_least64_t value_size) {
			auto inserted = false;
			for (uint32_t hash_fun = 0; hash_fun < HashFunctionCount; ++hash_fun) {
				const auto index = compute_hash(hash_fun, value, value_size);
				if (!get_bit(index)) {
					inserted = true;
					set_bit(index);
				}
			}
			return inserted;
		}

		std::size_t utilization() const {
			std::size_t count = 0;
			for (std::size_t i = 0; i < BitArraySize; ++i) {
				if (get_bit(i))
					++count;
			}
			return count;
		}

	private:
#ifndef USE_STL_GENERATOR
		class random {
			/*
			 * This is the xoroshiro128+ random generator, designed in 2016 by David Blackman
			 * and Sebastiano Vigna, distributed under the CC-0 license. For more details,
			 * see http://vigna.di.unimi.it/xorshift/.
			 */
			uint64_t rng_state_[2] = { 0, 0 };
		public:
			explicit random(const uint32_t seed) {
				rng_state_[0] = seed * 0xdeadbeef;
				rng_state_[1] = seed ^ 0xc0de1234;
				for (auto i = 0; i < 100; i++)
					next_u64();
			}

			static uint64_t rot_l(const uint64_t x, int k) {
				return (x << k) | (x >> (64 - k));
			}

			uint64_t next_u64() {
				const auto s0 = rng_state_[0];
				auto s1 = rng_state_[1];
				const auto result = s0 + s1;
				s1 ^= s0;
				rng_state_[0] = rot_l(s0, 55) ^ s1 ^ (s1 << 14);
				rng_state_[1] = rot_l(s1, 36);
				return result;
			}

			uint32_t next_u32() {
				return uint32_t(next_u64() >> 11);
			}
		};
#endif

		uint_least64_t compute_hash(const uint32_t function_index, const void* value, const uint_least64_t value_byte_size) {
			assert(function_index < HashFunctionCount);
			uint_least64_t counter = 0;
			const auto poly_base = GET_PARAM_A(function_index);
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

			const auto val = static_cast<const uint8_t*>(value);
			for (std::size_t i = 0; i < value_byte_size; ++i) {
				counter = (counter % prime_number_ + (val[i] * a_powers_[function_index][i]) % prime_number_) % prime_number_;
			}
			return (additive + (multiplicative * (counter  % prime_number_)) % prime_number_) % BitArraySize;
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
			std::memcpy(temp, arr, (*array_size) * sizeof(ArrayType) / sizeof(unsigned char));
			*array_size = next_size;
			delete[] arr;
			return temp;
		}
	};

}
#endif // BLOOM_FILTER_HPP