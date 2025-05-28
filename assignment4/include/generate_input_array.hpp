#ifndef GENERATE_INPUT_ARRAY_H
#define GENERATE_INPUT_ARRAY_H
#include <cstdlib>
#include "generate_input_array.hpp"
#include <random>

using namespace std;

struct Record {
    unsigned long key;
    char* payload;

    bool operator<(const Record& other) const {
        return key < other.key;
    }
    bool operator>(const Record& other) const {
        return other < *this;
    }
    bool operator<=(const Record& other) const {
        return !(other < *this);
    }
    bool operator>=(const Record& other) const {
        return !(*this < other);
    }
    bool operator==(const Record& other) const {
        return key == other.key;
    }
    bool operator!=(const Record& other) const {
        return !(*this == other);
    }
};

inline vector<Record> generate_input_array(size_t N, size_t payload_size) {
    mt19937 rng(static_cast<unsigned long>(42));
    uniform_int_distribution<unsigned long> key_dist(1, 100000);
    uniform_int_distribution<char> char_dist('A', 'Z');

    vector<Record> records(N);
    for (size_t i = 0; i < N; ++i) {
        records[i].key = key_dist(rng);
        records[i].payload = new char[payload_size];
        for (size_t j = 0; j < payload_size; ++j) {
            records[i].payload[j] = char_dist(rng);
        }
    }

    return records;
}

#endif //GENERATE_INPUT_ARRAY_H
