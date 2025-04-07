#ifndef DYNAMIC_TP_SCHEDULING_H
#define DYNAMIC_TP_SCHEDULING_H
#include <threadPool.hpp>

void execute_dynamic_TP_scheduling(int task_size, ThreadPool &tp,
                                   const std::pair<long, long> &range);
#endif //DYNAMIC_TP_SCHEDULING_H
