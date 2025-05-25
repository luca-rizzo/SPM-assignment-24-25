#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <vector>

#include "cmdline_merge_parser.hpp"
#include "generate_input_array.hpp"
#include "hpc_helpers.hpp"

using namespace std;

int get_my_rank(MPI_Comm COMM) {
    int rank;
    MPI_Comm_rank(COMM, &rank);
    return rank;
}

int largest_power_of_two(int n) {
    int p = 1;
    while (p * 2 <= n) p *= 2;
    return p;
}

int get_number_of_nodes() {
    int number_of_nodes;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_nodes);
    return largest_power_of_two(number_of_nodes);
}

vector<unsigned long> merge_two_sorted(const vector<unsigned long>& v1, const vector<unsigned long>& v2) {
    vector<unsigned long> res(v1.size() + v2.size());
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;
    while (i < v1.size() && j < v2.size()) {
        if (v1[i] < v2[j]) {
            res[k] = v1[i];
            i++;
            k++;
        } else {
            res[k] = v2[j];
            j++;
            k++;
        }
    }
    while (i < v1.size()) {
        res[k] = v1[i];
        i++;
        k++;
    }
    while (j < v2.size()) {
        res[k] = v2[j];
        j++;
        k++;
    }
    return res;
}

bool am_i_receiver(int rank, int level) {
    return (rank % (1 << (level + 1))) == 0;
}

bool am_i_sender(int rank, int level) {
    int group_size = 1 << (level + 1);
    return (rank % group_size) == (1 << level);
}

inline bool check_sort(const vector<unsigned long > &sorted) {
    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i - 1] > sorted[i]) {
            return false;
        }
    }
    return true;
}


vector<unsigned long> scatter_base_case_and_sort(int number_of_nodes, MPI_Comm ACTIVE_COMM, RunningParam running_param,
    vector<unsigned long> keys) {
    size_t base_case_size = running_param.array_size / number_of_nodes;
    vector<unsigned long> base_case(base_case_size);
    MPI_Scatter(keys.data(), base_case_size, MPI_UNSIGNED_LONG, base_case.data(),
                base_case_size, MPI_UNSIGNED_LONG, 0, ACTIVE_COMM);
    sort(base_case.begin(), base_case.end(), [](auto &a, auto &b) {
        return b > a;
    });
    return base_case;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank = get_my_rank(MPI_COMM_WORLD);
    int number_of_nodes = get_number_of_nodes();
    MPI_Comm ACTIVE_COMM;
    int color = (rank < number_of_nodes) ? 0 : MPI_UNDEFINED;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &ACTIVE_COMM);

    if (rank >= number_of_nodes) {
        MPI_Finalize();
        return 0;
    }
    rank = get_my_rank(ACTIVE_COMM);
    RunningParam running_param = parseCommandLine(argc, argv);
    vector<unsigned long> keys;
    vector<Record> to_sort;
    if (rank == 0) {
        to_sort = generate_input_array_to_distribute(running_param.array_size,
                                                     running_param.record_payload_size, keys);
    }
    MPI_Barrier(ACTIVE_COMM);
    double t_start = MPI_Wtime();
    std::vector<unsigned long> sorted_block = scatter_base_case_and_sort(number_of_nodes, ACTIVE_COMM, running_param, keys);
    size_t sub_array_level_size = sorted_block.size();
    int max_level = log2(number_of_nodes);
    int level = 0;
    while (level < max_level) {
        if (am_i_sender(rank, level)) {
            int receiver = rank - (1 << level);
            MPI_Send(sorted_block.data(), sub_array_level_size, MPI_UNSIGNED_LONG, receiver, 0, ACTIVE_COMM);
        } else if (am_i_receiver(rank, level)) {
            int sender = rank + (1 << level);
            std::vector<unsigned long> received_to_merge(sub_array_level_size);
            MPI_Recv(received_to_merge.data(), sub_array_level_size, MPI_UNSIGNED_LONG, sender, 0, ACTIVE_COMM,
                     MPI_STATUS_IGNORE);
            sorted_block = merge_two_sorted(sorted_block, received_to_merge);
            // in general at level 2^l we should have:
            // (base_case) * 2^l number of element already merged
            sub_array_level_size = sorted_block.size();
        }
        level++;
    }
    if (rank == 0) {
        double t_stop = MPI_Wtime();
        std::cout << "# elapsed time (mpi_merge_sort): "                       \
        << (t_stop - t_start)  << "s" << endl;
        cout << "Sorted: " << (check_sort(sorted_block) && sorted_block.size() == keys.size()) << endl;
    }
    MPI_Finalize();
}
