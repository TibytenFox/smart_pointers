#include <iostream>
#include "utility.hpp"
#include "../src/shared_ptr.hpp"

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

TEST(SharedPtr_DefaultConstructor) {
	SharedPtr<int> p;
	ASSERT_EQ(bool(p), false);
	ASSERT_EQ(p.get(), nullptr);
}

TEST(SharedPtr_RawPointerConstructor) {
	int* raw_p = new int(123);
	SharedPtr<int> p(raw_p);
	ASSERT_EQ(bool(p), true);
	ASSERT_EQ(p.get(), raw_p);
	ASSERT_EQ(*p, 123);
}

TEST(SharedPtr_NullPointerConstructor) {
	SharedPtr<int> p(nullptr);
	ASSERT_EQ(bool(p), false);
	ASSERT_EQ(p.get(), nullptr);
}

TEST(SharedPtr_CopyConstructor) {
	SharedPtr<int> p1(new int(100));
	SharedPtr<int> p2(p1);

	ASSERT_EQ(bool(p1), true);
	ASSERT_EQ(bool(p2), true);
	ASSERT_EQ(p1.get(), p2.get());
	ASSERT_EQ(*p1, 100);
	ASSERT_EQ(*p2, 100);

	*p2 = 123;
	ASSERT_EQ(*p1, 123);
}

TEST(SharedPtr_CopyAssignment) {
	SharedPtr<int> p1(new int(1));
	SharedPtr<int> p2(new int(2));

	p2 = p1;
	ASSERT_EQ(p1.get(), p2.get());
	ASSERT_EQ(*p1, 1);
	ASSERT_EQ(*p2, 1);
}

TEST(SharedPtr_SelfCopyAssignment) {
	SharedPtr<int> p(new int(100));
	int* raw_p = p.get();
	p = p;

	ASSERT_EQ(bool(p), true);
	ASSERT_EQ(p.get(), raw_p);
	ASSERT_EQ(*p, 100);
}

TEST(SharedPtr_MoveConstructor) {
	SharedPtr<int> p1(new int(100));
	SharedPtr<int> p2(std::move(p1));

	ASSERT_EQ(bool(p1), false);
	ASSERT_EQ(p1.get(), nullptr);
	ASSERT_EQ(bool(p2), true);
	ASSERT_EQ(*p2, 100);
}

TEST(SharedPtr_MoveAssignment) {
	SharedPtr<int> p1(new int(100));
	SharedPtr<int> p2(new int(200));

	p2 = std::move(p1);

	ASSERT_EQ(bool(p1), false);
	ASSERT_EQ(p1.get(), nullptr);
	ASSERT_EQ(bool(p2), true);
	ASSERT_EQ(*p2, 100);
}

TEST(SharedPtr_SelfMoveAssignment) {
	SharedPtr<int> p(new int(500));
	int* raw_p = p.get();

	p = std::move(p);
	
	ASSERT_EQ(bool(p), true);
	ASSERT_EQ(p.get(), raw_p);
	ASSERT_EQ(*p, 500);
}

TEST(SharedPtr_Reset) {
	SharedPtr<int> p1(new int(100));
	SharedPtr<int> p2(p1);

	p1.reset(new int(200));
	ASSERT_EQ(*p1, 200);
	ASSERT_EQ(*p2, 100);

	p1.reset();
	ASSERT_EQ(bool(p1), false);
	ASSERT_EQ(p1.get(), nullptr);
	ASSERT_EQ(*p2, 100);
}

TEST(SharedPtr_Swap) {
	SharedPtr<int> p1(new int(100));
	SharedPtr<int> p2(new int(200));

	p1.swap(p2);

	ASSERT_EQ(*p1, 200);
	ASSERT_EQ(*p2, 100);
}

TEST(SharedPtr_DereferenceAndArrow) {
	SharedPtr<int> p(new int(123));
	*p = 100;
	ASSERT_EQ(*p, 100);

	struct Point {
		int x, y;
	};

	SharedPtr<Point> pt(new Point{5, 10});
	ASSERT_EQ(pt->x, 5);
	ASSERT_EQ(pt->y, 10);
}

TEST(SharedPtr_Polymorphism) {
	SharedPtr<Base> p1(new Derived());
	SharedPtr<Base> p2(p1);

	ASSERT_EQ(bool(p1), true);
	ASSERT_EQ(p1->id(), 2);
	ASSERT_EQ(p2->id(), 2);
}

