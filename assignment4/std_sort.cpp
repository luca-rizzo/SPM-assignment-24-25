#include <algorithm>
#include <cmdline_merge_parser.hpp>
#include <hpc_helpers.hpp>
#include <iostream>

#include "generate_input_array.hpp"
#include "merge_sort_utility.hpp"

#define CUTOFF 2000

using namespace std;

int main(int argc, char **argv) {
    RunningParam running_param = parseCommandLine(argc, argv);
    debug_params(running_param);
    vector<Record> to_sort = generate_input_array(running_param.array_size, running_param.record_payload_size);
    fprintf(stderr, "STARTED\n");
    TIMERSTART(std_sort);
    std::sort(to_sort.begin(), to_sort.end(), [](auto& a, auto& b) {
            return a.key < b.key;
        });
    TIMERSTOP(std_sort);
    print_sort_res(to_sort);
}
