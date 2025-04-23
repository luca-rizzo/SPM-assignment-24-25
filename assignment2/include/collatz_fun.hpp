#ifndef COLLATZ_FUN_H
#define COLLATZ_FUN_H
#include <cstdio>
#include <future>
#include <vector>

#include "collatz_fun.hpp"
#include "parse_utility.hpp"

using namespace std;


inline long calculate_collatz_length(long n) {
    if (n < 1) {
        return -1;
    }
    long collatz_length = 0;
    while (n != 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        collatz_length++;
    }

    return collatz_length;
}

// Get in input a vector of future and return the maximum values
inline long reduce_to_global_maximum(vector<future<long>>& local_maximum_futures) {
    long global_maximum = 0;
    for (auto &future: local_maximum_futures) {
        global_maximum = max(global_maximum, future.get());
    }
    return global_maximum;
}

#endif //COLLATZ_FUN_H