TEST(SharedPtr_ReferenceCountingAndDestructorCalls) {
	DestructorTracker::construct_count = 0;
	DestructorTracker::destruct_count = 0;

	{
		SharedPtr<DestructorTracker> p1(new DestructorTracker(1));
		ASSERT_EQ(DestructorTracker::construct_count, 1);
		ASSERT_EQ(DestructorTracker::destruct_count, 0);

		{
			SharedPtr<DestructorTracker> p2(p1);
			SharedPtr<DestructorTracker> p3;
			p3 = p2;

			ASSERT_EQ(DestructorTracker::construct_count, 1);
			ASSERT_EQ(DestructorTracker::destruct_count, 0);

			p2.reset();
			ASSERT_EQ(DestructorTracker::destruct_count, 0);
		}

		ASSERT_EQ(DestructorTracker::destruct_count, 0);
	}

	ASSERT_EQ(DestructorTracker::destruct_count, 1);
}

TEST(SharedPtr_Array_DestructorTracking) {
	DestructorTracker::construct_count = 0;
	DestructorTracker::destruct_count = 0;

	{
		SharedPtr<DestructorTracker[]> arr1(new DestructorTracker[3]);
		ASSERT_EQ(DestructorTracker::construct_count, 3);
		ASSERT_EQ(DestructorTracker::destruct_count, 0);

		{
			SharedPtr<DestructorTracker[]> arr2(arr1);
			ASSERT_EQ(DestructorTracker::destruct_count, 0);
		}
	}

	ASSERT_EQ(DestructorTracker::destruct_count, 3);
}

TEST(SharePtr_Array_CopySemantics) {
	SharedPtr<int[]> arr1(new int[3]{100, 200, 300});
	SharedPtr<int[]> arr2(arr1);

	ASSERT_EQ(arr1.get(), arr2.get());
	ASSERT_EQ(arr1[0], 100);
	ASSERT_EQ(arr1[1], 200);
	ASSERT_EQ(arr1[2], 300);

	arr2[1] = 0;
	ASSERT_EQ(arr1[1], 0);

	SharedPtr<int[]> arr3(new int[2]{1, 2});
	arr3 = arr1;
	ASSERT_EQ(arr1.get(), arr3.get());
	ASSERT_EQ(arr3[1], 0);
}

TEST(SharedPtr_Array_MoveSemantics) {
	SharedPtr<int[]> arr1(new int[3]{10, 20, 30});
	SharedPtr<int[]> arr2(std::move(arr1));

	ASSERT_EQ(bool(arr1), false);
	ASSERT_EQ(arr1.get(), nullptr);
	ASSERT_EQ(bool(arr2), true);
	ASSERT_EQ(arr2[0], 10);
	ASSERT_EQ(arr2[2], 30);

	SharedPtr<int[]> arr3(new int[2]{1, 2});
	arr3 = std::move(arr2);
	ASSERT_EQ(bool(arr2), false);
	ASSERT_EQ(bool(arr3), true);
	ASSERT_EQ(arr3[0], 10);
	ASSERT_EQ(arr3[1], 20);
}

TEST(SharedPtr_Array_SelfAssignment) {
	SharedPtr<int[]> arr(new int[2]{6, 7});
	int* raw_p = arr.get();

	arr = arr;
	ASSERT_EQ(bool(arr), true);
	ASSERT_EQ(arr.get(), raw_p);
	ASSERT_EQ(arr[0], 6);
	ASSERT_EQ(arr[1], 7);
}

TEST(SharedPtr_Array_SwapAndReset) {
	SharedPtr<int[]> arr1(new int[2]{1, 2});
	SharedPtr<int[]> arr2(new int[2]{99, 100});

	arr1.swap(arr2);

	ASSERT_EQ(arr1[0], 99);
	ASSERT_EQ(arr1[1], 100);
	ASSERT_EQ(arr2[0], 1);
	ASSERT_EQ(arr2[1], 2);

	SharedPtr<int[]> arr_copy(arr1);
	arr1.reset(new int[2]{5, 6});

	ASSERT_EQ(arr1[0], 5);
	ASSERT_EQ(arr_copy[0], 99);

	arr1.reset();
	ASSERT_EQ(bool(arr1), false);
	ASSERT_EQ(arr1.get(), nullptr);
}