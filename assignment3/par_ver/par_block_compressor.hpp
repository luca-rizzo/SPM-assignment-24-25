#ifndef PAR_BLOCK_COMPRESSOR_H
#define PAR_BLOCK_COMPRESSOR_H

#include <vector>
#include <cstdarg>
#include <string>
#include <omp.h>
#include "miniz.h"
#include "par_utility.hpp"

using namespace std;

typedef struct CompBlockInfo {
    size_t comp_block_size;
    size_t orig_block_size;
    unsigned char *ptr;
} CompBlockInfo;

inline size_t compute_block_size(size_t filesize) {
    constexpr size_t MIN_BLOCK_SIZE = 1ULL << 19; //512kb
    constexpr size_t MAX_BLOCK_SIZE = 1ULL << 26; //64mb
    int num_threads = omp_get_max_threads();

    size_t block_size = filesize / num_threads;

    if (block_size < MIN_BLOCK_SIZE)
        block_size = MIN_BLOCK_SIZE;
    else if (block_size > MAX_BLOCK_SIZE)
        block_size = MAX_BLOCK_SIZE;

    return block_size;
}

inline void write_header(unsigned char *compressed_ptr,
                         const vector<size_t> &header) {
    size_t header_size = header.size() * sizeof(size_t);
    memcpy(compressed_ptr, header.data(), header_size);
}

inline void seq_write_header(const vector<CompBlockInfo> &blocks, std::ofstream &outFile) {
    //header: num_blocks block_size last_block_size offset_1... offset_n
    size_t num_blocks = blocks.size();
    size_t block_size = blocks[0].orig_block_size;
    size_t last_block_size = blocks[num_blocks - 1].orig_block_size;
    outFile.write(reinterpret_cast<const char *>(&num_blocks), sizeof(size_t));
    outFile.write(reinterpret_cast<const char *>(&block_size), sizeof(size_t));
    outFile.write(reinterpret_cast<const char *>(&last_block_size), sizeof(size_t));
    for (const auto &blk: blocks) {
        size_t comp_size = blk.comp_block_size;
        outFile.write(reinterpret_cast<const char *>(&comp_size), sizeof(size_t));
    }
}

inline void cleanup_resources(const CompressionParams &cpar, size_t filesize,
                              unsigned char *ptr, vector<CompBlockInfo> blocks) {
    for (auto &blk: blocks) {
        delete[] blk.ptr;
        blk.ptr = nullptr;
    }

    unmapFile(ptr, filesize, cpar);
}

inline size_t calculate_compressed_dim(const vector<CompBlockInfo> &blocks,
                                       vector<size_t> &header,
                                       vector<size_t> &write_offsets) {
    size_t num_blocks = blocks.size();
    size_t block_size = blocks[0].orig_block_size;
    size_t last_block_size = blocks[num_blocks - 1].orig_block_size;
    header.reserve(3 + blocks.size());
    header.push_back(num_blocks);
    header.push_back(block_size);
    header.push_back(last_block_size);
    size_t in_file_offset = sizeof(size_t) * (3 + blocks.size());
    for (size_t i = 0; i < blocks.size(); ++i) {
        const auto &blk = blocks[i];
        header.push_back(blk.comp_block_size);
        write_offsets[i] = in_file_offset;
        in_file_offset += blk.comp_block_size;
    }

    size_t header_size = header.size() * sizeof(size_t);

    return header_size + in_file_offset;
}

inline bool par_write_comp_file(const string &compressed_filename, const CompressionParams &cpar,
                                const vector<CompBlockInfo> &blocks) {
    vector<size_t> write_offsets(blocks.size());
    vector<size_t> header;
    size_t compressed_file_size = calculate_compressed_dim(blocks, header, write_offsets);
    unsigned char *compressed_mm_file = nullptr;
    // allocate the space in a file for the uncompressed data. The file is memory mapped.
    if (!allocateFile(compressed_filename.c_str(), compressed_file_size, compressed_mm_file, cpar)) {
        log_msg(ERROR, cpar, "I/O error writing file %s: %s\n", compressed_filename.c_str(), " cannot map file");
        return false;
    }
    write_header(compressed_mm_file, header);
#pragma omp taskloop grainsize(1) shared(blocks, write_offsets, compressed_mm_file) if (blocks.size() > 1)
    for (size_t i = 0; i < blocks.size(); ++i) {
        unsigned char *start_block = compressed_mm_file + write_offsets[i];
        memcpy(start_block, blocks[i].ptr, blocks[i].comp_block_size);
    }
    unmapFile(compressed_mm_file, compressed_file_size, cpar);
    return true;
}

inline bool seq_write_comp_file(const string &compressed_filename, const CompressionParams &cpar,
                                const vector<CompBlockInfo> &blocks) {
    try {
        // printf("Thread %d writes compressed blocks of file %s\n", omp_get_thread_num(), filename.c_str());
        std::ofstream outFile;
        outFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        outFile.open(compressed_filename, std::ios::binary);
        // Write header
        seq_write_header(blocks, outFile);
        // Write blocks content
        for (const auto &blk: blocks) {
            outFile.write(reinterpret_cast<const char *>(blk.ptr), blk.comp_block_size);
        }
        outFile.close();
    } catch (const std::ios_base::failure &e) {
        log_msg(ERROR, cpar, "I/O error writing file %s: %s\n", compressed_filename.c_str(), e.what());
        filesystem::remove(compressed_filename);
        return false;
    }
    return true;
}

static bool block_compress(const string &filename, const CompressionParams &cpar) {
    size_t filesize = 0;
    //pointer that will point to the original file mapped in memory
    unsigned char *original_mm_file = nullptr;
    if (!mapFile(filename.c_str(), filesize, original_mm_file, cpar)) {
        log_msg(ERROR, cpar, "mapFile %s failed\n", filename.c_str());
        return false;
    }

    vector<CompBlockInfo> blocks;
    size_t block_size = compute_block_size(filesize);
    size_t num_blocks = (filesize + block_size - 1) / block_size;
    blocks.resize(num_blocks);
    bool any_error = false;

#pragma omp taskloop grainsize(1) shared(blocks, original_mm_file) reduction(|:any_error) if (num_blocks > 1)
    for (size_t i = 0; i < filesize; i += block_size) {
        size_t eff_block_size = min(block_size, filesize - i);
        unsigned char *prt_in = original_mm_file + i;
        mz_ulong cmp_len = compressBound(eff_block_size);

        auto *ptr_out = new (nothrow) unsigned char[cmp_len];
        if (!ptr_out) {
            log_msg(ERROR, cpar, "Memory allocation failed for block %lu\n", i);
            any_error = true;
            continue;
        }
        if (compress(ptr_out, &cmp_len, prt_in, eff_block_size) != Z_OK) {
            log_msg(ERROR, cpar, "Error compressing block %lu of file %s\n", i, filename.c_str());
            delete[] ptr_out;
            any_error = true;
        } else {
            size_t block_index = i / block_size;
            blocks[block_index] = CompBlockInfo{cmp_len, eff_block_size, ptr_out};
        }
    }
    if (any_error) {
        cleanup_resources(cpar, filesize, original_mm_file, blocks);
        return false;
    }
    string compressed_filename = filename + COMP_FILE_SUFFIX;
    bool compression_res = cpar.parallel_write
                               ? par_write_comp_file(compressed_filename, cpar, blocks)
                               : seq_write_comp_file(compressed_filename, cpar, blocks);
    cleanup_resources(cpar, filesize, original_mm_file, blocks);
    return compression_res;
}

#endif // PAR_BLOCK_COMPRESSOR_H
