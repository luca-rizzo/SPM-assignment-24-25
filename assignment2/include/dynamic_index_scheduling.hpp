#ifndef DYNAMIC_INDEX_SCHEDULING_HPP
#define DYNAMIC_INDEX_SCHEDULING_HPP
#include <mutex>
#include <utility>

class ChunkDispatcher {
    std::pair<long, long> range;
    long task_size;
    long current_index;
    std::mutex _mutex;

    public:
    ChunkDispatcher(std::pair<long, long> range, long task_size);
    std::pair<long, long> next_chunk();
};

void execute_dynamic_index_scheduling(int task_size, int num_threads,
                                      const std::pair<long, long> &range);
#endif
