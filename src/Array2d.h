#ifndef ARRAY_2D_H
#define ARRAY_2D_H

#include <memory>

template <class T>
class Array2D
{
	unsigned int width_;
	unsigned int height_;
	unsigned int size_;
	std::unique_ptr<T[]> data_;
public:
	class iterator
	{
		friend Array2D;
		T* ptr;
		iterator(T* p) : ptr(p) {}
	public:
		iterator() : ptr(nullptr) {}
		iterator& operator --()
		{
			ptr--;
			return *this;
		}
		iterator& operator ++()
		{
			ptr++;
			return *this;
		}
		iterator& operator -=(int x)
		{
			ptr -= x;
			return *this;
		}
		iterator& operator +=(int x)
		{
			ptr += x;
			return *this;
		}
		bool operator !=(iterator& i)
		{
			return ptr != i.ptr;
		}
		T* operator ->()
		{
			return ptr;
		}
		T* operator *()
		{
			return ptr;
		}
	};
	Array2D(){ data_ = nullptr; }
	Array2D(unsigned wset, unsigned hset)
		: width_(wset), height_(hset), size_(width * height_)
	{
		data_ = std::unique_ptr<T>(new T[size_]);
	}
	~Array2D()
	{
	}
	Array2D(Array2D const& a)
	{
		width_ = a.width_;
		height_ = a.height_;
		size_ = a.size_;
		data_ = std::unique_ptr<T>(new T[size_]);
		memcpy(data_.get(), a.data_.get(), size_ * sizeof(T));
	}
	Array2D& operator= (Array2D const& a)
	{
		width_ = a.width_;
		height_ = a.height_;
		size_ = a.width_ * a.height_;
		data_ = std::unique_ptr<T>(new T[size_]);
		memcpy(data_.get(), a.data_.get(), size_ * sizeof(T));
		return *this;
	}
	bool operator! ()
	{
		return !data_;
	}
	// Access
	T& operator() (unsigned x, unsigned y) const
	{
		return data_.get()[y * width_ + x];
	}
	T& operator[] (int i) const
	{
		return data_.get()[i];
	}
	unsigned int width() const
	{
		return width_;
	}
	unsigned int height() const
	{
		return height_;
	}
	unsigned int size() const
	{
		return size_;
	}
	bool empty() const
	{
		return (size_ < 1U);
	}
	iterator begin()
	{
		return iterator(data_.get());
	}
	iterator end()
	{
		return iterator(&data_.get()[size_]);
	}
	// Manipulate
	void set(unsigned wset, unsigned hset)
	{
		width_ = wset;
		height_ = hset;
		size_ = wset * hset;
		if (data_) {
			data_.reset();
		}
		data_ = std::unique_ptr<T[]>(new T[size_]);
	}
	void reset()
	{
		width_ = 0;
		height_ = 0;
		size_ = 0;
		data_.reset();
	}
}; // Array2D

#endif
