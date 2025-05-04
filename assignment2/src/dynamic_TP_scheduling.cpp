#include <threadPool.hpp>
#include <future>
#include <string>
#include <vector>
#include "collatz_fun.hpp"

using namespace std;


void execute_dynamic_TP_scheduling(int task_size, ThreadPool &tp,
                                   const pair<long, long> &range) {
    auto calculate_task_maximum = [](const pair<long, long> &task_range) {
        long local_maximum = 0;
        //process the task (range) passed as input and return maximum within a task
        for (long i = task_range.first; i <= task_range.second; ++i) {
            long collatz_length = calculate_collatz_length(i);
            local_maximum = max(local_maximum, collatz_length);
        }
        return local_maximum;
    };

    //divide the full range in sub-range of task_size elem and submit task to threadPool,
    //saving the futures returned
    vector<future<long> > local_maximum_futures;
    for (long start = range.first; start <= range.second; start += task_size) {
        long end_task_index = min(start + task_size - 1, range.second);
        local_maximum_futures.emplace_back(
            tp.enqueue(calculate_task_maximum, std::make_pair(start, end_task_index))
        );
    }

    long global_maximum = reduce_to_global_maximum(local_maximum_futures);
    fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second, global_maximum);
}
