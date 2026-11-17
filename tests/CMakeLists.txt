cmake_minimum_required(VERSION 3.14)
project(SmartPointers VERSION 1.0.0 LANGUAGES CXX) 

add_executable(test 
	test.cpp
	test_unique.cpp
	test_shared.cpp
)
target_compile_features(test PRIVATE cxx_std_17)
