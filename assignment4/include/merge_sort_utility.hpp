#ifndef MERGE_SORT_UTILITY_H
#define MERGE_SORT_UTILITY_H
#include <hpc_helpers.hpp>
#include "generate_input_array.hpp"

using namespace std;

inline bool check_sort(const vector<Record> &sorted) {
    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i - 1].key > sorted[i].key) {
            return false;
        }
    }
    return true;
}

inline void print_sort_res(const vector<Record> &sorted) {
    if (check_sort(sorted)) {
        fprintf(stderr, "Sorting succeeded\n");
    } else {
        fprintf(stderr, "Sorting failed\n");
    }
}
#endif //MERGE_SORT_UTILITY_H
