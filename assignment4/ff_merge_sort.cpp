#include <variant>
#include <vector>
#include <ff/ff.hpp>

#include "cmdline_merge_parser.hpp"
#include "generate_input_array.h"
#include "merge_sort_utility.h"

using namespace ff;
using namespace std;

size_t CUTOFF = 1024;

struct MergeTask {
    // Left source half is ptr[left_start_index : left_end_index].
    // Right source half is ptr[left_end_index + 1 : right_end_index ].
    size_t left_start_index;
    size_t left_end_index;
    size_t right_end_index;
};

struct BaseCaseTask {
    size_t start_index;
    size_t end_index;
};

struct Task {
    BaseCaseTask *base;
    MergeTask *merge_task;
};

struct Worker : ff_minode_t<Task> {
    Worker(vector<reference_wrapper<Record> > &to_sort): to_sort(to_sort) {
    }

    Task *svc(Task *in) {
        if (in->base) {
            BaseCaseTask *base_case_task = in->base;
            auto first = to_sort.begin();
            auto last = to_sort.begin();
            std::advance(first, base_case_task->start_index);
            std::advance(last, base_case_task->end_index + 1);
            std::sort(first, last, [](auto &a, auto &b) {
                return !record_comp(a.get(), b.get());
            });
        } else {
            MergeTask *merge_task = in->merge_task;
            merge(to_sort, merge_task->left_start_index,
                  merge_task->left_end_index,
                  merge_task->right_end_index);
        }
        return in;
    }

    vector<reference_wrapper<Record> > &to_sort;
};

struct Master : ff_monode_t<Task> {
    Master(vector<reference_wrapper<Record> > &to_sort): to_sort(to_sort) {
        max_level = static_cast<size_t>(log2(to_sort.size() / CUTOFF));
        current_level_merge = deque<Task *>();
    }

    Task *svc(Task *in) {
        if (in == nullptr) {
            size_t size = to_sort.size();
            //divide for all base case
            for (size_t i = 0; i < size; i += CUTOFF) {
                size_t task_end_index = min(size - 1, i + CUTOFF - 1);
                Task *task = new Task();
                task->base = new BaseCaseTask(i, task_end_index);
                ff_send_out(task);
                current_level_merge.push_back(task);
                expected_merges++;
            }
            return GO_ON;
        }
        //ho ricevuto tutte le notifiche di merge per il livello corrente
        merge_per_level++;
        if (merge_per_level == expected_merges) {
            if (current_level == max_level) {
                if (current_level_merge.size() == 2) {
                    Task* l_task = current_level_merge.front();
                    current_level_merge.pop_front();
                    Task* r_task = current_level_merge.front();
                    current_level_merge.pop_front();
                    size_t left_start_index = l_task->base
                                                  ? l_task->base->start_index
                                                  : l_task->merge_task->left_start_index;
                    size_t left_end_index = l_task->base ? l_task->base->end_index : l_task->merge_task->right_end_index;
                    size_t right_end_index = r_task->base ? r_task->base->end_index : r_task->merge_task->right_end_index;
                    std::cout << "[merge] left_start = " << left_start_index
                              << ", left_end = " << left_end_index
                              << ", right_end = " << right_end_index << std::endl;

                    Task *task = new Task();
                    task->merge_task = new MergeTask{left_start_index, left_end_index, right_end_index};
                    task->base = nullptr;
                    merge_per_level = 0;
                    expected_merges = 1;
                    ff_send_out(task);
                    return GO_ON;
                }
                return EOS;
            }
            size_t size = current_level_merge.size();
            deque<Task *> nextLevelMerge = deque<Task *>();
            expected_merges = 0;
            for (size_t i = 0; i + 1 < size; i += 2) {
                Task* l_task = current_level_merge.front();
                current_level_merge.pop_front();
                Task* r_task = current_level_merge.front();
                current_level_merge.pop_front();
                size_t left_start_index = l_task->base
                                              ? l_task->base->start_index
                                              : l_task->merge_task->left_start_index;
                size_t left_end_index = l_task->base ? l_task->base->end_index : l_task->merge_task->right_end_index;
                size_t right_end_index = r_task->base ? r_task->base->end_index : r_task->merge_task->right_end_index;
                std::cout << "[merge] left_start = " << left_start_index
                          << ", left_end = " << left_end_index
                          << ", right_end = " << right_end_index << std::endl;

                Task *task = new Task();
                task->merge_task = new MergeTask{left_start_index, left_end_index, right_end_index};
                task->base = nullptr;
                ff_send_out(task);
                nextLevelMerge.push_back(task);
                expected_merges++;
            }
            cout << size << endl;
            if (size % 2 != 0) {
                nextLevelMerge.push_back(current_level_merge.front());
            }
            current_level_merge = std::move(nextLevelMerge);
            current_level++;
            merge_per_level = 0;
        }
        return GO_ON;
    }

    void free_task(Task *in) {
        if (in->merge_task != nullptr) {
            delete in->merge_task;
        }
        if (in->base != nullptr) {
            delete in->base;
        }
        delete in;
    }

    vector<reference_wrapper<Record> > &to_sort;
    size_t max_level;
    size_t current_level = 0;
    size_t merge_per_level = 0;
    size_t expected_merges = 0;
    deque<Task *> current_level_merge;
    int par_degree;
};


int main(int argc, char **argv) {
    RunningParam running_param = parseCommandLine(argc, argv);
    debug_params(running_param);
    vector<Record> to_sort = generate_input_array(running_param.array_size, running_param.record_payload_size);
    vector<reference_wrapper<Record> > refs(to_sort.begin(), to_sort.end());
    // create a farm
    ff_farm farm;
    Master m(refs);
    farm.add_emitter(&m);
    std::vector<ff_node *> W;
    for (int i = 0; i < running_param.ff_num_threads - 1; ++i) W.push_back(new Worker(refs));
    farm.add_workers(W);
    farm.wrap_around();
    farm.cleanup_workers();
    if (farm.run_and_wait_end() < 0) {
        error("running the farm\n");
    }
    print_sort_res(refs);
}
