#ifndef CMDLINE_PARAM_PARSER_H
#define CMDLINE_PARAM_PARSER_H
#include <string>
#include <vector>
#include <atomic>
#include <fcntl.h>
#include <getopt.h>
#include <filesystem>
#include "miniz.h"

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

static inline void usage(const char *argv0) {
    CompressionParams params;
    std::printf("--------------------\n");
    std::printf("Usage: %s [options] file-or-directory [file-or-directory]\n", argv0);
    std::printf("\nOptions:\n");
    std::printf(" -r 0 does not recur, 1 will process the content of all subdirectories (default r=%d)\n",
                params.scan_subdirectories);
    std::printf(" -C compress: 0 preserves, 1 removes the original file (default C=%d)\n",
                params.remove_origin);
    std::printf(" -D decompress: 0 preserves, 1 removes the original file (default D=%d)\n",
                params.remove_origin);
    std::printf(" -q 0 silent mode, 1 prints only error messages to stderr, 2 verbose (default q=%d)\n",
                params.quite_mode);
    std::printf("--------------------\n");
}

// check if the string 's' is a number, otherwise it returns false
static bool isNumber(const char *s, long &n) {
    try {
        size_t e;
        n = std::stol(s, &e, 10);
        return e == strlen(s);
    } catch (const std::invalid_argument &) {
        return false;
    } catch (const std::out_of_range &) {
        return false;
    }
}

static inline CompressionParams parseCommandLine(int argc, char *argv[]) {
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

static inline void debug_params(const CompressionParams &cpar) {
    printf("CompressionParams:\n");
    printf("  Operation:            %s\n", (cpar.op_to_perform == COMPRESS) ? "Compress" : "Decompress");
    printf("  Remove origin:        %s\n", cpar.remove_origin ? "true" : "false");
    printf("  Verbosity level:      %d\n", cpar.quite_mode);
    printf("  Scan subdirectories:  %s\n", cpar.scan_subdirectories ? "true" : "false");
    printf("  Files:\n");
    for (const auto &f: cpar.files)
        printf("    - %s\n", f.c_str());
}

#endif //CMDLINE_PARAM_PARSER_H
