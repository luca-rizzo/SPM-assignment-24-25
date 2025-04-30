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
    //header: num_blocks block_size last_block_size comp_size_1 ...comp_size_n
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
    size_t offset = 0;
    for (size_t i = 0; i < num_blocks; ++i) {
        size_t comp_size = *reinterpret_cast<size_t *>(cur);
        cur += sizeof(size_t);
        blocks[i] = DecompBlockHeaderInfo{
            block_size,
            comp_size,
            offset
        };
        offset += comp_size;
    }
    blocks[num_blocks - 1].orig_block_size = last_block_size;
    return blocks;
}

static void cleanup_resources(const CompressionParams &cpar, size_t filesize, unsigned char *ptr,
                              const vector<DecompBlockInfo> &decompr_blocks) {
    for (const auto &blk: decompr_blocks) {
        delete[] blk.ptr;
    }
    unmapFile(ptr, filesize, cpar);
}

static bool block_decompress(const string &filename, const CompressionParams &cpar) {
    if (!filename.ends_with(COMP_FILE_SUFFIX)) {
        log_msg(VERBOSE, cpar, "%s is not a zip: the file will not be decompressed\n", filename.c_str());
        return false;
    }
    size_t filesize = 0;
    unsigned char *ptr = nullptr;
    if (!mapFile(filename.c_str(), filesize, ptr, cpar)) {
        log_msg(ERROR, cpar, "mapFile %s failed\n", filename.c_str());
        return false;
    }
    vector<DecompBlockHeaderInfo> block_header = read_header(ptr);
    vector<DecompBlockInfo> decompr_blocks;
    decompr_blocks.resize(block_header.size());
    size_t header_size = sizeof(size_t) * 3 + block_header.size() * sizeof(size_t);
    unsigned char *data_ptr = ptr + header_size;
    bool any_error = false;
#pragma omp taskloop grainsize(1) shared(decompr_blocks, block_header, data_ptr) reduction(|:any_error) if (block_header.size() > 1)
    for (size_t i = 0; i < block_header.size(); i++) {
        DecompBlockHeaderInfo blk = block_header[i];
        auto *ptr_out = new (nothrow) unsigned char[blk.orig_block_size];
        if (!ptr_out) {
            log_msg(ERROR, cpar, "Memory allocation failed for block %lu\n", i);
            any_error = true;
            continue;
        }
        unsigned char *inPtr = data_ptr + blk.offset;
        mz_ulong orig_len = blk.orig_block_size;
        if (uncompress(ptr_out, &orig_len, inPtr, blk.comp_block_size) != Z_OK) {
            log_msg(ERROR, cpar, "uncompress failed!\n", filename.c_str());
            any_error = true;
        } else {
            size_t block_index = i;
            decompr_blocks[block_index] = DecompBlockInfo{orig_len, ptr_out};
        }
    }
    if (any_error) {
        cleanup_resources(cpar, filesize, ptr, decompr_blocks);
        return false;
    }
    std::string outfile = filename.substr(0, filename.size() - COMP_FILE_SUFFIX.size());
    try {
        std::ofstream outFile;
        outFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        outFile.open(outfile, std::ios::binary);
        for (const auto &blk: decompr_blocks) {
            outFile.write(reinterpret_cast<const char *>(blk.ptr), blk.block_size);
        }
        outFile.close();
    } catch (const std::ios_base::failure &e) {
        log_msg(ERROR, cpar, "I/O error writing decompressed file %s: %s\n", outfile.c_str(), e.what());
        std::filesystem::remove(outfile);
        any_error = true;
    }
    cleanup_resources(cpar, filesize, ptr, decompr_blocks);

    return !any_error;
}

#endif //PAR_BLOCK_DECOMPRESSOR_H
