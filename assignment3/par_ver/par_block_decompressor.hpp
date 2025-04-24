#ifndef PAR_BLOCK_DECOMPRESSOR_H
#define PAR_BLOCK_DECOMPRESSOR_H

#include <atomic>
#include <fcntl.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "cmdline_param_parser.hpp"
#include "miniz.h"
#include "par_block_compressor.hpp"

using namespace std;


typedef struct DecompBlockInfo {
    size_t block_size;
    unsigned char *ptr;
} DecompBlockInfo;

typedef struct DecompBlockHeaderInfo {
    size_t orig_block_size;
    size_t comp_block_size;
    size_t offset;
} DecompBlockHeaderInfo;

inline vector<DecompBlockHeaderInfo> read_header(unsigned char *ptr) {
    //header: num_blocks block_size last_block_size comp_size_1 offset_1...comp_size_n offset_n
    vector<DecompBlockHeaderInfo> blocks;
    unsigned char *cur = ptr;
    size_t num_blocks = *reinterpret_cast<size_t *>(cur);
    cur += sizeof(size_t);
    size_t block_size = *reinterpret_cast<size_t *>(cur);
    cur += sizeof(size_t);
    size_t last_block_size = *reinterpret_cast<size_t *>(cur);
    cur += sizeof(size_t);

    if (num_blocks == 0)
        return blocks;

    blocks.resize(num_blocks);
    for (size_t i = 0; i < num_blocks; ++i) {
        size_t comp_size = *reinterpret_cast<size_t *>(cur);
        cur += sizeof(size_t);
        size_t offset = *reinterpret_cast<size_t *>(cur);
        cur += sizeof(size_t);
        blocks[i] = DecompBlockHeaderInfo{
            block_size,
            comp_size,
            offset
        };
    }
    blocks[num_blocks - 1].orig_block_size = last_block_size;
    return blocks;
}

static bool block_decompress(const string &filename, const CompressionParams &cpar) {
    if (!filename.ends_with(".zip")) {
        std::fprintf(stderr, "%s is not a zip: the file will not be decompressed\n", filename.c_str());
        return false;
    }
    size_t filesize = 0;
    unsigned char *ptr = nullptr;
    if (!mapFile(filename.c_str(), filesize, ptr, cpar)) {
        if (cpar.quite_mode >= 1)
            std::fprintf(stderr, "mapFile %s failed\n", filename.c_str());
        exit(EXIT_FAILURE);
    }
    vector<DecompBlockHeaderInfo> block_header = read_header(ptr);
    vector<DecompBlockInfo> decompr_blocks;
    decompr_blocks.resize(block_header.size());
    size_t header_size = sizeof(size_t) * 3 + block_header.size() * (sizeof(size_t) * 2);
    unsigned char *data_ptr = ptr + header_size;
    std::atomic_bool any_error(false);
#pragma omp taskloop grainsize(1) nogroup shared(decompr_blocks, block_header, data_ptr, any_error)
    for (size_t i = 0; i < block_header.size(); i += 1) {
        DecompBlockHeaderInfo blk = block_header[i];
        auto *ptrOut = new unsigned char[blk.orig_block_size];
        unsigned char *inPtr = data_ptr + blk.offset;
        mz_ulong orig_len = blk.orig_block_size;
        int cmp_status = uncompress(ptrOut, &orig_len, inPtr, blk.comp_block_size);
        if (cmp_status != Z_OK) {
            if (cpar.quite_mode >= 1)
                std::fprintf(stderr, "uncompress failed!\n");
            any_error = true;
        } else {
            size_t block_index = i;
            decompr_blocks[block_index] = DecompBlockInfo{orig_len, ptrOut};
        }
    }
#pragma omp taskwait
    if (any_error != true) {
        //write ordered to file
        std::string outfile = filename.substr(0, filename.size() - 4); // remove the SUFFIX (i.e., .zip)
        std::ofstream outFile(outfile, std::ios::binary);
        if (!outFile.is_open()) {
            std::fprintf(stderr, "Cannot open output file!\n");
            any_error = true;
        } else {
            for (const auto &blk: decompr_blocks) {
                outFile.write(reinterpret_cast<const char *>(blk.ptr), blk.block_size);
            }
            outFile.close();
        }
    }
    for (const auto &blk: decompr_blocks) {
        delete[] blk.ptr;
    }
    unmapFile(ptr, filesize, cpar);

    return !any_error;
}

#endif //PAR_BLOCK_DECOMPRESSOR_H
