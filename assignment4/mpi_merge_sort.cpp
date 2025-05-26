#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <vector>

#include "cmdline_merge_parser.hpp"
#include "ff_MergeSort_Map.hpp"
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

vector<KeyIndex> merge_two_sorted(const vector<KeyIndex> &v1, const vector<KeyIndex> &v2) {
    vector<KeyIndex> res(v1.size() + v2.size());
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

template <typename T>
bool check_sort(const vector<T> &sorted) {
    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i - 1] > sorted[i]) {
            return false;
        }
    }
    return true;
}

vector<KeyIndex> scatter_base_case_and_sort(int number_of_nodes, MPI_Comm ACTIVE_COMM, RunningParam running_param,
                                                 const vector<KeyIndex>& keys, MPI_Datatype MPI_KEY_INDEX) {
    int rank;
    MPI_Comm_rank(ACTIVE_COMM, &rank);
    size_t chunk_size = static_cast<size_t>(
        std::ceil(static_cast<double>(running_param.array_size) / number_of_nodes)
    );

    std::vector<int> sendcounts(number_of_nodes);
    std::vector<int> displs(number_of_nodes);

    for (int i = 0; i < number_of_nodes; ++i) {
        int start = i * chunk_size;
        int end = std::min(start + chunk_size, running_param.array_size);
        sendcounts[i] = end - start;
        displs[i] = start;
    }

    std::vector<KeyIndex> base_case(sendcounts[rank]);

    MPI_Scatterv(keys.data(), sendcounts.data(), displs.data(), MPI_KEY_INDEX,
                 base_case.data(), sendcounts[rank], MPI_KEY_INDEX, 0, ACTIVE_COMM);

    vector<reference_wrapper<KeyIndex> > refs(base_case.begin(), base_case.end());

    // create a map
    ff_MergeSort_Map farm(refs, running_param.ff_num_threads - 1);

    if (farm.run_and_wait_end() < 0) {
        error("running the farm\n");
    }
    return vector<KeyIndex>(refs.begin(), refs.end());
}

MPI_Datatype create_mpi_keyindex_type_struct() {
    MPI_Datatype mpi_keyindex_type;

    int block_lengths[2] = {1, 1};
    MPI_Datatype types[2] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG};
    MPI_Aint displacements[2];

    // Calculate the displacements in case of (possible) padding
    KeyIndex dummy{};
    MPI_Aint base_address;
    MPI_Get_address(&dummy, &base_address);
    MPI_Get_address(&dummy.key, &displacements[0]);
    MPI_Get_address(&dummy.original_index, &displacements[1]);

    displacements[0] -= base_address;
    displacements[1] -= base_address;

    MPI_Type_create_struct(2, block_lengths, displacements, types, &mpi_keyindex_type);
    MPI_Type_commit(&mpi_keyindex_type);

    return mpi_keyindex_type;
}
MPI_Datatype create_mpi_keyindex_type_contiguous() {
    MPI_Datatype MPI_KEY_INDEX;
    MPI_Type_contiguous(2, MPI_UNSIGNED_LONG, &MPI_KEY_INDEX);
    MPI_Type_commit(&MPI_KEY_INDEX);
    return MPI_KEY_INDEX;
}

Record& get_elem_at_position(vector<KeyIndex>& sorted_index, vector<Record>& original_array, size_t position) {
    if(sorted_index.size() < position) {
        throw std::invalid_argument("The requested position does not exsit");
    }
    return original_array[sorted_index[position].original_index];
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
    vector<KeyIndex> keys;
    vector<Record> to_sort;
    if (rank == 0) {
        to_sort = generate_input_array_to_distribute(running_param.array_size,
                                                     running_param.record_payload_size, keys);
    }
    MPI_Datatype MPI_KEY_INDEX = create_mpi_keyindex_type_contiguous();
    MPI_Barrier(ACTIVE_COMM);
    double t_start = MPI_Wtime();
    std::vector<KeyIndex> sorted_block = scatter_base_case_and_sort(
        number_of_nodes, ACTIVE_COMM, running_param, keys, MPI_KEY_INDEX);
    size_t sub_array_level_size = sorted_block.size();
    int max_level = log2(number_of_nodes);
    int level = 0;
    while (level < max_level) {
        if (am_i_sender(rank, level)) {
            int receiver = rank - (1 << level);
            MPI_Send(sorted_block.data(), sub_array_level_size, MPI_KEY_INDEX, receiver, 0, ACTIVE_COMM);
        } else if (am_i_receiver(rank, level)) {
            int sender = rank + (1 << level);
            std::vector<KeyIndex> received_to_merge(sub_array_level_size);
            MPI_Status status;
            MPI_Recv(received_to_merge.data(), sub_array_level_size, MPI_KEY_INDEX,
                sender, 0, ACTIVE_COMM, &status);
            int real_received_count;
            MPI_Get_count(&status, MPI_KEY_INDEX, &real_received_count);
            received_to_merge.resize(real_received_count);
            sorted_block = merge_two_sorted(sorted_block, received_to_merge);
            // in general at level 2^l we should have:
            // (base_case) * 2^l number of element already merged
            sub_array_level_size = sorted_block.size();
        }
        level++;
    }
    if (rank == 0) {
        double t_stop = MPI_Wtime();
        std::cout << "# elapsed time (mpi_merge_sort): "
                << (t_stop - t_start) << "s" << endl;
        cout << "Sorted: " << (check_sort(sorted_block) && sorted_block.size() == to_sort.size()) << endl;
    }
    MPI_Type_free(&MPI_KEY_INDEX);
    MPI_Finalize();
}
