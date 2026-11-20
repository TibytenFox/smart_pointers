#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>

#include "utility.hpp"
#include "../src/unique_ptr.hpp"
#include "../src/shared_ptr.hpp"

namespace {
	template <class T>
	inline void do_not_optimize(const T& val) {
		asm volatile("" : : "g"(val) : "memory");
	}

	template <class Func>
	double measure_ms(Func&& func) {
		auto start = std::chrono::steady_clock::now();
		func();
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		return elapsed.count();
	}

	constexpr std::size_t repetitions = 3;
	constexpr std::array<std::size_t, 3> default_scales{100'000, 1'000'000, 10'000'000};
	constexpr std::array<std::size_t, 3> array_loop_scales{10'000, 100'000, 1'000'000};

	using BenchFn = double (*)(std::size_t);

	template <std::size_t NScales>
	void run_scaled_benchmark(const char* title, 
		const std::array<std::size_t, NScales>& scales, 
		const std::array<const char*, 3>& names, 
		const std::array<BenchFn, 3>& funcs, 
		std::size_t warmup_iters) {
		
		std::cout << "\n=== " << title << " (median of " << repetitions << " runs, ms) ===\n" << std::setw(14) << "Scale";
		for (const char* name : names) {
			std::cout << std::setw(20) << name;
		}
		std::cout << '\n';

		for (std::size_t scale : scales) {
			for (BenchFn fn : funcs) {
				fn(warmup_iters);
			}

			std::array<std::array<double, repetitions>, 3> samples{};
			for (std::size_t rep = 0; rep < repetitions; ++rep) {
				for (std::size_t offset = 0; offset < funcs.size(); ++offset) {
					const std::size_t idx = (rep + offset) % funcs.size();
					samples[idx][rep] = funcs[idx](scale);
				}
			}

			std::cout << std::setw(14) << scale << std::fixed << std::setprecision(3);
			for (auto& s : samples) {
				std::sort(s.begin(), s.end());
				std::cout << std::setw(20) << s[repetitions / 2];
			}
			std::cout << '\n';
		}
	}

	double unique_create_raw(std::size_t n) {
		return measure_ms([n]() {
			for (std::size_t i = 0; i < n; ++i) {
				int* p = new int(static_cast<int>(i));
				do_not_optimize(p);
				delete p;
			}
		});
	}

	double unique_create_std(std::size_t n) {
		return measure_ms([n]() {
			for (std::size_t i = 0; i < n; ++i) {
				std::unique_ptr<int> p(new int(static_cast<int>(i)));
				do_not_optimize(p.get());
			}
		});
	}

	double unique_create_custom(std::size_t n) {
		return measure_ms([n]() {
			for (std::size_t i = 0; i < n; ++i) {
				UniquePtr<int> p(new int(static_cast<int>(i)));
				do_not_optimize(p.get());
			}
		});
		
	}

	double unique_move_raw(std::size_t n) {
		return measure_ms([n]() {
			int* src = new int(100);
			int* dst = nullptr;
			for (std::size_t i = 0; i < n; ++i) {
				dst = src;
				src = nullptr;
				do_not_optimize(dst);
				src = dst;
				dst = nullptr;
			}
			delete src;
			delete dst;
		});
	}

	double unique_move_std(std::size_t n) {
		return measure_ms([n]() {
			auto src = std::make_unique<int>(100);
			std::unique_ptr<int> dst;
			for (std::size_t i = 0; i < n; ++i) {
				dst = std::move(src);
				do_not_optimize(dst.get());
				src = std::move(dst);
			}
		});
	}

	double unique_move_custom(std::size_t n) {
		return measure_ms([n]() {
			UniquePtr<int> src(new int(100));
			UniquePtr<int> dst;
			for (std::size_t i = 0; i < n; ++i) {
				dst = std::move(src);
				do_not_optimize(dst.get());
				src = std::move(dst);
			}
		});
	}

	double shared_create_raw(std::size_t n) {
		return measure_ms([n]() {
			for (std::size_t i = 0; i < n; ++i) {
				int* p = new int(static_cast<int>(i));
				do_not_optimize(p);
				delete p;
			}
		});
	}

	double shared_create_std(std::size_t n) {
		return measure_ms([n]() {
			for (std::size_t i = 0; i < n; ++i) {
				auto p = std::make_shared<int>(static_cast<int>(i));
				do_not_optimize(p.get());
			}
		});
	}

	double shared_create_custom(std::size_t n) {
		return measure_ms([n]() {
			for (std::size_t i = 0; i < n; ++i) {
				SharedPtr<int> p(new int(static_cast<int>(i)));
				do_not_optimize(p.get());
			}
		});
	}

	double shared_copy_raw(std::size_t n) {
		int* raw_shared = new int(100);
		double elapsed = measure_ms([raw_shared, n]() {
			std::size_t ref_count = 1; // Чтобы уравнять с shared_ptr, так как сырой указатель не считает количество копий
			for (std::size_t i = 0; i < n; ++i) {
				int* copy = raw_shared;
				++ref_count;
				do_not_optimize(copy);
				--ref_count;
			}
			if (ref_count == 1) {
				do_not_optimize(raw_shared);
			}
		});
		delete raw_shared;
		return elapsed;
	}
	
	double shared_copy_std(std::size_t n) {
		auto std_base = std::make_shared<int>(42);
		return measure_ms([&std_base, n]() {
			for (std::size_t i = 0; i < n; ++i) {
				std::shared_ptr<int> copy = std_base;
				do_not_optimize(copy.get());
			}
		});
	}

	double shared_copy_custom(std::size_t n) {
		SharedPtr<int> custom_base(new int(42));
		return measure_ms([&custom_base, n]() {
			for (std::size_t i = 0; i < n; ++i) {
				SharedPtr<int> copy = custom_base;
				do_not_optimize(copy.get());
			}
		});
	}

	constexpr std::size_t ARR_SIZE = 1024;

	double array_index_raw(std::size_t loops) {
		int* raw_arr = new int[ARR_SIZE]{};
		for (std::size_t i = 0; i < ARR_SIZE; ++i) {
			raw_arr[i] = static_cast<int>(i);
		}
		double elapsed = measure_ms([raw_arr, loops]() {
			long long sum = 0;
			for (std::size_t l = 0; l < loops; ++l) {
				for (std::size_t i = 0; i < ARR_SIZE; ++i) {
					sum += raw_arr[i];
				}
			}
			do_not_optimize(sum);
		});
		delete[] raw_arr;
		return elapsed;
	}

	double array_index_std(std::size_t loops) {
		auto std_arr = std::make_unique<int[]>(ARR_SIZE);
		for (std::size_t i = 0; i < ARR_SIZE; ++i) {
			std_arr[i] = static_cast<int>(i);
		}
		return measure_ms([&std_arr, loops]() {
			long long sum = 0;
			for (std::size_t l = 0; l < loops; ++l) {
				for (std::size_t i = 0; i < ARR_SIZE; ++i) {
					sum += std_arr[i];
				}
			}
			do_not_optimize(sum);
		});
	}

	double array_index_custom(std::size_t loops) {
		UniquePtr<int[]> custom_arr(new int[ARR_SIZE]);
		for (std::size_t i = 0; i < ARR_SIZE; ++i) {
			custom_arr[i] = static_cast<int>(i);
		}
		return measure_ms([&custom_arr, loops]() {
			long long sum = 0;
			for (std::size_t l = 0; l < loops; ++l) {
				for (std::size_t i = 0; i < ARR_SIZE; ++i) {
					sum += custom_arr[i];
				}
			}
			do_not_optimize(sum);
		});
	}
}


TEST(Benchmark_UniquePtr_CreateAndDestroy) {
	run_scaled_benchmark("Unique Create & Destroy", 
		default_scales, 
		{"raw_ptr", "std::unique_ptr", "UniquePtr"}, 
		{unique_create_raw, unique_create_std, unique_create_custom},
		10'000);
}

TEST(Benchmark_UniquePtr_Move) {
    run_scaled_benchmark("Unique Move Two Times",
    	default_scales,
        {"raw ptr", "std::unique_ptr", "UniquePtr"},
        {unique_move_raw, unique_move_std, unique_move_custom},
        10'000);
}

TEST(Benchmark_SharedPtr_CreateAndDestroy) {
    run_scaled_benchmark("Shared Create & Destroy",
		default_scales,
		{"raw ptr", "std::shared_ptr", "SharedPtr"},
		{shared_create_raw, shared_create_std, shared_create_custom},
		10'000);
}

TEST(Benchmark_SharedPtr_CopyAndDestroy) {
    run_scaled_benchmark("Shared Copy & Destroy",
		default_scales,
		{"raw ptr", "std::shared_ptr", "SharedPtr"},
		{shared_copy_raw, shared_copy_std, shared_copy_custom},
		10'000);
}

TEST(Benchmark_Array_IndexAccess) {
    run_scaled_benchmark("Array Indexing Sum (1024 elems x loops)",
		array_loop_scales,
		{"raw ptr", "std::unique_ptr[]", "UniquePtr[]"},
		{array_index_raw, array_index_std, array_index_custom},
		1'000);
}
