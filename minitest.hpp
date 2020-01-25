#pragma once

#include <stdio.h>

#define _TEST_ABORT(...) do { printf("\033[1;31m\n\tFailed in %s:%i\033[0m"  "\n\tCondition: " #__VA_ARGS__ " \n", __FILE__, __LINE__); return -1; } while(0)
#define REQUIRE(...) do { ++_asserts_count; if (!(__VA_ARGS__)) _TEST_ABORT(__VA_ARGS__); } while(0)
#define SECTION(name) for (size_t _asserts_count = (printf("\033[1;32mTest case '%s'... \033[0m", name), fflush(stdout), 0), _flag = 1; _flag; _flag = (printf("\033[1;33mDONE, %zu assertions\n\033[0m", _asserts_count), 0))
