#include <cstdio>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <regex>

#include "hpc_helpers.hpp"
#include "threadPool.hpp"

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

class ChunkDispatcher {
    pair<long, long> range;
    long task_size;
    long current_index;
    mutex _mutex;

public:
    ChunkDispatcher(pair<long, long> range, long task_size) {
        this->range = range;
        this->task_size = task_size;
        this->current_index = range.first;
    }

    // interval [) open on the right, close on the left
    pair<long, long> next_chunk() {
        unique_lock<mutex> lock(_mutex);
        int start_index_chunk = current_index;
        int end_index_chunk = min(current_index + task_size, range.second);
        current_index = end_index_chunk;
        return {start_index_chunk, end_index_chunk};
    }
};

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
        }
    }
    for (int i = optind; i < argc; ++i) {
        runningParam.ranges.emplace_back(parseRange(argv[i]));
    }

    return runningParam;
}

void debug_run_parsed_param(const RunningParam &running_param) {
    printf("RunningParam: %d %d %d \n", running_param.num_threads, running_param.task_size,
           running_param.scheduling_policy);
    for (const auto &range: running_param.ranges) {
        printf("Range: %lu, %lu\n", range.first, range.second);
    }
}

long reduce_to_global_maximum(vector<future<long> > &local_maximum_futures) {
    long global_maximum = 0;
    for (auto &future: local_maximum_futures) {
        global_maximum = max(global_maximum, future.get());
    }
    return global_maximum;
}

long calculate_collatz_length(long n) {
    if (n < 1) {
        return -1;
    }
    long collatz_length = 0;
    while (n != 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        collatz_length++;
    }

    return collatz_length;
}

void execute_static_scheduling(int task_size, int num_threads,
                               const pair<long, long> &range) {
    auto block_cyclic = [&](int threadId) {
        const long offset = threadId * task_size + range.first;
        const long stride = num_threads * task_size;
        long local_maximum = 0;
        //starting from offset the thread pick a chunk of task_size elem every stride elem
        for (long i = offset; i < range.second; i += stride) {
            long last_index = min(i + task_size, range.second);
            for (long j = i; j < last_index; j++) {
                local_maximum = max(local_maximum, calculate_collatz_length(j));
            }
        }
        return local_maximum;
    };
    vector<future<long> > local_maximum_futures;
    vector<thread> threads;
    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        std::packaged_task<long(int)> thread_task(block_cyclic);
        local_maximum_futures.emplace_back(thread_task.get_future());
        threads.emplace_back(std::move(thread_task), thread_id);
    }

    long global_maximum = reduce_to_global_maximum(local_maximum_futures);

    for (auto &thread: threads) {
        thread.join();
    }
    fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second, global_maximum);
}

void execute_dynamic_index_scheduling(int task_size, int num_threads,
                                      const pair<long, long> &range) {
    ChunkDispatcher chunkDispatcher(range, task_size);
    auto dynamic_index = [&]() {
        long local_max = 0;
        pair<long, long> currentChunk;
        do {
            currentChunk = chunkDispatcher.next_chunk();
            for (long i = currentChunk.first; i < currentChunk.second; i += 1) {
                long collatz_length = calculate_collatz_length(i);
                local_max = max(local_max, collatz_length);
            }
        } while (currentChunk.first != currentChunk.second);
        return local_max;
    };
    vector<future<long> > local_maximum_futures;
    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        std::packaged_task<long()> thread_task(dynamic_index);
        local_maximum_futures.emplace_back(thread_task.get_future());
        threads.emplace_back(std::move(thread_task));
    }

    long global_maximum = reduce_to_global_maximum(local_maximum_futures);

    for (auto &thread: threads) {
        thread.join();
    }
    fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second, global_maximum);
}


void execute_dynamic_TP_scheduling(int task_size, ThreadPool &tp,
                                   const pair<long, long> &range) {
    auto calculate_task_maximum = [](const pair<long, long> &task_range) {
        long local_maximum = 0;
        for (long i = task_range.first; i < task_range.second; i += 1) {
            long collatz_length = calculate_collatz_length(i);
            local_maximum = max(local_maximum, collatz_length);
        }
        return local_maximum;
    };

    vector<future<long> > local_maximum_futures;
    for (long start = range.first; start < range.second; start += task_size) {
        long end_task_index = min(start + task_size, range.second);
        local_maximum_futures.emplace_back(
            tp.enqueue([=, &calculate_task_maximum]() {
                return calculate_task_maximum({start, end_task_index});
            }));
    }

    long global_maximum = reduce_to_global_maximum(local_maximum_futures);
    fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second, global_maximum);
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
