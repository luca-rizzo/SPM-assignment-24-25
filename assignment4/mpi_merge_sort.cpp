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

void merge_sorted_inplace_tail_expand(vector<Record> &v1, const vector<Record> &v2) {
    size_t n1 = v1.size();
    size_t n2 = v2.size();
    v1.resize(n1 + n2); // expand v1 to hold the full result

    ssize_t i = n1 - 1;
    ssize_t j = n2 - 1;
    ssize_t k = n1 + n2 - 1;

    while (i >= 0 && j >= 0) {
        if (v1[i] > v2[j]) {
            v1[k] = v1[i];
            i--;
        } else {
            v1[k] = v2[j];
            j--;
        }
        k--;
    }

    while (j >= 0) {
        v1[k] = v2[j];
        k--;
        j--;
    }
    // if i >= 0, they're already in place
}

bool am_i_receiver(int rank, int level, int max_level) {
    if (level >= max_level)
        return false;
    return (rank % (1 << (level + 1))) == 0;
}

bool am_i_sender(int rank, int level, int max_level) {
    if (level >= max_level)
        return false;
    int group_size = 1 << (level + 1);
    return (rank % group_size) == (1 << level);
}

template<typename T>
bool check_sort(const vector<T> &sorted) {
    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i - 1] > sorted[i]) {
            return false;
        }
    }
    return true;
}

vector<Record> scatter_base_case(int number_of_nodes, MPI_Comm ACTIVE_COMM, RunningParam running_param,
                                   const vector<Record> &records, MPI_Datatype MPI_RECORD_TYPE) {
    int rank;
    MPI_Comm_rank(ACTIVE_COMM, &rank);
    auto chunk_size = static_cast<size_t>(
        ceil(static_cast<double>(running_param.array_size) / number_of_nodes)
    );

    vector<int> sendcounts(number_of_nodes);
    vector<int> displs(number_of_nodes);

    for (int i = 0; i < number_of_nodes; ++i) {
        int start = i * chunk_size;
        int end = min(start + chunk_size, running_param.array_size);
        sendcounts[i] = end - start;
        displs[i] = start;
    }

    vector<Record> base_case(sendcounts[rank]);
    if(rank == 0) {
        double t_stop = MPI_Wtime();

    }
    double t_start = 0;
    if (rank == 0) {
        t_start = MPI_Wtime();

    }

    MPI_Scatterv(records.data(), sendcounts.data(), displs.data(), MPI_RECORD_TYPE,
                 base_case.data(), sendcounts[rank], MPI_RECORD_TYPE, 0, ACTIVE_COMM);

    if(rank == 0) {
        double t_stop = MPI_Wtime();
        cout << "SCATTER: " << t_stop - t_start << endl;
    }
    return base_case;
}

MPI_Datatype create_mpi_record_type() {
    MPI_Datatype MPI_RECORD;
    const int count = 2;
    int blocklengths[2] = {1, 1};
    MPI_Datatype types[2] = {MPI_UNSIGNED_LONG, MPI_UINT64_T};

    MPI_Aint offsets[2];
    offsets[0] = offsetof(Record, key);
    offsets[1] = offsetof(Record, payload);  //treat address as an integer

    MPI_Type_create_struct(count, blocklengths, offsets, types, &MPI_RECORD);
    MPI_Type_commit(&MPI_RECORD);
    return MPI_RECORD;
}

int get_my_sender_at_level(int rank, int level) {
    return rank + (1 << level);
}

int get_my_receiver_at_level(int rank, int level) {
    return rank - (1 << level);
}

void sort_base_case(const RunningParam &running_param, vector<Record> &base_case) {
    // create a map
    ff_MergeSort_Map farm(base_case, running_param.ff_num_threads);

    if (farm.run_and_wait_end() < 0) {
        error("running the farm\n");
    }
}

