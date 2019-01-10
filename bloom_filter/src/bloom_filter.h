#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <functional>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cmath>

#define STUDENT_NUMBER 95

template<std::size_t BitArraySize, std::size_t HashFunctionCount>
class bloom_filter {
	const std::size_t bit_array_size_ = (BitArraySize + 7) / 8;
	const std::size_t prime_number_ = 27644437;
	uint8_t* bit_array_ = nullptr;
	uint32_t* hash_functions_parameters_ = nullptr;
public:
	explicit bloom_filter(const size_t seed) :
	bit_array_(new uint8_t[bit_array_size_]),
	hash_functions_parameters_(new std::size_t[HashFunctionCount]) {
		random rnd(seed);
		for (std::size_t i = 0; i < HashFunctionCount; ++i) {
			uint32_t parameter = 0;
			for (size_t j = 0; j < 1000; j++) {
				parameter = rnd.next_u32();
				if (parameter < prime_number_) {
					hash_functions_parameters_[i] = parameter;
					break;
				}
			}
			hash_functions_parameters_[i] = (!parameter) ? STUDENT_NUMBER : parameter;
		}
	}
	bloom_filter() : bloom_filter(STUDENT_NUMBER) {}
	bloom_filter(const bloom_filter& other) = delete;
	bloom_filter(bloom_filter&& other) noexcept = default;
	bloom_filter& operator=(const bloom_filter& other) = delete;
	bloom_filter& operator=(bloom_filter&& other) noexcept = default;
	~bloom_filter() {
		delete[] bit_array_;
		delete[] hash_functions_parameters_;
	}

	bool insert(const void* value, const uint_least64_t value_size) {
		auto inserted = false;
		for (size_t hash_fun = 0; hash_fun < HashFunctionCount; ++hash_fun) {
			const auto index = compute_hash(hash_fun, &value, value_size) % bit_array_size_;
			if (!get_bit(index)) {
				inserted = true;
				set_bit(index);
			}
		}
		return inserted;
	}

private:
	class random {
		/*
		 * This is the xoroshiro128+ random generator, designed in 2016 by David Blackman
		 * and Sebastiano Vigna, distributed under the CC-0 license. For more details,
		 * see http://vigna.di.unimi.it/xorshift/.
		 */
		uint64_t rng_state_[2] = { 0, 0 };
	public:
		explicit random(const unsigned int seed) {
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

	uint_least64_t compute_hash(const std::size_t hash_function_index, const void* value, const uint_least64_t value_byte_size) const {
		assert(hash_function_index < HashFunctionCount);
		uint_least64_t counter = 0;
		const uint_least64_t poly_base = hash_functions_parameters_[hash_function_index];
		for (std::size_t i = 0; i < value_byte_size; i++) {
			const auto val = static_cast<const uint8_t*>(value);
			counter += (uint_least64_t(val[i]) * uint_least64_t(std::pow(poly_base, i))) % prime_number_;
		}
		return counter % prime_number_;
	}

	bool get_bit(const uint_least64_t index) const {
		assert(index < bit_array_size_);
		return bit_array_[index / 8] & 1 << (index % 8);
	}

	void set_bit(const uint_least64_t index) {
		assert(index < bit_array_size_);
		bit_array_[index / 8] |= 1 << (index % 8);
	}
};

#endif // BLOOM_FILTER_HPP