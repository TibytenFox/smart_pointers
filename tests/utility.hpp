#pragma once
#include <iostream>

#define TEST(name) void name(); \
    struct Register_##name { Register_##name() { register_test(name); std::cout << #name << '\n'; } } reg_##name; \
    void name()

inline int tests_passed = 0;
inline int tests_failed = 0;

inline void register_test(void (*f)()) {
    try {
        f();
        ++tests_passed;
        std::cout << "[PASS]\t";
    } catch (const std::exception &e) {
        ++tests_failed;
        std::cout << "[FAIL] " << e.what() << "\t";
    }
}

#define ASSERT_EQ(x, y) \
    if ((x) != (y)) throw std::runtime_error("ASSERT_EQ failed: " #x " != " #y)

#define ASSERT_THROWS(expr, exc) \
    do { \
        bool caught = false; \
        try { expr; } \
        catch (const exc&) { caught = true; } \
        catch (...) {} \
        if (!caught) throw std::runtime_error("ASSERT_THROWS failed: " #expr); \
    } while(0)

#define ASSERT_NO_THROW(expr) \
    do { \
        try { expr; } \
        catch (...) { throw std::runtime_error("ASSERT_NO_THROW failed: " #expr); } \
    } while(0)
