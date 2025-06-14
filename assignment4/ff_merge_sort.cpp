#include <variant>
#include <vector>
#include <ff/ff.hpp>

#include "cmdline_merge_parser.hpp"
#include "ff_MergeSort_Map.hpp"
#include "generate_input_array.hpp"
#include "merge_sort_utility.hpp"

using namespace ff;
using namespace std;


int main(int argc, char **argv) {
    RunningParam running_param = parseCommandLine(argc, argv);
    debug_params(running_param);
    vector<Record> to_sort = generate_input_array(running_param.array_size,
                                                  running_param.record_payload_size);
    // create a farm
    ff_MergeSort_Map farm(to_sort.data(), to_sort.size(),
                          running_param.ff_num_threads,
                          running_param.base_case_size);
    TIMERSTART(ff_merge_sort);
    if (farm.run_and_wait_end() < 0) {
        error("running the farm\n");
    }
    TIMERSTOP(ff_merge_sort);
    print_sort_res(to_sort);
    free_input_array(to_sort);
}
