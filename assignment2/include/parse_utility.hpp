
#ifndef UTILITY_H
#define UTILITY_H
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <regex>

using namespace std;

enum SchedulingPolicy {
    STATIC_BLOCK_CYCLING,
    DYNAMIC_THREAD_POOL,
    DYNAMIC_WITH_INDEX
};

struct RunningParam {
    int num_threads;
    int task_size;
    SchedulingPolicy scheduling_policy;
    vector<pair<long, long> > ranges;
};

pair<long, long> parseRange(const string &rangeStr);

RunningParam parse_running_param(int argc, char *argv[]);

#endif //UTILITY_H
