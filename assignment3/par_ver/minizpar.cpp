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

bool execute_op(const string &filename, const CompressionParams &cpar) {
    if (cpar.op_to_perform == DECOMPRESS && !filename.ends_with(".zip")) {
        log_msg(VERBOSE, cpar, "%s is not a zip: the file will not be decompressed\n", filename.c_str());
        return true;
    }
    bool local_success = cpar.op_to_perform == COMPRESS
                             ? block_compress(filename, cpar)
                             : block_decompress(filename, cpar);
    if (local_success && cpar.remove_origin) {
        unlink(filename.c_str());
    }
    return local_success;
}

//recursive function to collect all file in a dir and (based on flag) also subdir
vector<string> recurse_and_collect(const string &dir_to_traverse, bool scan_subdirectories) {
    vector<string> sub_files;
    for (const auto &entry: fs::directory_iterator(dir_to_traverse)) {
        if (entry.is_regular_file()) {
            sub_files.push_back(entry.path().generic_string());
        } else if (entry.is_directory() && scan_subdirectories) {
            vector<string> deeper_files = recurse_and_collect(entry.path().generic_string(), scan_subdirectories);
            sub_files.insert(sub_files.end(), deeper_files.begin(), deeper_files.end());
        }
    }
    return sub_files;
}

vector<string> collect_all_files(const CompressionParams &cpar) {
    vector<string> accumulator;
    for (const std::string &file: cpar.files) {
        if (fs::is_regular_file(file)) {
            accumulator.push_back(file);
        } else if (fs::is_directory(file)) {
            vector<string> deeper_files = recurse_and_collect(file, cpar.scan_subdirectories);
            accumulator.insert(accumulator.end(), deeper_files.begin(), deeper_files.end());
        }
    }
    return accumulator;
}

int main(int argc, char *argv[]) {
    CompressionParams cpar = parseCommandLine(argc, argv);
    TIMERSTART(minizpar)
    //as first thing collect all file to compress
    vector<string> all_files = collect_all_files(cpar);
    bool global_success = true;
#pragma omp parallel
    {
#pragma omp single
        {
            //create a task for each file
#pragma omp taskloop grainsize(1) shared(all_files) reduction(&:global_success)
            for (const std::string &file_name: all_files) {
                global_success = execute_op(file_name, cpar);
            }
        }
    }
    TIMERSTOP(minizpar)
    if (!global_success) {
        printf("Exiting with Failure\n");
        return -1;
    }
    printf("Exiting with Success\n");
    return 0;
}
