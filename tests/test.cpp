#include <iostream>

int main() {
	std::cout << "=== RUNNING TESTS ===\n";

	extern int tests_passed;
	extern int tests_failed;

	std::cout << "\n=====================\n";
    std::cout << "Tests passed: " << tests_passed << '\n';
    std::cout << "Tests failed: " << tests_failed << '\n';

	return (tests_failed > 0) ? 1 : 0;
}