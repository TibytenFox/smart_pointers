#include <iostream>
#include "utility.hpp"
#include "../src/unique_ptr.hpp"

// Для корректной работы test_unique и test_shared
namespace {
	struct Base {
		virtual ~Base() = default;
		virtual int id() const { return 1; }
	};

	struct Derived : public Base {
		int id() const override { return 2; }
	};

	struct DestructorTracker {
		inline static int construct_count = 0;
		inline static int destruct_count = 0;
		int value;

		explicit DestructorTracker(int v = 0) : value(v) { ++construct_count; }
		~DestructorTracker() { ++destruct_count; }
	};
}

TEST(UniquePtr_DefaultConstructor) {
	UniquePtr<int> p;
	ASSERT_EQ(bool(p), false);
	ASSERT_EQ(p.get(), nullptr);
}

TEST(UniquePtr_RawPointerConstructor) {
	int* raw_p = new int(123);
	UniquePtr<int> p(raw_p);
	ASSERT_EQ(bool(p), true);
	ASSERT_EQ(p.get(), raw_p);
	ASSERT_EQ(*p, 123);
}

TEST(UniquePtr_NullptrConstructor) {
	UniquePtr<int> p(nullptr);
	ASSERT_EQ(bool(p), false);
	ASSERT_EQ(p.get(), nullptr);
}

TEST(UniquePtr_MoveConstructor) {
	UniquePtr<int> p1(new int(100));
	UniquePtr<int> p2(std::move(p1));

	ASSERT_EQ(bool(p1), false);
	ASSERT_EQ(p1.get(), nullptr);
	ASSERT_EQ(bool(p2), true);
	ASSERT_EQ(*p2, 100);
}

TEST(UniquePtr_MoveAssignment) {
	UniquePtr<int> p1(new int(100));
	UniquePtr<int> p2(new int(200));

	p2 = std::move(p1);

	ASSERT_EQ(bool(p1), false);
	ASSERT_EQ(p1.get(), nullptr);
	ASSERT_EQ(bool(p2), true);
	ASSERT_EQ(*p2, 100);
}

TEST(UniquePtr_SelfMoveAssignment) {
	UniquePtr<int> p1(new int(500));

	int* raw_ptr = p1.get();
	p1 = std::move(p1);

	ASSERT_EQ(bool(p1), true);
	ASSERT_EQ(p1.get(), raw_ptr);
	ASSERT_EQ(*p1, 500);
}

TEST(UniquePtr_Release) {
	UniquePtr<int> p(new int(100));
	int* raw_ptr = p.release();

	ASSERT_EQ(bool(p), false);
	ASSERT_EQ(p.get(), nullptr);
	ASSERT_EQ(*raw_ptr, 100);

	delete raw_ptr;
}

TEST(UniquePtr_Reset) {
	UniquePtr<int> p(new int(10));

	p.reset(new int(20));
	ASSERT_EQ(bool(p), true);
	ASSERT_EQ(*p, 20);

	p.reset();
	ASSERT_EQ(bool(p), false);
	ASSERT_EQ(p.get(), nullptr);

	int* raw_ptr = new int(30);
	p.reset(raw_ptr);
	p.reset(raw_ptr);
	ASSERT_EQ(*p, 30);
}

TEST(UniquePtr_Swap) {
	UniquePtr<int> p1(new int(1));
    UniquePtr<int> p2(new int(2));

	p1.swap(p2);

	ASSERT_EQ(*p1, 2);
	ASSERT_EQ(*p2, 1);
}

TEST(UniquePtr_DereferenceAndArrow) {
	UniquePtr<int> p(new int(99));
	*p = 100;
	ASSERT_EQ(*p, 100);

	struct Point {
		int x, y;
	};

	UniquePtr<Point> pt(new Point{5, 10});
	ASSERT_EQ(pt->x, 5);
	ASSERT_EQ(pt->y, 10);
}

TEST(UniquePtr_Polymorphism) {
	UniquePtr<Base> p(new Derived());
	ASSERT_EQ(bool(p), true);
	ASSERT_EQ(p->id(), 2);
}

