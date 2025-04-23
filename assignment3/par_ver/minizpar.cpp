#include <atomic>
#include <getopt.h>
#include <string>
#include <utility.hpp>
#include <vector>
#include <filesystem>
#include "hpc_helpers.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

namespace fs = std::filesystem;
using namespace std;

enum OpToPerform {
    COMPRESS, DECOMPRESS
};

typedef struct CompressionParams {
    OpToPerform op_to_perform = COMPRESS; // by default, it compresses
    bool remove_origin = false; // Does it keep the origin file?
    int quite_mode = 1; // 0 silent, 1 error messages, 2 verbose
    bool scan_subdirectories = false;
    std::vector<std::string> files;
} CompressionParams;

void usage(const char *argv0) {
    CompressionParams params;
    std::printf("--------------------\n");
    std::printf("Usage: %s [options] file-or-directory [file-or-directory]\n", argv0);
    std::printf("\nOptions:\n");
    std::printf(" -r 0 does not recur, 1 will process the content of all subdirectories (default r=%d)\n",
                params.scan_subdirectories ? 1 : 0);
    std::printf(" -C compress: 0 preserves, 1 removes the original file (default C=%d)\n",
                params.remove_origin & params.op_to_perform == COMPRESS ? 1 : 0);
    std::printf(" -D decompress: 0 preserves, 1 removes the original file (default D=%d)\n",
                params.remove_origin & params.op_to_perform == DECOMPRESS ? 1 : 0);
    std::printf(" -q 0 silent mode, 1 prints only error messages to stderr, 2 verbose (default q=%d)\n",
                params.quite_mode);
    std::printf("--------------------\n");
}


constexpr size_t compute_block_size(size_t filesize) {
    if (filesize < 1 * 1024 * 1024)      // < 1MB
        return 16 * 1024;                // 16KB
    else if (filesize < 10 * 1024 * 1024) // < 10MB
        return 64 * 1024;                // 64KB
    else if (filesize < 100 * 1024 * 1024) // < 100MB
        return 256 * 1024;               // 256KB
    else
        return 1 * 1024 * 1024;          // 1MB
}


