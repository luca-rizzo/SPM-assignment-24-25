#ifndef FF_MERGESORT_FARM_H
#define FF_MERGESORT_FARM_H

#include <vector>
#include <memory>
#include <cmath>
#include <deque>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <unistd.h>


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
    Worker(T *data, size_t len) : to_sort(data), to_sort_len(len) {
    }

    void print_thread_cpu_mapping() {
        ssize_t ff_id = get_my_id();
        int core_id = sched_getcpu();
        std::cerr << "[Worker] ff_id = " << ff_id
                << ", core = " << core_id
                << std::endl;
    }

    Task *svc(Task *in) override {
        //print_thread_cpu_mapping();
        if (in->type == TaskType::SORT) {
            std::sort(to_sort + in->start, to_sort + in->end + 1);
        } else {
            if (to_sort[in->middle] <= to_sort[in->middle + 1])
                return in;
            std::inplace_merge(to_sort + in->start,
                               to_sort + in->middle + 1,
                               to_sort + in->end + 1);
        }

        return in;
    }


    T *to_sort;
    size_t to_sort_len;
};

template<typename T>
struct Master : ff_monode_t<Task> {
    Master(T *data, const size_t len, int _par_degree, size_t _base_case_size)
        : to_sort(data),
          to_sort_len(len),
          num_workers(_par_degree - 1),
          par_degree(_par_degree) {
        base_case_size = (_base_case_size == 0)
                             ? (len + par_degree - 1) / par_degree
                             : _base_case_size;

        size_t leaf_tasks = (len + base_case_size - 1) / base_case_size;
        reusable_tasks.resize(leaf_tasks * 2);
    }

    Task *create_and_send_merge_task(int idx) {
        Task *l = current_level_merge.front();
        current_level_merge.pop_front();
        Task *r = current_level_merge.front();
        current_level_merge.pop_front();

        Task &t = reusable_tasks[idx];
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

    Task *svc(Task *in) override {
        if (in == nullptr) {
            size_t i = 0;
            int j = 0;
            for (; i + base_case_size < to_sort_len; i += base_case_size) {
                size_t end = i + base_case_size - 1;
                Task &t = reusable_tasks[level_merges_expected];
                t.type = TaskType::SORT;
                t.start = i;
                t.end = end;
                ff_send_out_to(&t, j++);
                current_level_merge.push_back(&t);
                level_merges_expected++;
            }

            Task &emitter_base_case = reusable_tasks[level_merges_expected];
            emitter_base_case.type = TaskType::SORT;
            emitter_base_case.start = i;
            emitter_base_case.end = to_sort_len - 1;
            std::sort(to_sort + emitter_base_case.start, to_sort + emitter_base_case.end + 1);
            current_level_merge.push_back(&emitter_base_case);
            return GO_ON;
        }

        level_merges_completed++;

        if (level_merges_completed == level_merges_expected) {
            level_merges_expected = 0;
            level_merges_completed = 0;

            // Final merge of last two sorted blocks
            if (current_level_merge.size() == 2) {
                terminate_useless_worker();
                Task *l = current_level_merge.front();
                current_level_merge.pop_front();
                Task *r = current_level_merge.front();
                current_level_merge.pop_front();
                std::inplace_merge(to_sort + l->start,
                                   to_sort + l->end + 1,
                                   to_sort + r->end + 1);
                return EOS;
            }

            // create and send merge task
            size_t to_merge = current_level_merge.size();
            for (size_t i = 0; i + 1 < to_merge; i += 2) {
                Task *merged = create_and_send_merge_task(level_merges_expected);
                current_level_merge.push_back(merged);
                level_merges_expected++;
            }

            // If a task remains unpaired it is put back in deque
            if (to_merge % 2 != 0) {
                Task *leftover = current_level_merge.front();
                current_level_merge.pop_front();
                current_level_merge.push_back(leftover);
            }
            //at the end of a level we can terminate all the useless workers
            terminate_useless_worker();
        }

        return GO_ON;
    }

    T *to_sort;
    size_t to_sort_len;
    unsigned int num_workers;
    unsigned int par_degree;
    size_t base_case_size;
    unsigned int level_merges_expected = 0;
    unsigned int level_merges_completed = 0;
    vector<Task> reusable_tasks;
    deque<Task *> current_level_merge;
};

template<typename T>
class ff_MergeSort_Map : public ff_Farm<void> {
public:

    ff_MergeSort_Map(T *data, size_t len, unsigned int par_degree, unsigned int base_case_size = 0)
        : ff_Farm(
              [&]() {
                  vector<unique_ptr<ff_node>> W;
                  for (unsigned int i = 0; i < par_degree - 1; ++i)
                      W.push_back(make_unique<Worker<T>>(data, len));
                  return W;
              }(),
              make_unique<Master<T>>(data, len, par_degree, base_case_size)) {
        this->remove_collector();
        this->wrap_around();
    }

#ifdef MPI_VERSION

    ff_MergeSort_Map(T *data, size_t len, unsigned int par_degree, MPI_Comm comm, unsigned int base_case_size = 0)
        : ff_MergeSort_Map(data, len, par_degree, base_case_size) {
        active_comm = comm;
        if (multiple_process_in_this_node()) {
            // Multiple MPI processes are running on this node.
            // If we don't disable FastFlow's default thread-to-core mapping,
            // processes like proc1 and proc2 may each allocate their threads
            // on the same set of cores (according to the fixed mapping policy),
            // causing severe core overlap and contention.
            // Disabling the mapping allows the OS to schedule threads on free cores,
            // which generally improves overall parallel performance.
            fprintf(stderr, "Disabling default mapping (multiple MPI processes on this node)\n");
            this->ff_farm::no_mapping();
        }
    }

private:
    MPI_Comm active_comm;

    bool multiple_process_in_this_node() {
        MPI_Comm shared_comm;
        MPI_Comm_split_type(active_comm, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shared_comm);

        int local_rank_count;
        MPI_Comm_size(shared_comm, &local_rank_count);
        MPI_Comm_free(&shared_comm);
        return local_rank_count > 1;
    }
#endif
};


#endif // FF_MERGESORT_FARM_H
