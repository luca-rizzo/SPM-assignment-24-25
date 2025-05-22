#ifndef CMDLINE_PARAM_PARSER_H
#define CMDLINE_PARAM_PARSER_H
#include <string>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <getopt.h>

using namespace std;

struct RunningParam {
    size_t array_size;
    size_t record_payload_size;
    //0 means ceil(size/ff_num_threads)
    size_t base_case_size;
    unsigned int ff_num_threads;
};

inline void usage(const char *argv0) {
}

// check if the string 's' is a number, otherwise it returns false
inline bool isNumber(const char *s, long &n) {
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

static RunningParam parseCommandLine(int argc, char *argv[]) {
    RunningParam params(10240, 1024, 0, 8);
    const std::string optstr = "r:s:t:b:";
    long opt;
    while ((opt = getopt(argc, argv, optstr.c_str())) != -1) {
        switch (opt) {
            case 'r': {
                long r_val = 0;
                if (!isNumber(optarg, r_val)) {
                    std::fprintf(stderr, "Error: wrong '-r' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                params.record_payload_size = r_val;
            }
            break;
            case 'b': {
                long b_val = 0;
                if (!isNumber(optarg, b_val)) {
                    std::fprintf(stderr, "Error: wrong '-b' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                params.base_case_size = b_val;
            }
            break;
            case 's': {
                long s_val = 0;
                if (!isNumber(optarg, s_val)) {
                    std::fprintf(stderr, "Error: wrong '-s' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                params.array_size = s_val;
            }
            break;
            case 't': {
                long t_val = 0;
                if (!isNumber(optarg, t_val)) {
                    std::fprintf(stderr, "Error: wrong '-t' option\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                params.ff_num_threads = t_val;
            }
            break;
        }
    }
    return params;
}

static inline void debug_params(const RunningParam &run_par) {
    printf("RunningParams:\n");
    printf("  array size:\t %lu\n", run_par.array_size);
    printf("  record payload:\t %lu\n", run_par.record_payload_size);
    printf("  number of FastFlow threads:\t %d\n", run_par.ff_num_threads);
}

#endif //CMDLINE_PARAM_PARSER_H
