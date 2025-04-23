#include <future>
#include <string>
#include <vector>
#include "collatz_fun.hpp"
#include "threadPool.hpp"

using namespace std;

void execute_static_scheduling(int task_size, int num_threads,
const pair<long, long> &range) {
    auto block_cyclic = [&](int threadId) {
        const long offset = threadId * task_size + range.first;
        const long stride = num_threads * task_size;
        long local_maximum = 0;
        //each worker will process task_size elem every stride elem
        for (long i = offset; i <= range.second; i += stride) {
            long last_index = std::min(i + task_size - 1, range.second);
            //process task (composed of at most task_size elem)
            for (long j = i; j <= last_index; j++) {
                local_maximum = std::max(local_maximum, calculate_collatz_length(j));
            }
        }
        return local_maximum;
    };
    //create threads and task
    vector<future<long> > local_maximum_futures;
    vector<thread> threads;
    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        std::packaged_task<long(int)> thread_task(block_cyclic);
        //save futures in vector to retrieve the local maximum later
        local_maximum_futures.emplace_back(thread_task.get_future());
        threads.emplace_back(std::move(thread_task), thread_id);
    }

    long global_maximum = reduce_to_global_maximum(local_maximum_futures);

    for (auto &thread: threads) {
        thread.join();
    }
    fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second, global_maximum);
}