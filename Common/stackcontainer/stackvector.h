

#pragma once
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <initializer_list>

template<typename T, size_t N>
class StackVector
{
public:
	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
	T data_[N];
	size_t size_;

public:
	// 构造函数
	StackVector() noexcept : size_(0) {}

	explicit StackVector(size_type count, const T& value = T())
	{
		if (count > N) throw std::length_error("Count exceeds capacity");
		size_ = count;
		std::fill_n(data_, count, value);
	}

	StackVector(std::initializer_list<T> init)
	{
		if (init.size() > N) throw std::length_error("Initializer list too large");
		size_ = init.size();
		std::copy(init.begin(), init.end(), data_);
	}

	StackVector(const StackVector& other)
	{
		size_ = other.size_;
		std::copy(other.data_, other.data_ + size_, data_);
	}

	// 赋值运算符
	StackVector& operator=(const StackVector& other)
	{
		if (this != &other)
		{
			size_ = other.size_;
			std::copy(other.data_, other.data_ + size_, data_);
		}
		return *this;
	}

	// 元素访问
	reference at(size_type pos)
	{
		if (pos >= size_) throw std::out_of_range("Index out of range");
		return data_[pos];
	}

	const_reference at(size_type pos) const
	{
		if (pos >= size_) throw std::out_of_range("Index out of range");
		return data_[pos];
	}

	reference operator[](size_type pos) { return data_[pos]; }
	const_reference operator[](size_type pos) const { return data_[pos]; }

	reference front() { return data_[0]; }
	const_reference front() const { return data_[0]; }

	reference back() { return data_[size_ - 1]; }
	const_reference back() const { return data_[size_ - 1]; }

	pointer data() noexcept { return data_; }
	const_pointer data() const noexcept { return data_; }

	// 迭代器
	iterator begin() noexcept { return data_; }
	const_iterator begin() const noexcept { return data_; }
	const_iterator cbegin() const noexcept { return data_; }

	iterator end() noexcept { return data_ + size_; }
	const_iterator end() const noexcept { return data_ + size_; }
	const_iterator cend() const noexcept { return data_ + size_; }

	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

	// 容量
	bool empty() const noexcept { return size_ == 0; }
	size_type size() const noexcept { return size_; }
	size_type max_size() const noexcept { return N; }
	size_type capacity() const noexcept { return N; }

	// 修改器
	void clear() noexcept { size_ = 0; }

	iterator insert(const_iterator pos, const T& value)
	{
		if (size_ >= N) throw std::length_error("Container is full");

		size_t index = pos - begin();
		if (index > size_) throw std::out_of_range("Invalid position");

		for (size_t i = size_; i > index; --i)
		{
			data_[i] = std::move(data_[i - 1]);
		}
		data_[index] = value;
		++size_;
		return iterator(data_ + index);
	}

	iterator erase(const_iterator pos)
	{
		size_t index = pos - begin();
		if (index >= size_) throw std::out_of_range("Invalid position");

		for (size_t i = index; i < size_ - 1; ++i)
		{
			data_[i] = std::move(data_[i + 1]);
		}
		--size_;
		return iterator(data_ + index);
	}

	void push_back(const T& value)
	{
		if (size_ >= N) throw std::length_error("Container is full");
		data_[size_++] = value;
	}

	void push_back(T&& value)
	{
		if (size_ >= N) throw std::length_error("Container is full");
		data_[size_++] = std::move(value);
	}

	void pop_back()
	{
		if (size_ == 0) throw std::runtime_error("Container is empty");
		--size_;
	}

	void resize(size_type count, const value_type& value = T())
	{
		if (count > N) throw std::length_error("New size exceeds capacity");

		if (count > size_)
		{
			std::fill(data_ + size_, data_ + count, value);
		}
		size_ = count;
	}

	void swap(StackVector& other) noexcept
	{
		std::swap(size_, other.size_);
		for (size_t i = 0; i < std::max(size_, other.size_); ++i)
		{
			std::swap(data_[i], other.data_[i]);
		}
	}

	// 比较运算符
	bool operator==(const StackVector& other) const
	{
		if (size_ != other.size_) return false;
		return std::equal(begin(), end(), other.begin());
	}

	bool operator!=(const StackVector& other) const
	{
		return !(*this == other);
	}

	bool operator<(const StackVector& other) const
	{
		return std::lexicographical_compare(begin(), end(),
			other.begin(), other.end());
	}

	bool operator<=(const StackVector& other) const
	{
		return !(other < *this);
	}

	bool operator>(const StackVector& other) const
	{
		return other < *this;
	}

	bool operator>=(const StackVector& other) const
	{
		return !(*this < other);
	}
};

// 非成员函数
template<typename T, size_t N>
void swap(StackVector<T, N>& lhs, StackVector<T, N>& rhs) noexcept
{
	lhs.swap(rhs);
}