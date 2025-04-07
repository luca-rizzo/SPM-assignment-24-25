#ifndef BLOCK_CYCLIC_SCHEDULING_H
#define BLOCK_CYCLIC_SCHEDULING_H
#include <utility>

void execute_static_scheduling(int task_size, int num_threads,
                               const std::pair<long, long> &range);
#endif //BLOCK_CYCLIC_SCHEDULING_H
