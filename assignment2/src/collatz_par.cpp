#include <cstdio>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include "block_cyclic_scheduling.hpp"
#include "dynamic_TP_scheduling.hpp"
#include "dynamic_index_scheduling.hpp"
#include "hpc_helpers.hpp"
#include "threadPool.hpp"
#include "parse_utility.hpp"

using namespace std;

void debug_run_parsed_param(const RunningParam &running_param) {
    printf("RunningParam: %d %d %d \n", running_param.num_threads, running_param.task_size,
           running_param.scheduling_policy);
    for (const auto &range: running_param.ranges) {
        printf("Range: %lu, %lu\n", range.first, range.second);
    }
}


int main(int argc, char **argv) {
    RunningParam running_param = parse_running_param(argc, argv);
    //debug_run_parsed_param(runningParam);
    TIMERSTART(collatz_par);
    switch (running_param.scheduling_policy) {
        case DYNAMIC_THREAD_POOL: {
            ThreadPool tp(running_param.num_threads);
            for (const auto &range: running_param.ranges) {
                execute_dynamic_TP_scheduling(running_param.task_size, tp, range);
            }
        }
        break;
        case DYNAMIC_WITH_INDEX:
            for (const auto &range: running_param.ranges) {
                execute_dynamic_index_scheduling(running_param.task_size,
                                                 running_param.num_threads, range);
            }
            break;
        case STATIC_BLOCK_CYCLING:
            for (const auto &range: running_param.ranges) {
                execute_static_scheduling(running_param.task_size,
                                          running_param.num_threads, range);
            }
            break;
        default:
            printf("UNKNOWN\n");
    }
    TIMERSTOP(collatz_par);
}
