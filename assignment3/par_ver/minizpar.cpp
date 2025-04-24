#include <atomic>
#include <fcntl.h>
#include <string>
#include <vector>
#include <filesystem>
#include "hpc_helpers.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "cmdline_param_parser.hpp"
#include "miniz.h"
#include "par_block_compressor.hpp"
#include "par_block_decompressor.hpp"

namespace fs = std::filesystem;
using namespace std;

void execute_op(const string &file_name, const CompressionParams &cpar) {
    bool success = cpar.op_to_perform == COMPRESS ?
        block_compress(file_name, cpar) :
        block_decompress(file_name, cpar);
    if (success && cpar.remove_origin) {
        unlink(file_name.c_str());
    }
}

void traverse_and_apply_op(const string &dir_to_traverse, const CompressionParams &cpar) {
    std::vector<fs::directory_entry> snapshot;
    for (const auto &entry: fs::directory_iterator(dir_to_traverse)) {
        snapshot.push_back(entry);
    }
    for (const auto &entry: snapshot) {
        auto file_name = entry.path().generic_string();
        if (entry.is_directory() && cpar.scan_subdirectories) {
#pragma omp task
            {
                traverse_and_apply_op(file_name, cpar);
            }
        } else if (entry.is_regular_file()) {
#pragma omp task
            {
                execute_op(file_name, cpar);
            }
        }
    }
}

void do_work(const string &file_name, const CompressionParams &cpar) {
    if (fs::is_directory(file_name)) {
        traverse_and_apply_op(file_name, cpar);
    } else {
#pragma omp task
        {
            execute_op(file_name, cpar);
        }
    }
}

int main(int argc, char *argv[]) {
    CompressionParams cpar = parseCommandLine(argc, argv);
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
#pragma omp taskwait
    }
    TIMERSTOP(minizpar)
}
