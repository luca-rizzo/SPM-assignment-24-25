#ifndef PAR_BLOCK_COMPRESSOR_H
#define PAR_BLOCK_COMPRESSOR_H
#include <vector>
#include <atomic>
#include <fcntl.h>
#include <string>
#include <omp.h>
#include <filesystem>
#include <fstream>
#include "miniz.h"
#include "par_utility.hpp"

using namespace std;

typedef struct CompBlockInfo {
    size_t comp_block_size;
    size_t orig_block_size;
    unsigned char *ptr;
} CompBlockInfo;

inline size_t compute_block_size(size_t filesize) {
    int num_threads = omp_get_max_threads();

    size_t block_size = filesize / (num_threads);

    return block_size;
}

inline void write_header(const vector<CompBlockInfo> &blocks, std::ofstream &outFile) {
    //header: num_blocks block_size last_block_size comp_size_1 offset_1...comp_size_n offset_n
    size_t num_blocks = blocks.size();
    size_t block_size = blocks[0].orig_block_size;
    size_t last_block_size = blocks[num_blocks - 1].orig_block_size;
    outFile.write(reinterpret_cast<const char *>(&num_blocks), sizeof(size_t));
    outFile.write(reinterpret_cast<const char *>(&block_size), sizeof(size_t));
    outFile.write(reinterpret_cast<const char *>(&last_block_size), sizeof(size_t));
    size_t offset = 0;
    for (const auto& blk: blocks) {
        size_t comp_size = blk.comp_block_size;
        outFile.write(reinterpret_cast<const char *>(&comp_size), sizeof(size_t));
        outFile.write(reinterpret_cast<const char *>(&offset), sizeof(size_t));
        offset += comp_size;
    }
}

static inline bool block_compress(const string &filename, const CompressionParams &cpar) {
    size_t filesize = 0;
    unsigned char *ptr = nullptr;
    if (!mapFile(filename.c_str(), filesize, ptr, cpar)) {
        if (cpar.quite_mode >= 1)
            std::fprintf(stderr, "mapFile %s failed\n", filename.c_str());
        exit(EXIT_FAILURE);
    }
    vector<CompBlockInfo> blocks;
    size_t block_size = compute_block_size(filesize);
    size_t num_blocks = (filesize + block_size - 1) / block_size;
    blocks.resize(num_blocks);
    std::atomic_bool any_error(false);
#pragma omp taskloop grainsize(1) nogroup shared(blocks, ptr, any_error)
    for (size_t i = 0; i < filesize; i += block_size) {
        //each thread compress a block and store the various reference to
        size_t eff_block_size = min(block_size, filesize - i);
        unsigned char *inPtr = ptr + i;
        // get an estimation of the maximum compression size
        mz_ulong cmp_len = compressBound(eff_block_size);
        // allocate memory to store compressed data in memory
        auto *ptrOut = new unsigned char[cmp_len];
        if (compress(ptrOut, &cmp_len, inPtr, eff_block_size) != Z_OK) {
            if (cpar.quite_mode >= 1)
                std::fprintf(stderr, "Error compressing block %lu of file %s\n", i, filename.c_str());
            delete [] ptrOut;
            any_error = true;
        } else {
            size_t block_index = i / block_size;
            blocks[block_index] = CompBlockInfo{cmp_len, eff_block_size, ptrOut};
        }
    }
#pragma omp taskwait
    if (any_error != true) {
        //write ordered to file
        std::ofstream outFile(filename + ".zip", std::ios::binary);
        if (!outFile.is_open()) {
            std::fprintf(stderr, "Cannot open output file!\n");
            any_error = true;
        } else {
            //Write header
            write_header(blocks, outFile);
            //Write blocks content
            for (const auto &blk: blocks) {
                outFile.write(reinterpret_cast<const char *>(blk.ptr), blk.comp_block_size);
            }
            outFile.close();
        }
    }
    for (const auto &blk: blocks) {
        delete[] blk.ptr;
    }
    unmapFile(ptr, filesize, cpar);


    return !any_error;
}

#endif //PAR_BLOCK_COMPRESSOR_H