bool check_records_match(const std::vector<Record>& original,
                                      const std::vector<Record>& sorted,
                                      size_t payload_size) {
    if (original.size() != sorted.size()) {
        std::cerr << "Size mismatch: original.size() = " << original.size()
                  << ", sorted.size() = " << sorted.size() << "\n";
        return false;
    }

    std::vector<bool> matched(original.size(), false);

    for (size_t i = 0; i < sorted.size(); ++i) {
        const Record& sr = sorted[i];
        bool found = false;

        for (size_t j = 0; j < original.size(); ++j) {
            if (matched[j]) continue;

            const Record& orr = original[j];
            if (sr.key != orr.key) continue;

            bool equal = true;
            for (size_t k = 0; k < payload_size; ++k) {
                if (sr.payload[k] != orr.payload[k]) {
                    equal = false;
                    break;
                }
            }

            if (equal) {
                matched[j] = true;
                found = true;
                break;
            }
        }

        if (!found) {
            std::cerr << "No matching record found for sorted[" << i << "] key = "
                      << sr.key << "\n";
            return false;
        }
    }

    return true;
}

void wait_data_current_level(const MPI_Datatype &MPI_KEY_INDEX,
                             vector<Record> &current_level_received,
                             MPI_Request &current_request) {
    MPI_Status status;
    MPI_Wait(&current_request, &status);
    int real_received_count;
    MPI_Get_count(&status, MPI_KEY_INDEX, &real_received_count);
    current_level_received.resize(real_received_count);
}

int main(int argc, char **argv) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        std::printf("MPI does not provide required threading support\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        std::abort();
    }
    int is_main_flag;
    MPI_Is_thread_main(&is_main_flag);
    if (!is_main_flag) {
        std::printf("This thread called MPI_Init_thread but it is not the main thread\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
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
    vector<Record> records;
    if (rank == 0) {
        records = generate_input_array(running_param.array_size,
                                           running_param.record_payload_size);
    }
    MPI_Datatype MPI_RECORD_TYPE = create_mpi_record_type();
    MPI_Barrier(ACTIVE_COMM);
    if (rank == 0) {
        cout << "STARTED" << endl;
    }
    double t_start = MPI_Wtime();
    vector<Record> sorted_block = scatter_base_case(
        number_of_nodes, ACTIVE_COMM, running_param, records, MPI_RECORD_TYPE);
    size_t sub_array_level_size = sorted_block.size();
    int max_level = log2(number_of_nodes);
    int level = 0;
    vector<Record> current_level_received(sub_array_level_size);
    vector<Record> next_level_received(sub_array_level_size * 2);
    MPI_Request current_request, next_request;

    // Post the initial Irecv for merge at level 0
    if (am_i_receiver(rank, level, max_level)) {
        int sender = get_my_sender_at_level(rank, level);
        MPI_Irecv(current_level_received.data(), sub_array_level_size, MPI_RECORD_TYPE,
                  sender, 0, ACTIVE_COMM, &current_request);
    }

    // Sort the local base case in parallel with the initial Irecv
    sort_base_case(running_param, sorted_block);
    while (level < max_level) {
        if (am_i_sender(rank, level, max_level)) {
            int receiver = get_my_receiver_at_level(rank, level);
            MPI_Send(sorted_block.data(), sub_array_level_size, MPI_RECORD_TYPE,
                     receiver, 0, ACTIVE_COMM);
            break; // A sender no longer participates in future merge steps
        }
        if (am_i_receiver(rank, level, max_level)) {
            // Pre-post the Irecv for the next level if needed
            if (am_i_receiver(rank, level + 1, max_level)) {
                next_level_received.resize(2 * sub_array_level_size);
                int sender = get_my_sender_at_level(rank, level + 1);
                MPI_Irecv(next_level_received.data(), 2 * sub_array_level_size, MPI_RECORD_TYPE,
                          sender, 0, ACTIVE_COMM, &next_request);
            }

            // Complete the current level's receive
            wait_data_current_level(MPI_RECORD_TYPE, current_level_received, current_request);

            // Merge the received block with the current sorted block
            merge_sorted_inplace_tail_expand(sorted_block, current_level_received);
            sub_array_level_size = sorted_block.size();

            // Swap buffers and request for the next iteration
            swap(current_level_received, next_level_received);
            swap(current_request, next_request);
        }
        level++;
    }

    if (rank == 0) {
        double t_stop = MPI_Wtime();
        cout << "# elapsed time (mpi_merge_sort): "
                << (t_stop - t_start) << "s" << endl;
        cout << "Sorted: " << check_sort(sorted_block) << endl;
        //cout << "All records payload match: " << check_records_match(records, sorted_block, running_param.record_payload_size) <<endl;
    }
    MPI_Type_free(&MPI_RECORD_TYPE);
    MPI_Finalize();
}
