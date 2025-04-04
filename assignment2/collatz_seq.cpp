#include <iostream>
#include <regex>
#include <string>

#include "hpc_helpers.hpp"

long calculate_collatz_length(long n) {
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

std::pair<long, long> parseRange(const std::string &rangeStr) {
    std::regex pattern(R"(^\s*(\d+)\s*-\s*(\d+)\s*$)");
    std::smatch match;

    if (!std::regex_match(rangeStr, match, pattern)) {
        std::cerr << "Not valid range format: " << rangeStr << std::endl;
        exit(EXIT_FAILURE);
    }
    long start = std::stol(match[1].str());
    long end = std::stol(match[2].str());
    return {start, end};
}

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