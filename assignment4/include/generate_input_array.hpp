#ifndef GENERATE_INPUT_ARRAY_H
#define GENERATE_INPUT_ARRAY_H
#include <cstdlib>
#include <ctime>
#include "generate_input_array.hpp"
#include <random>

struct Record {
    unsigned long key;
    std::vector<char> rpayload;
};

std::vector<Record> generate_input_array(size_t N, size_t payload_size) {
    std::mt19937 rng(static_cast<unsigned long>(std::time(nullptr)));
    std::uniform_int_distribution<unsigned long> key_dist(1, 100000);
    std::uniform_int_distribution<char> char_dist('A', 'Z');

    std::vector<Record> records(N);
    for (size_t i = 0; i < N; ++i) {
        records[i].key = key_dist(rng);
        records[i].rpayload.resize(payload_size);
        for (size_t j = 0; j < payload_size; ++j) {
            records[i].rpayload[j] = char_dist(rng);
        }
    }

    return records;  // MOVE, non copia!
}


#endif //GENERATE_INPUT_ARRAY_H
