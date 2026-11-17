#pragma once
#include <stddef.h>

template <class T>
class UniquePtr {
public:
	UniquePtr(T* ptr = nullptr) : ptr(ptr) {}

	~UniquePtr() {
		delete ptr;
	}

	UniquePtr(const UniquePtr<T>& other) = delete;
	UniquePtr<T>& operator=(const UniquePtr<T>& other) = delete;

	UniquePtr(UniquePtr<T>&& other) noexcept : ptr(other.ptr) {
		other.ptr = nullptr;
	}

	UniquePtr<T>& operator=(UniquePtr<T>&& other) noexcept {
		if (this != &other) {
			delete ptr;

			ptr = other.ptr;
			other.ptr = nullptr;
		}

		return *this;
	}

	T& operator*() const {
		return *ptr;
	}

	T* operator->() const {
		return ptr;
	}

	explicit operator bool() const noexcept {
		return ptr != nullptr;
	}

	T* get() const noexcept {
		return ptr;
	}

	T* release() noexcept {
		T* result = ptr;
		ptr = nullptr;
		return result;
	}

	void reset(T* ptr = nullptr) noexcept {
		if (this->ptr != ptr) {
			delete this->ptr;
			this->ptr = ptr;
		}
	}

	void swap(UniquePtr<T>& other) noexcept {
		T* temp = ptr;
		ptr = other.ptr;
		other.ptr = temp;
	}

private:
	T* ptr;
};

template <class T>
class UniquePtr<T[]> {
public:
	UniquePtr(T* ptr = nullptr) : ptr(ptr) {}

	~UniquePtr() {
		delete[] ptr;
	}

	UniquePtr(const UniquePtr<T[]>& other) = delete;
	UniquePtr<T[]>& operator=(const UniquePtr<T[]>& other) = delete;

	UniquePtr(UniquePtr<T[]>&& other) noexcept : ptr(other.ptr) {
		other.ptr = nullptr;
	}

	UniquePtr<T[]>& operator=(UniquePtr<T[]>&& other) noexcept {
		if (this != &other) {
			delete[] ptr;

			ptr = other.ptr;
			other.ptr = nullptr;
		}

		return *this;
	}

	T& operator[](std::size_t idx) const {
		return ptr[idx];
	}

	explicit operator bool() const noexcept {
		return ptr != nullptr;
	}

	T* get() const noexcept {
		return ptr;
	}

	T* release() noexcept {
		T* result = ptr;
		ptr = nullptr;
		return result;
	}

	void reset(T* ptr = nullptr) noexcept {
		if (this->ptr != ptr) {
			delete[] this->ptr;
			this->ptr = ptr;
		}
	}

	void swap(UniquePtr<T[]>& other) noexcept {
		T* temp = ptr;
		ptr = other.ptr;
		other.ptr = temp;
	}

private:
	T* ptr;
};