#ifndef MERGE_SORT_UTILITY_H
#define MERGE_SORT_UTILITY_H
#include <hpc_helpers.hpp>
#include "generate_input_array.hpp"

using namespace std;

inline bool check_sort(const vector<reference_wrapper<Record> > &sorted) {
    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i - 1].get().key > sorted[i].get().key) {
            return false;
        }
    }
    return true;
}

inline void print_sort_res(const vector<reference_wrapper<Record> > &sorted) {
    if (check_sort(sorted)) {
        cout << "Sorting succeeded" << endl;
    } else {
        cout << "Sorting failed" << endl;
    }
}

inline bool second_bigger(const Record &a, const Record &b) {
    return a.key < b.key;
}

//merge the two part
//first is vector[start, middle]
//second is vector[middle + 1, end]
inline void merge(vector<reference_wrapper<Record> > &to_sort, size_t start, size_t middle, size_t end) {
    vector<reference_wrapper<Record> > temp;
    temp.reserve(end - start + 1);

    size_t left = start;
    size_t right = middle + 1;

    while (left <= middle && right <= end) {
        if (second_bigger(to_sort[left].get(), to_sort[right].get())) {
            temp.push_back(to_sort[left++]);
        } else {
            temp.push_back(to_sort[right++]);
        }
    }

    while (left <= middle) {
        temp.push_back(to_sort[left++]);
    }

    while (right <= end) {
        temp.push_back(to_sort[right++]);
    }

    // Copy back the sorted subrange
    for (size_t i = 0; i < temp.size(); ++i) {
        to_sort[start + i] = temp[i];
    }
}
#endif //MERGE_SORT_UTILITY_H
