#include "parse_utility.hpp"
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <regex>

using namespace std;

pair<long, long> parseRange(const string &rangeStr) {
    regex pattern(R"(^\s*(\d+)\s*-\s*(\d+)\s*$)");
    smatch match;

    if (!regex_match(rangeStr, match, pattern)) {
        cerr << "Not valid range format: " << rangeStr << endl;
        exit(EXIT_FAILURE);
    }
    long start = stol(match[1].str());
    long end = stol(match[2].str());
    return {start, end};
}

int parse_int(const char *arg, const string &option) {
    try {
        return stol(arg);
    } catch (const invalid_argument &e) {
        cerr << "Invalid argument for " << option << ": must be an integer." << endl;
        exit(EXIT_FAILURE);
    } catch (const out_of_range &e) {
        cerr << "Out of range value for " << option << ": too big for long." << endl;
        exit(EXIT_FAILURE);
    }
}

RunningParam parse_running_param(int argc, char *argv[]) {
    int opt;
    RunningParam runningParam{16, 1, STATIC_BLOCK_CYCLING};
    while ((opt = getopt(argc, argv, "n:c:dst")) != EOF) {
        switch (opt) {
            case 'n':
                runningParam.num_threads = parse_int(optarg, "-n");
            break;
            case 'c':
                runningParam.task_size = parse_int(optarg, "-c");
            break;
            case 'd':
                runningParam.scheduling_policy = DYNAMIC_WITH_INDEX;
            break;
            case 't':
                runningParam.scheduling_policy = DYNAMIC_THREAD_POOL;
            break;
            case 's':
                runningParam.scheduling_policy = STATIC_BLOCK_CYCLING;
            break;
            default:
                cerr << "Unknown option " << opt << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (int i = optind; i < argc; ++i) {
        runningParam.ranges.emplace_back(parseRange(argv[i]));
    }

    return runningParam;
}
