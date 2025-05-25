#include <algorithm>
#include <cmdline_merge_parser.hpp>
#include <hpc_helpers.hpp>
#include <iostream>

#include "generate_input_array.hpp"
#include "merge_sort_utility.hpp"

#define CUTOFF 2000

using namespace std;

void merge_sort(vector<reference_wrapper<Record> > &to_sort, size_t start, size_t end) {
    size_t size = end - start + 1;
    if (size <= CUTOFF) {
        auto first = to_sort.begin();
        auto last = to_sort.begin();
        std::advance(first, start);
        std::advance(last, end + 1);
        std::sort(first, last, [](auto& a, auto& b) {
            return second_bigger(a.get(), b.get());
        });
        return;
    }
    size_t middle = (start + end) / 2;
    merge_sort(to_sort, start, middle);
    merge_sort(to_sort, middle + 1, end);
    merge(to_sort, start, middle, end);
}

int main(int argc, char **argv) {
    RunningParam running_param = parseCommandLine(argc, argv);
    //debug_params(running_param);
    vector<Record> to_sort = generate_input_array(running_param.array_size, running_param.record_payload_size);
    vector<reference_wrapper<Record> > refs(to_sort.begin(), to_sort.end());
    TIMERSTART(seq_merge_sort);
    merge_sort(refs, 0, refs.size() - 1);
    TIMERSTOP(seq_merge_sort);
    print_sort_res(refs);

}
