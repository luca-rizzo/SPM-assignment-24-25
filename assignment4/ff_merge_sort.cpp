#include <variant>
#include <vector>
#include <ff/ff.hpp>

#include "cmdline_merge_parser.hpp"
#include "generate_input_array.hpp"
#include "merge_sort_utility.hpp"

using namespace ff;
using namespace std;


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
    std::variant<std::monostate, BaseCaseTask, MergeTask> payload;

    bool is_base() const {
        return std::holds_alternative<BaseCaseTask>(payload);
    }

    BaseCaseTask &as_base() {
        return std::get<BaseCaseTask>(payload);
    }

    MergeTask &as_merge() {
        return std::get<MergeTask>(payload);
    }
};

struct Worker : ff_minode_t<Task> {
    Worker(vector<reference_wrapper<Record> > &to_sort): to_sort(to_sort) {
    }

    Task *svc(Task *in) {
        if (in->is_base()) {
            BaseCaseTask base_case_task = in->as_base();
            auto first = to_sort.begin();
            auto last = to_sort.begin();
            std::advance(first, base_case_task.start_index);
            std::advance(last, base_case_task.end_index + 1);
            std::sort(first, last, [](auto &a, auto &b) {
                return second_bigger(a.get(), b.get());
            });
        } else {
            MergeTask merge_task = in->as_merge();
            merge(to_sort, merge_task.left_start_index,
                  merge_task.left_end_index,
                  merge_task.right_end_index);
        }
        return in;
    }

    vector<reference_wrapper<Record> > &to_sort;
};

struct Master : ff_monode_t<Task> {
    Master(vector<reference_wrapper<Record> > &to_sort, size_t par_degree): to_sort(to_sort), par_degree(par_degree),
                                                                            reusable_tasks(par_degree) {
        current_level_merge = deque<Task *>();
        CUT_OFF = to_sort.size() / par_degree;
    }

    Task *create_and_send_merge_task() {
        Task *l_task = current_level_merge.front();
        current_level_merge.pop_front();
        Task *r_task = current_level_merge.front();
        current_level_merge.pop_front();
        size_t left_start_index = l_task->is_base()
                                      ? l_task->as_base().start_index
                                      : l_task->as_merge().left_start_index;
        size_t left_end_index = l_task->is_base() ?
            l_task->as_base().end_index : l_task->as_merge().right_end_index;
        size_t right_end_index = r_task->is_base() ? r_task->as_base().end_index : r_task->as_merge().right_end_index;
        free_task(l_task);
        free_task(r_task);
        Task *task = new Task();
        task->payload = MergeTask{left_start_index, left_end_index, right_end_index};
        ff_send_out(task);
        return task;
    }

    Task *svc(Task *in) {
        if (in == nullptr) {
            size_t size = to_sort.size();
            //divide for all base case
            for (size_t i = 0; i < size; i += CUT_OFF) {
                size_t task_end_index = min(size - 1, i + CUT_OFF - 1);
                Task *task = new Task();
                task->payload = BaseCaseTask{i, task_end_index};
                ff_send_out(task);
                current_level_merge.push_back(task);
                level_merges_expected++;
            }
            return GO_ON;
        }
        level_merges_completed++;
        //ho ricevuto tutte le notifiche di merge per il livello corrente
        if (level_merges_completed == level_merges_expected) {
            if (current_level_merge.size() == 1) {
                Task *task = current_level_merge.front();
                current_level_merge.pop_front();
                free_task(task);
                return EOS;
            }
            deque<Task *> nextLevelMerge = deque<Task *>();
            level_merges_expected = 0;
            while (current_level_merge.size() > 1) {
                Task *task = create_and_send_merge_task();
                nextLevelMerge.push_back(task);
                level_merges_expected++;
            }
            if (!current_level_merge.empty()) {
                nextLevelMerge.push_back(current_level_merge.front());
            }
            current_level_merge = std::move(nextLevelMerge);
            level_merges_completed = 0;
        }
        return GO_ON;
    }

    static void free_task(const Task *in) {
        delete in;
    }

    vector<reference_wrapper<Record> > &to_sort;
    size_t level_merges_completed = 0;
    size_t level_merges_expected = 0;
    size_t CUT_OFF;
    vector<Task> reusable_tasks;
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
    unsigned int num_workers = running_param.ff_num_threads - 1;
    Master m(refs, num_workers);
    farm.add_emitter(&m);
    std::vector<ff_node *> W;
    for (size_t i = 0; i < num_workers; ++i) W.push_back(new Worker(refs));
    farm.add_workers(W);
    farm.wrap_around();
    farm.cleanup_workers();
    cout << "STARTING " << endl;
    TIMERSTART(ff_merge_sort);
    if (farm.run_and_wait_end() < 0) {
        error("running the farm\n");
    }
    TIMERSTOP(ff_merge_sort);
    print_sort_res(refs);
}