CompressionParams parseCommandLine(int argc, char *argv[]) {
    CompressionParams params;
    extern char *optarg;
    const std::string optstr = "r:C:D:q:";
    long opt;
    bool cpresent = false, dpresent = false;

    while ((opt = getopt(argc, argv, optstr.c_str())) != -1) {
        switch (opt) {
            case 'r': {
                long n = 0;
                if (!isNumber(optarg, n)) {
                    std::fprintf(stderr, "Error: wrong '-r' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                params.scan_subdirectories = (n == 1);
            }
            break;
            case 'C': {
                long c = 0;
                if (!isNumber(optarg, c)) {
                    std::fprintf(stderr, "Error: wrong '-C' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                cpresent = true;
                params.remove_origin = (c == 1);
                params.op_to_perform = COMPRESS;
            }
            break;
            case 'D': {
                long d = 0;
                if (!isNumber(optarg, d)) {
                    std::fprintf(stderr, "Error: wrong '-D' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                dpresent = true;
                params.remove_origin = (d == 1);
                params.op_to_perform = DECOMPRESS;
            }
            break;
            case 'q': {
                long q = 0;
                if (!isNumber(optarg, q)) {
                    std::fprintf(stderr, "Error: wrong '-q' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                params.quite_mode = (q == 1);
            }
            break;
            default:
                usage(argv[0]);
                exit(EXIT_SUCCESS);
        }
    }

    // Ensure -C and -D are not used together
    if (cpresent && dpresent) {
        std::fprintf(stderr, "Error: -C and -D are mutually exclusive!\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    if ((argc - optind) <= 0) {
        std::fprintf(stderr, "Error: at least one file or directory should be provided!\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; ++i) {
        params.files.emplace_back(argv[i]);
    }

    return params;
}

void debug_params(const CompressionParams &cpar) {
    printf("CompressionParams:\n");
    printf("  Operation:            %s\n", (cpar.op_to_perform == COMPRESS) ? "Compress" : "Decompress");
    printf("  Remove origin:        %s\n", cpar.remove_origin ? "true" : "false");
    printf("  Verbosity level:      %d\n", cpar.quite_mode);
    printf("  Scan subdirectories:  %s\n", cpar.scan_subdirectories ? "true" : "false");
    printf("  Files:\n");
    for (const auto &f: cpar.files)
        printf("    - %s\n", f.c_str());
}

typedef struct CompBlockInfo {
    size_t comp_block_size;
    size_t orig_block_size;
    unsigned char* ptr;
} CompBlockInfo;

typedef struct DecompBlockInfo {
    size_t block_size;
    unsigned char* ptr;
} DecompBlockInfo;

typedef struct DecompBlockHeaderInfo {
    size_t orig_block_size;
    size_t comp_block_size;
    size_t offset;
} DecompBlockHeaderInfo;

void write_header(const vector<CompBlockInfo>& blocks, std::ofstream& outFile) {
    uint32_t num_blocks = blocks.size();
    outFile.write(reinterpret_cast<const char*>(&num_blocks), sizeof(uint32_t));
    uint32_t offset = 0;
    for (const auto& blk : blocks) {
        uint32_t comp_size = blk.comp_block_size;
        uint32_t orig_size = blk.orig_block_size;
        outFile.write(reinterpret_cast<const char*>(&comp_size), sizeof(uint32_t));
        outFile.write(reinterpret_cast<const char*>(&orig_size), sizeof(uint32_t));
        outFile.write(reinterpret_cast<const char*>(&offset), sizeof(uint32_t));
        offset += blk.comp_block_size;
    }
}

void block_compress(const string& filename) {
    size_t filesize = 0;
    unsigned char *ptr = nullptr;
    if (!mapFile(filename.c_str(), filesize, ptr)) {
        if (QUITE_MODE>=1)
            std::fprintf(stderr, "mapFile %s failed\n", filename.c_str());
        exit(EXIT_FAILURE);
    }
    vector<CompBlockInfo> blocks;
    size_t block_size = compute_block_size(filesize);
    size_t num_blocks = (filesize + block_size - 1) / block_size;
    blocks.resize(num_blocks);
    atomic<bool> has_error = false;
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < filesize; i += block_size) {
            //each thread compress a block and store the various reference to
            size_t eff_block_size = min(block_size, filesize - i);
            unsigned char* inPtr = ptr + i;
            // get an estimation of the maximum compression size
            mz_ulong cmp_len = compressBound(eff_block_size);
            // allocate memory to store compressed data in memory
            auto *ptrOut = new unsigned char[cmp_len];
            if (compress(ptrOut, &cmp_len, inPtr, eff_block_size) != Z_OK) {
                if (QUITE_MODE>=1)
                    std::fprintf(stderr, "Error compressing block %llu of file %s\n", i, filename.c_str());
                delete [] ptrOut;
                has_error = true;
            }
            else {
                size_t block_index = i / block_size;
                blocks[block_index] = CompBlockInfo{eff_block_size, cmp_len, ptrOut};
            }
        }
        #pragma omp single
        {
            if (has_error == true) {
                for (const auto& blk : blocks) {
                    delete[] blk.ptr;
                }
            }
            //write ordered to file
            std::ofstream outFile(filename + ".zip", std::ios::binary);
            if (!outFile.is_open()) {
                std::fprintf(stderr, "Cannot open output file!\n");
                for (const auto& blk : blocks) {
                    delete[] blk.ptr;
                }
            } else {
                //Write header
                write_header(blocks, outFile);
                //Write blocks content
                for (const auto& blk : blocks) {
                    outFile.write(reinterpret_cast<const char*>(blk.ptr), blk.comp_block_size);
                    delete[] blk.ptr;
                }

                outFile.close();
            }
            unmapFile(ptr, filesize);
        }
    }
}

vector<DecompBlockHeaderInfo> read_header(unsigned char *ptr) {
    vector<DecompBlockHeaderInfo> blocks;
    unsigned char* cur = ptr;
    uint32_t num_blocks = *reinterpret_cast<uint32_t*>(cur);
    cur += sizeof(uint32_t);
    blocks.resize(num_blocks);
    for (uint32_t i = 0; i < num_blocks; ++i) {
        uint32_t comp_size = *reinterpret_cast<uint32_t*>(cur);
        cur += sizeof(uint32_t);
        uint32_t orig_size = *reinterpret_cast<uint32_t*>(cur);
        cur += sizeof(uint32_t);
        uint32_t offset = *reinterpret_cast<uint32_t*>(cur);
        cur += sizeof(uint32_t);
        blocks[i] = DecompBlockHeaderInfo{comp_size,
            orig_size, offset};
    }
    return blocks;
}

void block_decompress(const string& filename) {
    size_t filesize = 0;
    unsigned char *ptr = nullptr;
    if (!mapFile(filename.c_str(), filesize, ptr)) {
        if (QUITE_MODE>=1)
            std::fprintf(stderr, "mapFile %s failed\n", filename.c_str());
        exit(EXIT_FAILURE);
    }
    vector<DecompBlockHeaderInfo> block_header = read_header(ptr);
    vector<DecompBlockInfo> decompr_blocks;
    decompr_blocks.resize(block_header.size());
    size_t header_size = sizeof(uint32_t) + block_header.size() * (sizeof(uint32_t) * 3);
    unsigned char* data_ptr = ptr + header_size;
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < block_header.size(); i += 1) {
            DecompBlockHeaderInfo blk = block_header[i];
            //each thread decompress a block and store the various reference to
            auto *ptrOut = new unsigned char[blk.orig_block_size];
            unsigned char * inPtr = data_ptr + blk.offset;
            mz_ulong orig_len = blk.orig_block_size;
            int cmp_status = uncompress(ptrOut, &orig_len, inPtr, blk.comp_block_size);
            if (cmp_status != Z_OK) {
                if (QUITE_MODE>=1)
                    std::fprintf(stderr, "uncompress failed!\n");
            }
            else {
                size_t block_index = i;
                decompr_blocks[block_index] = DecompBlockInfo{orig_len, ptrOut};
            }
        }
        #pragma omp single
        {
            //write ordered to file
            std::string outfile = filename.substr(0, filename.size() - 4);  // remove the SUFFIX (i.e., .zip)
            std::ofstream outFile(outfile+"dec", std::ios::binary);
            if (!outFile.is_open()) {
                std::fprintf(stderr, "Cannot open output file!\n");
                for (const auto& blk : decompr_blocks) {
                    delete[] blk.ptr;
                }
            } else {
                for (const auto& blk : decompr_blocks) {
                    outFile.write(reinterpret_cast<const char*>(blk.ptr), blk.block_size);
                    delete[] blk.ptr;
                }
                outFile.close();
            }
            unmapFile(ptr, filesize);
        }
    }
}


void traverse_and_compress(const string& dir_to_traverse, const CompressionParams& cpar) {
    std::vector<fs::directory_entry> snapshot;
    for (const auto& entry : fs::directory_iterator(dir_to_traverse)) {
        snapshot.push_back(entry);  // crea una copia sicura
    }
    for (const auto& entry : snapshot) {
        if (entry.is_directory()) {
            #pragma omp task
            {
                traverse_and_compress(entry.path().generic_string(), cpar);
            }
        } else {
            #pragma omp task
            {
                block_compress(entry.path().generic_string());
            }
        }
    }
}

void do_work(const string& file_name, const CompressionParams& cpar) {
    if (fs::is_directory(file_name)) {
        #pragma omp task
        {
            traverse_and_compress(file_name, cpar);
        }
    } else {
        #pragma omp task
        {
            if (cpar.op_to_perform == COMPRESS) {
                block_compress(file_name);
            } else {
                block_decompress(file_name);
            }
        }
    }
}



int main(int argc, char *argv[]) {
    CompressionParams cpar = parseCommandLine(argc, argv);
    //debug_params(cpar);
    TIMERSTART(minizpar)
#pragma omp parallel
        {
#pragma omp single
            {
                for (const std::string &file: cpar.files) {
                    {
                        do_work(file, cpar);
                    }
                }
            }
        }
    TIMERSTOP(minizpar)
}
