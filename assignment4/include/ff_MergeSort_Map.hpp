#ifndef FF_MERGESORT_FARM_H
#define FF_MERGESORT_FARM_H

#include <vector>
#include <memory>
#include <cmath>
#include <deque>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <iostream>
#include <algorithm>

using namespace ff;
using namespace std;

enum class TaskType { SORT, MERGE };

struct Task {
    TaskType type;
    size_t start;
    size_t end;
    size_t middle; // valido solo per MERGE
};

template<typename T>
struct Worker : ff_minode_t<Task> {
    Worker(T* data, size_t len) : data(data), len(len) {}

    Task* svc(Task* in) override {
        if (in->type == TaskType::SORT) {
            std::sort(data + in->start, data + in->end + 1);
        } else { // MERGE
            if (data[in->middle] <= data[in->middle + 1])
                return in;
            std::inplace_merge(data + in->start,
                               data + in->middle + 1,
                               data + in->end + 1);
        }
        return in;
    }

    T* data;
    size_t len;
};

template<typename T>
struct Master : ff_monode_t<Task> {
    Master(T* data, size_t len, int _par_degree, size_t _base_case_size)
        : data(data),
          len(len),
          num_workers(_par_degree - 1),
          par_degree(_par_degree)
    {
        base_case_size = (_base_case_size == 0) ? ceil(static_cast<double>(len) / par_degree) : _base_case_size;
        size_t leaf_tasks = (len + base_case_size - 1) / base_case_size;
        reusable_tasks.resize(leaf_tasks * 2);
    }

    Task* create_and_send_merge_task(int idx) {
        Task* l = current_level_merge.front(); current_level_merge.pop_front();
        Task* r = current_level_merge.front(); current_level_merge.pop_front();

        Task& t = reusable_tasks[idx];
        t.type = TaskType::MERGE;
        t.start = l->start;
        t.middle = l->end;
        t.end = r->end;

        ff_send_out_to(&t, idx % num_workers);
        return &t;
    }

    void terminate_useless_worker() {
        while (num_workers > level_merges_expected) {
            --num_workers;
            ff_send_out_to(EOS, num_workers);
        }
    }

    Task* svc(Task* in) override {
        if (in == nullptr) {
            size_t i = 0;
            for (; i + base_case_size < len; i += base_case_size) {
                size_t end = i + base_case_size - 1;
                Task& t = reusable_tasks[level_merges_expected];
                t.type = TaskType::SORT;
                t.start = i;
                t.end = end;
                ff_send_out(&t);
                current_level_merge.push_back(&t);
                level_merges_expected++;
            }

            Task& emitter_base_case = reusable_tasks[level_merges_expected];
            emitter_base_case.type = TaskType::SORT;
            emitter_base_case.start = i;
            emitter_base_case.end = len - 1;
            std::sort(data + emitter_base_case.start, data + emitter_base_case.end + 1);
            current_level_merge.push_back(&emitter_base_case);
            return GO_ON;
        }

        level_merges_completed++;
        if (level_merges_completed == level_merges_expected) {
            if (current_level_merge.size() == 2) {
                Task* l = current_level_merge.front(); current_level_merge.pop_front();
                Task* r = current_level_merge.front(); current_level_merge.pop_front();
                std::inplace_merge(data + l->start,
                                   data + l->end + 1,
                                   data + r->end + 1);
                return EOS;
            }

            deque<Task*> nextLevel;
            level_merges_expected = 0;

            while (current_level_merge.size() > 1) {
                Task* task = create_and_send_merge_task(level_merges_expected);
                nextLevel.push_back(task);
                level_merges_expected++;
            }

            terminate_useless_worker();

            if (!current_level_merge.empty())
                nextLevel.push_back(current_level_merge.front());

            current_level_merge = std::move(nextLevel);
            level_merges_completed = 0;
        }

        return GO_ON;
    }

    T* data;
    size_t len;
    int num_workers;
    int par_degree;
    size_t base_case_size;
    int level_merges_expected = 0;
    int level_merges_completed = 0;
    vector<Task> reusable_tasks;
    deque<Task*> current_level_merge;
};

template<typename T>
class ff_MergeSort_Map : public ff_Farm<void> {
public:
    ff_MergeSort_Map(T* data, size_t len, int par_degree, int base_case_size = 0)
        : ff_Farm(
              [&]() {
                  vector<unique_ptr<ff_node>> W;
                  for (int i = 0; i < par_degree - 1; ++i)
                      W.push_back(make_unique<Worker<T>>(data, len));
                  return W;
              }(),
              make_unique<Master<T>>(data, len, par_degree, base_case_size))
    {
        this->remove_collector();
        this->wrap_around();
    }
};

#endif // FF_MERGESORT_FARM_H
