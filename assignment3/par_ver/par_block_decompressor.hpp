#ifndef PAR_BLOCK_DECOMPRESSOR_H
#define PAR_BLOCK_DECOMPRESSOR_H

#include <atomic>
#include <fcntl.h>
#include <string>
#include <vector>
#include <filesystem>
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

inline vector<DecompBlockHeaderInfo> read_header(unsigned char *ptr, size_t& original_filesize) {
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
    original_filesize = block_size * (num_blocks - 1) + last_block_size;
    return blocks;
}


static bool block_decompress(const string &filename, const CompressionParams &cpar) {
    if (!filename.ends_with(COMP_FILE_SUFFIX)) {
        log_msg(VERBOSE, cpar, "%s is not a zip: the file will not be decompressed\n", filename.c_str());
        return false;
    }
    size_t filesize = 0;
    unsigned char *compressed_mm_file = nullptr;
    if (!mapFile(filename.c_str(), filesize, compressed_mm_file, cpar)) {
        log_msg(ERROR, cpar, "mapFile %s failed\n", filename.c_str());
        return false;
    }
    size_t total_decompressed_size = 0;
    vector<DecompBlockHeaderInfo> block_header = read_header(compressed_mm_file, total_decompressed_size);
    size_t header_size = sizeof(size_t) * (3 + block_header.size());
    unsigned char *data_ptr = compressed_mm_file + header_size;
    std::string outfile = filename.substr(0, filename.size() - COMP_FILE_SUFFIX.size());
    unsigned char *decompressed_mm_file = nullptr;
    if (!allocateFile(outfile.c_str(), total_decompressed_size, decompressed_mm_file, cpar)) {
        log_msg(ERROR, cpar, "I/O error writing file %s: %s\n", filename.c_str(), " cannot map file");
        unmapFile(compressed_mm_file, filesize, cpar);
        return false;
    }
    bool any_error = false;
#pragma omp taskloop grainsize(1) shared(block_header, data_ptr, decompressed_mm_file) reduction(|:any_error) if (block_header.size() > 1)
    for (size_t i = 0; i < block_header.size(); i++) {
        const auto &blk = block_header[i];
        unsigned char *in_ptr = data_ptr + blk.offset;
        //Calculate output offset: all blocks except the last one have the same size
        size_t offset = i * block_header[0].orig_block_size;
        unsigned char *write_ptr = decompressed_mm_file + offset;
        mz_ulong orig_len = blk.orig_block_size;
        if (uncompress(write_ptr, &orig_len, in_ptr, blk.comp_block_size) != Z_OK) {
            log_msg(ERROR, cpar, "uncompress failed for block %lu of %s\n", i, filename.c_str());
            any_error = true;
        }
    }
    unmapFile(compressed_mm_file, filesize, cpar);
    unmapFile(decompressed_mm_file, total_decompressed_size, cpar);
    if (any_error)
        std::filesystem::remove(outfile);

    return !any_error;
}

#endif //PAR_BLOCK_DECOMPRESSOR_H