TEST(UniquePtr_DestructorCalls) {
	DestructorTracker::construct_count = 0;
    DestructorTracker::destruct_count = 0;

	{
		UniquePtr<DestructorTracker> p1(new DestructorTracker(1));
		UniquePtr<DestructorTracker> p2(new DestructorTracker(2));

		ASSERT_EQ(DestructorTracker::construct_count, 2);
		ASSERT_EQ(DestructorTracker::destruct_count, 0);

		p1.reset();
		ASSERT_EQ(DestructorTracker::destruct_count, 1);
	}
	ASSERT_EQ(DestructorTracker::destruct_count, 2);
}

TEST(UniquePtr_Array_DestructorTracking) {
	DestructorTracker::construct_count = 0;
	DestructorTracker::destruct_count = 0;

	{
		UniquePtr<DestructorTracker[]> arr(new DestructorTracker[3]);

		ASSERT_EQ(DestructorTracker::construct_count, 3);
		ASSERT_EQ(DestructorTracker::destruct_count, 0);
	}

	ASSERT_EQ(DestructorTracker::destruct_count, 3);
}

TEST(UniquePtr_Array_MoveSemantics) {
	UniquePtr<int[]> arr1(new int[3]{10, 20, 30});

	UniquePtr<int[]> arr2(std::move(arr1));
	ASSERT_EQ(bool(arr1), false);
    ASSERT_EQ(arr1.get(), nullptr);
    ASSERT_EQ(bool(arr2), true);
    ASSERT_EQ(arr2[0], 10);
    ASSERT_EQ(arr2[2], 30);

	UniquePtr<int[]> arr3(new int[2]{1, 2});
    arr3 = std::move(arr2);
    ASSERT_EQ(bool(arr2), false);
    ASSERT_EQ(bool(arr3), true);
    ASSERT_EQ(arr3[0], 10);
    ASSERT_EQ(arr3[1], 20);
}

TEST(UniquePtr_Array_SelfMove) {
	UniquePtr<int[]> arr(new int[2]{6, 7});
	int* raw_ptr = arr.get();

	arr = std::move(arr);

	ASSERT_EQ(bool(arr), true);
	ASSERT_EQ(arr.get(), raw_ptr);
	ASSERT_EQ(arr[0], 6);
	ASSERT_EQ(arr[1], 7);
}

TEST(UniquePtr_Array_Swap) {
    UniquePtr<int[]> arr1(new int[2]{1, 2});
    UniquePtr<int[]> arr2(new int[2]{99, 100});

    arr1.swap(arr2);

    ASSERT_EQ(arr1[0], 99);
    ASSERT_EQ(arr1[1], 100);
    ASSERT_EQ(arr2[0], 1);
    ASSERT_EQ(arr2[1], 2);
}

TEST(UniquePtr_Array_ResetAndRelease) {
	UniquePtr<int[]> arr(new int[2]{7, 8});

	int* raw_ptr = arr.release();
	ASSERT_EQ(bool(arr), false);
	ASSERT_EQ(raw_ptr[0], 7);
	ASSERT_EQ(raw_ptr[1], 8);
	delete[] raw_ptr;

	UniquePtr<int[]> arr2(new int[2]{1, 2});
    arr2.reset(new int[3]{3, 4, 5});
    ASSERT_EQ(bool(arr2), true);
    ASSERT_EQ(arr2[0], 3);
    ASSERT_EQ(arr2[2], 5);

	arr2.reset();
    ASSERT_EQ(bool(arr2), false);
    ASSERT_EQ(arr2.get(), nullptr);

	int* shared_raw = new int[2]{10, 30};
	UniquePtr<int[]> arr3(shared_raw);
	arr3.reset(shared_raw);
	ASSERT_EQ(arr3[0], 10);
	ASSERT_EQ(arr3[1], 30);
}

TEST(UniquePtr_Array_ConstAndModification) {
	UniquePtr<int[]> arr(new int[3]{1, 2, 3});

	arr[0] = 6;
	arr[2] = 7;

	ASSERT_EQ(arr[0], 6);
	ASSERT_EQ(arr[1], 2);
	ASSERT_EQ(arr[2], 7);
}



