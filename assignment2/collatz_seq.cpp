#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>  // per getopt
#include <vector>
#include <regex>

enum SchedulingPolicy {
    STATIC_BLOCK_CYCLING,
    DYNAMIC_THREAD_POOL,
    DYNAMIC_WITH_INDEX
};

struct RunningParam {
    int numThreads;
    int taskSize;
    SchedulingPolicy scheduling_policy;
    std::vector<std::pair<long, long> > ranges;
};

std::pair<long, long> parseRange(const std::string &rangeStr) {
    std::regex pattern(R"(^\s*(\d+)\s*-\s*(\d+)\s*$)");
    std::smatch match;

    if (!std::regex_match(rangeStr, match, pattern)) {
        std::cerr << "Formato di range non valido: " << rangeStr << std::endl;
        exit(EXIT_FAILURE);
    }
    long start = std::stol(match[1].str());
    long end = std::stol(match[2].str());
    return {start, end};
}

RunningParam parseRunningParam(int argc, char *argv[]) {
    int opt;
    RunningParam runningParam{16, 1, STATIC_BLOCK_CYCLING};
    while ((opt = getopt(argc, argv, "n:c:ds")) != EOF) {
        switch (opt) {
            case 'n':
                runningParam.numThreads = std::stol(optarg);
                break;
            case 'c':
                runningParam.taskSize = std::stol(optarg);
                break;
            case 'd':
                runningParam.scheduling_policy = DYNAMIC_THREAD_POOL;
                break;
            case 's':
                runningParam.scheduling_policy = STATIC_BLOCK_CYCLING;
                break;
        }
    }
    for (int i = optind; i < argc; ++i) {
        runningParam.ranges.emplace_back(parseRange(argv[i]));
    }

    return runningParam;
}

void debug_run_parsed_param(const RunningParam& runningParam) {
    printf("RunningParam: %d %d %d \n", runningParam.numThreads, runningParam.taskSize, runningParam.scheduling_policy);
    for (const auto& range : runningParam.ranges) {
        printf("Range: %lu, %lu\n", range.first, range.second);
    }
}

int main(int argc, char *argv[]) {
    RunningParam runningParam = parseRunningParam(argc, argv);
    debug_run_parsed_param(runningParam);
    switch (runningParam.scheduling_policy) {
        case DYNAMIC_THREAD_POOL:
            break;
        case DYNAMIC_WITH_INDEX:
            break;
        case STATIC_BLOCK_CYCLING:
            break;
        default:
            printf("UNKNOWN\n");
    }
}
