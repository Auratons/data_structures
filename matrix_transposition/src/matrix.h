#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <exception>
#include <cstdint>
#include <cstring>
#include <ostream>

#define CHECK(height,width) \
if (height < 0 || width < 0 || size_t(height) >= height_ || size_t(width) >= width_) \
	throw std::exception("At least one of the input coordinates is out of bounds.");

namespace mt {
	/*
	 * Simple matrix class implementation. The data are stored in row-major order.
	 * Basic operations always share underlying data. For deep copy, the caller must
	 * use copy() (for exact behavior refer to its implementation).
	 */
	template <typename T>
	class matrix {
		T* data_;
		size_t height_;
		size_t width_;

	public:
		matrix() noexcept : data_(nullptr), height_(0), width_(0) {}

		// Instantiate matrix with predefined dimensions filled with random data.
		matrix(const int_least32_t height, const int_least32_t width) {
			if (height > 0 && width > 0) {
				data_ = new T[height * width];
				height_ = height;
				width_ = width;
			}
			else {
				throw std::exception("At least on of matrix input parameters is less than one.");
			}
		}

		// Shares matrix data.
		matrix(const matrix& other) :
			data_{ other.data_ }, height_(other.height_), width_(other.width_) {}

		// Shares matrix data.
		matrix& operator=(const matrix& other) noexcept {
			if (this == &other)
				return *this;
			data_ = other.data_;
			height_ = other.height_;
			width_ = other.width_;
			return *this;
		}

		matrix(matrix&& other) noexcept :
			data_{ std::move(other.data_) }, height_(other.height_), width_(other.width_) {}

		matrix& operator=(matrix&& other) noexcept {
			if (this == &other)
				return *this;
			data_ = std::move(other.data_);
			height_ = other.height_;
			width_ = other.width_;
			return *this;
		}

		// Return zeroed-out matrix with defined dimensions.
		static matrix zeros(const int_least32_t height, const int_least32_t width) {
			matrix m(height, width);
			std::memset(m.to_array(), 0, sizeof(T) * width * height);
			return std::move(m);
		}

		// Return indexed matrix element. No bound checking -- outside of bounds
		// causes undefined behavior.
		T& get(const int_least32_t height, const int_least32_t width) noexcept {
			return data_[height * width_ + width];
		}

		const T& get(const int_least32_t height, const int_least32_t width) const noexcept {
			return data_[height * width_ + width];
		}

		// Return indexed element. Throw std::exception in case of wrong bounds.
		T& at(const int_least32_t height, const int_least32_t width) {
			CHECK(height, width);
			return get(height, width);
		}

		const T& at(const int_least32_t height, const int_least32_t width) const {
			CHECK(height, width);
			return get(height, width);
		}

		// Return height of the matrix.
		size_t height() const noexcept { return height_; }

		// Return width of the matrix.
		size_t width() const noexcept { return width_; }

		// Return copy of the matrix. Exact behavior depends on
		// T& operator(const T&) making it deep or shallow copy.
		matrix copy() const {
			matrix temp = matrix(height_, width_);
			auto err = memcpy_s(temp.to_array(), height_ * width_, data_, height_ * width_);
			if (err) {
				std::exception("Problem");
			}
			return std::move(temp);
		}

		// Return underlying const pointer to matrix data.
		T* const to_array() const {
			return data_;
		}

		friend std::ostream& operator<<(std::ostream& os, const matrix& obj) {
			for (size_t row = 0; row < obj.height(); row++) {
				for (size_t column = 0; column < obj.width(); column++) {
					os << obj.get(row, column) << " ";
				}
				os << std::endl;
			}
			return os;
		}

		~matrix() {
			delete[] data_;
		}
	};
}

#endif /* MATRIX_HPP */