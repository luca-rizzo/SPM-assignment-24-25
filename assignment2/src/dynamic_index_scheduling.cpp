#include "dynamic_index_scheduling.hpp"
#include "collatz_fun.hpp"
#include <utility>
#include <future>
#include <string>
#include <vector>
#include "threadPool.hpp"

using namespace std;

ChunkDispatcher::ChunkDispatcher(std::pair<long, long> range, long task_size)
    : range(range), task_size(task_size), current_index(range.first) {}

std::pair<long, long> ChunkDispatcher::next_chunk() {
    std::unique_lock<std::mutex> lock(_mutex);
    long start_index_chunk = current_index;
    long end_index_chunk = std::min(current_index + task_size - 1, range.second);
    current_index = end_index_chunk + 1;
    return {start_index_chunk, end_index_chunk};
}

void execute_dynamic_index_scheduling(int task_size, int num_threads,
                                      const pair<long, long> &range) {
    ChunkDispatcher chunkDispatcher(range, task_size);
    auto dynamic_index = [&]() {
        long local_max = 0;
        pair<long, long> currentChunk;
        do {
            //extract a chunk from the shared concurrent structure
            currentChunk = chunkDispatcher.next_chunk();
            //process task (composed of at most task_size elem)
            for (long i = currentChunk.first; i <= currentChunk.second; i += 1) {
                long collatz_length = calculate_collatz_length(i);
                local_max = max(local_max, collatz_length);
            }
        } while (currentChunk.first <= currentChunk.second);
        return local_max;
    };
    //create threads and task
    vector<future<long> > local_maximum_futures;
    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        std::packaged_task<long()> thread_task(dynamic_index);
        //save futures in vector to retrieve the local maximum later
        local_maximum_futures.emplace_back(thread_task.get_future());
        threads.emplace_back(std::move(thread_task));
    }

    long global_maximum = reduce_to_global_maximum(local_maximum_futures);

    for (auto &thread: threads) {
        thread.join();
    }
    fprintf(stderr, "%ld-%ld: %ld\n", range.first, range.second, global_maximum);
}
