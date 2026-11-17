#pragma once
#include <stddef.h> 

template <class T>
class SharedPtr {
public:
    explicit SharedPtr(T* ptr = nullptr) : ptr(ptr), count(nullptr) {
		try {
			count = new std::size_t(1);
		} catch(const std::bad_alloc&) {
			delete ptr;
			throw;
		}

    }

    ~SharedPtr() {
		release_ownership();
    }

    SharedPtr(const SharedPtr<T>& other) : ptr(other.ptr), count(other.count) {
		if (count != nullptr) {
			++(*count);
		}
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other) {
		if (this != &other) {
			release_ownership();
			ptr = other.ptr;
			count = other.count;

			if (count != nullptr) {
				++(*count);
			}
		}

		return *this;
    }

    SharedPtr(SharedPtr<T>&& other) noexcept : ptr(other.ptr), count(other.count) {
		other.ptr = nullptr;
		other.count = nullptr;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& other) noexcept {
		if (this != &other) {
			release_ownership();

			ptr = other.ptr;
			count = other.count;

			other.ptr = nullptr;
			other.count = nullptr;
		}

		return *this;
    }

    explicit operator bool() noexcept {
		return ptr != nullptr;
    }

    T& operator*() const noexcept {
		return *ptr;
    }

    T* operator->() const noexcept {
		return ptr;
    }

    T* get() const noexcept {
		return ptr;
    }

    void swap(SharedPtr<T>& other) noexcept {
		T* temp_ptr = ptr;
		ptr = other.ptr;
		other.ptr = temp_ptr;

		std::size_t* temp_count = count;
		count = other.count;
		other.count = temp_count;
    }

    void reset(T* ptr = nullptr) {
	// NOTE: p.reset(p.get()) - will crush
		release_ownership();
		if (ptr != nullptr) {
			try {
				std::size_t* new_count = new std::size_t(1);
				this->count = new_count;
				this->ptr = ptr;
			} catch(const std::bad_alloc&) {
				delete ptr;
				throw;
			}
		}

    }


private:
    T* ptr;
    std::size_t* count;

    void release_ownership() noexcept {
		if (count == nullptr) {
	    	return;
		}

		--(*count);

		if (*count == 0) {
			delete ptr;
			delete count;
		}

		ptr = nullptr;
		count = nullptr;
    }
};


template <class T>
class SharedPtr<T[]> {
public:
    explicit SharedPtr(T* ptr = nullptr) : ptr(ptr), count(nullptr) {
		try {
			count = new std::size_t(1);
		} catch(const std::bad_alloc&) {
			delete[] ptr;
			throw;
		}

    }

    ~SharedPtr() {
		release_ownership();
    }

    SharedPtr(const SharedPtr<T[]>& other) : ptr(other.ptr), count(other.count) {
		if (count != nullptr) {
			++(*count);
		}
    }

    SharedPtr<T[]>& operator=(const SharedPtr<T[]>& other) {
		if (this != &other) {
			release_ownership();
			ptr = other.ptr;
			count = other.count;

			if (count != nullptr) {
				++(*count);
			}
		}

		return *this;
    }

    SharedPtr(SharedPtr<T[]>&& other) noexcept : ptr(other.ptr), count(other.count) {
		other.ptr = nullptr;
		other.count = nullptr;
    }

    SharedPtr<T[]>& operator=(SharedPtr<T[]>&& other) noexcept {
		if (this != &other) {
			release_ownership();

			ptr = other.ptr;
			count = other.count;

			other.ptr = nullptr;
			other.count = nullptr;
		}

		return *this;
    }

    explicit operator bool() noexcept {
		return ptr != nullptr;
    }

    T& operator[](std::size_t idx) const noexcept {
		return ptr[idx];
	}

    T* get() const noexcept {
		return ptr;
    }

    void swap(SharedPtr<T[]>& other) noexcept {
		T* temp_ptr = ptr;
		ptr = other.ptr;
		other.ptr = temp_ptr;

		std::size_t* temp_count = count;
		count = other.count;
		other.count = temp_count;
    }

    void reset(T* ptr = nullptr) {
	// NOTE: p.reset(p.get()) - will crush
		release_ownership();
		if (ptr != nullptr) {
			try {
				std::size_t* new_count = new std::size_t(1);
				this->count = new_count;
				this->ptr = ptr;
			} catch(const std::bad_alloc&) {
				delete[] ptr;
				throw;
			}
		}

    }


private:
    T* ptr;
    std::size_t* count;

    void release_ownership() noexcept {
		if (count == nullptr) {
	    	return;
		}

		--(*count);

		if (*count == 0) {
			delete[] ptr;
			delete count;
		}

		ptr = nullptr;
		count = nullptr;
    }
};


