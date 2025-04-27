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

void execute_op(const string &filename, const CompressionParams &cpar, atomic_bool &global_success) {
    if (cpar.op_to_perform == DECOMPRESS && !filename.ends_with(".zip")) {
        log_msg(VERBOSE, cpar, "%s is not a zip: the file will not be decompressed\n", filename.c_str());
        return;
    }
    bool local_success = cpar.op_to_perform == COMPRESS
                             ? block_compress(filename, cpar)
                             : block_decompress(filename, cpar);
    if (!local_success) {
        global_success.store(false);
        return;
    }
    if (cpar.remove_origin) {
        unlink(filename.c_str());
    }
}

void traverse_and_apply_op(const string &dir_to_traverse, const CompressionParams &cpar, atomic_bool &global_success) {
    std::vector<fs::directory_entry> snapshot;
    for (const auto &entry: fs::directory_iterator(dir_to_traverse)) {
        snapshot.push_back(entry);
    }
    for (const auto &entry: snapshot) {
        auto file_name = entry.path().generic_string();
        if (entry.is_directory() && cpar.scan_subdirectories) {
#pragma omp task shared(global_success)
            {
                traverse_and_apply_op(file_name, cpar, global_success);
            }
        } else if (entry.is_regular_file()) {
#pragma omp task shared(global_success)
            {
                execute_op(file_name, cpar, global_success);
            }
        }
    }
}

void do_work(const string &file_name, const CompressionParams &cpar, atomic_bool &global_success) {
    if (fs::is_directory(file_name)) {
        traverse_and_apply_op(file_name, cpar, global_success);
    } else {
#pragma omp task shared(global_success)
        {
            execute_op(file_name, cpar, global_success);
        }
    }
}

int main(int argc, char *argv[]) {
    CompressionParams cpar = parseCommandLine(argc, argv);
    atomic_bool global_success(true);
    TIMERSTART(minizpar)
#pragma omp parallel
    {
#pragma omp single
        {
            for (const std::string &file: cpar.files) {
                {
                    do_work(file, cpar, global_success);
                }
            }
        }
#pragma omp taskwait
    }
    TIMERSTOP(minizpar)
    if (!global_success.load()) {
        printf("Exiting with Failure\n");
        return -1;
    }
    printf("Exiting with Success\n");
    return 0;
}
