#ifndef FF_MERGESORT_FARM_H
#define FF_MERGESORT_FARM_H

#include <vector>
#include <variant>
#include <memory>
#include <cmath>
#include <optional>
#include <ff/ff.hpp>
#include <iostream>
#include <ff/farm.hpp>

using namespace ff;
using namespace std;

struct MergeTask {
    size_t left_start_index;
    size_t left_end_index;
    size_t right_end_index;
};

struct BaseCaseTask {
    size_t start_index;
    size_t end_index;
};

struct Task {
    variant<monostate, BaseCaseTask, MergeTask> payload;

    bool is_base() const {
        return holds_alternative<BaseCaseTask>(payload);
    }

    BaseCaseTask &as_base() {
        return get<BaseCaseTask>(payload);
    }

    MergeTask &as_merge() {
        return get<MergeTask>(payload);
    }
};

template<totally_ordered T>
struct Worker : ff_minode_t<Task> {
    explicit Worker(vector<T> &to_sort): to_sort(to_sort) {
    }

    Task *svc(Task *in) override {
        if (in->is_base()) {
            auto &base_case_task = in->as_base();
            sort(to_sort.begin() + base_case_task.start_index, to_sort.begin() + base_case_task.end_index + 1,
                 [](const T &a, const T &b) { return a < b; });
        } else {
            auto &merge_task = in->as_merge();
            if (to_sort[merge_task.left_end_index] <= to_sort[merge_task.left_end_index + 1])
                return in;
            inplace_merge(
                to_sort.begin() + merge_task.left_start_index,
                to_sort.begin() + merge_task.left_end_index + 1,
                to_sort.begin() + merge_task.right_end_index + 1,
                [](const T &a, const T &b) { return a < b; });
        }
        return in;
    }

    vector<T> &to_sort;
};

template<totally_ordered T>
struct Master : ff_monode_t<Task> {
    Master(vector<T> &to_sort, int par_degree, size_t _base_case_size)
        : to_sort(to_sort),
          num_workers(par_degree),
          base_case_size(_base_case_size == 0 ? ceil((double) to_sort.size() / par_degree) : _base_case_size) {
        int leaf_of_merge_rec = (to_sort.size() + base_case_size - 1) / base_case_size;
        int max_tasks_needed = 2 * leaf_of_merge_rec; // base + all merge levels
        reusable_tasks = vector<Task>(max_tasks_needed);
    }

    void terminate_useless_worker() {
        while (num_workers > level_merges_expected) {
            --num_workers;
            ff_send_out_to(EOS, num_workers);
        }
    }

    void insert_in_order(Task *in) {
        size_t in_start = in->is_base()
                              ? in->as_base().start_index
                              : in->as_merge().left_start_index;

        auto it = lower_bound(current_level_merge.begin(),
                              current_level_merge.end(), in_start,
                              [](Task *task, size_t value) {
                                  size_t task_start = task->is_base()
                                                          ? task->as_base().start_index
                                                          : task->as_merge().left_start_index;
                                  return task_start < value;
                              });

        current_level_merge.insert(it, in);
    }

    // cerca la prima coppia adiacente, la rimuove, e la restituisce
    optional<pair<Task *, Task *> > extract_first_two_adiacent() {
        for (size_t i = 0; i + 1 < current_level_merge.size(); ++i) {
            Task *current = current_level_merge[i];
            Task *next = current_level_merge[i + 1];

            size_t end_current = current->is_base()
                                     ? current->as_base().end_index
                                     : current->as_merge().right_end_index;
            size_t start_next = next->is_base()
                                    ? next->as_base().start_index
                                    : next->as_merge().left_start_index;

            if (end_current + 1 == start_next) {
                current_level_merge.erase(current_level_merge.begin() + i, current_level_merge.begin() + i + 2);
                return make_pair(current, next);
            }
        }
        return nullopt;
    }

    Task *svc(Task *in) override {
        if (in == nullptr) {
            size_t size = to_sort.size();
            for (size_t i = 0; i < size; i += base_case_size) {
                size_t task_end_index = min(size - 1, i + base_case_size - 1);
                Task *task = &reusable_tasks[next_free_task++];
                task->payload = BaseCaseTask{i, task_end_index};
                ff_send_out(task);
                level_merges_expected++;
            }
            return GO_ON;
        }

        insert_in_order(in);
        level_merges_expected--;

        // Prova a mergiare le prime due adiacenti
        auto maybe_pair = extract_first_two_adiacent();
        if (maybe_pair) {
            Task *l_task = maybe_pair->first;
            Task *r_task = maybe_pair->second;

            size_t left_start_index = l_task->is_base()
                                          ? l_task->as_base().start_index
                                          : l_task->as_merge().left_start_index;
            size_t left_end_index = l_task->is_base()
                                        ? l_task->as_base().end_index
                                        : l_task->as_merge().right_end_index;
            size_t right_end_index = r_task->is_base()
                                         ? r_task->as_base().end_index
                                         : r_task->as_merge().right_end_index;

            Task *task = &reusable_tasks[next_free_task++];
            task->payload = MergeTask{left_start_index, left_end_index, right_end_index};
            level_merges_expected++;
            ff_send_out(task);
        }

        // Se ho ricevuto tutti i task e ho solo uno alla fine → finito
        if (current_level_merge.size() == 1 &&
            level_merges_expected == 0) {
            current_level_merge.clear();
            return EOS;
        }

        return GO_ON;
    }

    vector<T> &to_sort;
    int level_merges_expected = 0;
    int num_workers;
    size_t base_case_size;
    int next_free_task;
    vector<Task> reusable_tasks;
    deque<Task *> current_level_merge;
};

template<totally_ordered T>
class ff_MergeSort_Map : public ff_Farm<void> {
public:
    ff_MergeSort_Map(vector<T> &to_sort, int num_workers, int base_case_size = 0)
        : ff_Farm(
            [&]() {
                vector<unique_ptr<ff_node> > W;
                for (int i = 0; i < num_workers; ++i)
                    W.push_back(make_unique<Worker<T> >(to_sort));
                return W;
            }(),
            make_unique<Master<T> >(to_sort, num_workers, base_case_size)) {
        this->remove_collector();
        this->wrap_around();
    }
};

#endif // FF_MERGESORT_FARM_H
