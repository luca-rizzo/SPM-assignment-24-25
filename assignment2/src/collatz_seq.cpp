#include <iostream>
#include <string>

#include "collatz_fun.hpp"
#include "hpc_helpers.hpp"
#include "parse_utility.hpp"

long find_max_collatz_seq_in_range(std::pair<long, long> range) {
    long global_max = 0;
    for (long i = range.first; i <= range.second; i++) {
        long collatz_length = calculate_collatz_length(i);
        global_max = std::max(global_max, collatz_length);
    }
    return global_max;
}

int main(int argc, char** argv) {
    std::vector<std::pair<long, long>> ranges;
    for (int i = 1; i < argc; ++i) {
        ranges.emplace_back(parseRange(argv[i]));
    }
    TIMERSTART(collatz_seq);
    for (const auto &range: ranges) {
        std::fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second,find_max_collatz_seq_in_range(range));
    }
    TIMERSTOP(collatz_seq);
}