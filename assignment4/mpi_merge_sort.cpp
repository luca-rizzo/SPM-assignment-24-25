#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <vector>

#include "cmdline_merge_parser.hpp"
#include "ff_MergeSort_Map.hpp"
#include "generate_input_array.hpp"
#include "hpc_helpers.hpp"
#include "merge_sort_utility.hpp"

#define MPI_SAFE_CALL(call, MPI_COMM) do {                                         \
    int err = (call);                                               \
    if (err != MPI_SUCCESS) {                                       \
        char errstr[MPI_MAX_ERROR_STRING];                                \
        int errlen = 0;                                                   \
        MPI_Error_string(err, errstr, &errlen);                           \
        fprintf(stderr,                                                   \
        "MPI error in %s\n"                                       \
        "  Code: %d (%.*s)\n"                                     \
        "  Location: %s:%d\n",                                    \
        #call, err, errlen, errstr, __FILE__, __LINE__);         \
        MPI_Abort(MPI_COMM, err);                                   \
        std::abort();                                                     \
    }                                                               \
} while(0)

#define MPI_TIME_AND_LOG(label, call, rank) do {                            \
    double t_start = MPI_Wtime();                                       \
    call;                                                               \
    if (rank == 0) {                                              \
        double t_end = MPI_Wtime();                               \
        fprintf(stderr, "%s time: %f s\n", label, t_end - t_start);     \
    }                                                                   \
} while(0)

using namespace std;

int get_my_rank(const MPI_Comm &COMM) {
    int rank;
    MPI_Comm_rank(COMM, &rank);
    return rank;
}

int largest_power_of_two(const int n) {
    int p = 1;
    while (p * 2 <= n) p *= 2;
    return p;
}

int get_number_of_nodes() {
    int number_of_nodes;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_nodes);
    return number_of_nodes;
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

void scatter_base_case(int number_of_nodes, const MPI_Comm &ACTIVE_COMM, int number_of_records,
                       const vector<Record> &records, vector<int> &displs,
                       vector<int> &sendcounts, vector<Record> &sorted_records,
                       MPI_Datatype MPI_RECORD_TYPE) {
    int rank;
    MPI_Comm_rank(ACTIVE_COMM, &rank);
    int chunk_size = (number_of_records + number_of_nodes - 1) / number_of_nodes;


    sendcounts.resize(number_of_nodes);
    displs.resize(number_of_nodes);

    for (int i = 0; i < number_of_nodes; ++i) {
        int start = i * chunk_size;
        int end = min(start + chunk_size, number_of_records);
        sendcounts[i] = end - start;
        displs[i] = start;
    }

    MPI_SAFE_CALL(MPI_Scatterv(records.data(), sendcounts.data(), displs.data(), MPI_RECORD_TYPE,
                      sorted_records.data() + displs[rank], sendcounts[rank], MPI_RECORD_TYPE, 0, ACTIVE_COMM),
                  ACTIVE_COMM);
}

MPI_Datatype create_mpi_record_type() {
    MPI_Datatype MPI_RECORD;
    const int count = 2;
    int block_lengths[2] = {1, 1};
    MPI_Datatype types[2] = {MPI_UNSIGNED_LONG, MPI_UINT64_T};

    MPI_Aint offsets[2];
    offsets[0] = offsetof(Record, key);
    offsets[1] = offsetof(Record, payload); //treat address as an integer

    MPI_Type_create_struct(count, block_lengths, offsets, types, &MPI_RECORD);
    MPI_Type_commit(&MPI_RECORD);
    return MPI_RECORD;
}


int get_my_sender_at_level(int rank, int level) {
    return rank + (1 << level);
}

int get_my_receiver_at_level(int rank, int level) {
    return rank - (1 << level);
}

vector<int> get_my_sender_at_all_level(int rank, int max_level) {
    vector<int> senders;
    for (int level = 0; level < max_level; ++level) {
        if (am_i_receiver(rank, level, max_level)) {
            int sender_at_level = get_my_sender_at_level(rank, level);
            senders.push_back(sender_at_level);
        }
    }
    return senders;
}

void sort_base_case(const RunningParam &running_param,
                    Record *my_sorting_point_start,
                    int my_send_count) {
    cout << my_send_count << endl;
    // create a map
    ff_MergeSort_Map farm(my_sorting_point_start, my_send_count, running_param.ff_num_threads);

    if (farm.run_and_wait_end() < 0) {
        error("running the farm\n");
    }
}

// Ensure that array_size is safe for all MPI operations that rely on `int` counts.
//
// MPI functions such as MPI_Scatterv, MPI_Send, and MPI_Recv use `int` parameters
// to specify the number of elements to send or receive. This includes:
//   - MPI_Scatterv: all values in `sendcounts[]` and `displs[]` must be ≤ INT_MAX.
//   - MPI_Send / MPI_Recv: maximum message size must be ≤ INT_MAX elements and in
//      merge steps, at the highest level, a process may receive and send up to half of the array.
//
// This check ensures that no part of the distributed computation (from initial
// scatter to final merge) attempts to send or receive more elements than MPI can handle.
int check_on_array_size(int rank, RunningParam running_param) {
    if (running_param.array_size > static_cast<size_t>(numeric_limits<int>::max())) {
        if (rank == 0) {
            fprintf(stderr,
                    "Error: array_size (%zu) is too large — may exceed MPI count limits (max allowed: %d)\n",
                    running_param.array_size, numeric_limits<int>::max()
            );
        }
        MPI_Finalize();
        throw overflow_error("array_size too large: may exceed MPI count limits during Scatterv and final merge");
    }
    return static_cast<int>(running_param.array_size);
}

void init_MPI_threads_checks(int argc, char **argv) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        printf("MPI does not provide required threading support\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        abort();
    }
    int is_main_flag;
    MPI_Is_thread_main(&is_main_flag);
    if (!is_main_flag) {
        printf("This thread called MPI_Init_thread but it is not the main thread\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
        abort();
    }
}

MPI_Comm setup_active_comm(int &rank, int &number_of_nodes) {
    MPI_Comm active_comm;
    int world_rank = get_my_rank(MPI_COMM_WORLD);
    int world_size = get_number_of_nodes();
    //if number of nodes is not a multiple of 2 all processes with rank above the maximum will be included in the new communicator
    //using color 0, otherwise with MPI_UNDEFINED they will be excluded and then they will stop
    if (!(world_size & (world_size - 1)) == 0) {
        int active_nodes = largest_power_of_two(world_size);

        if (world_rank == 0) {
            fprintf(stderr,
                    "Warning: number of processes (%d) is not a power of 2.\n"
                    "Only the first %d ranks will participate in sorting. Others will exit.\n",
                    world_size, active_nodes);
        }

        int color = world_rank < active_nodes ? 0 : MPI_UNDEFINED;
        MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &active_comm);

        if (color == MPI_UNDEFINED) {
            MPI_Finalize();
            exit(0);
        }

        rank = get_my_rank(active_comm);
        number_of_nodes = active_nodes;
    } else {
        active_comm = MPI_COMM_WORLD;
        rank = world_rank;
        number_of_nodes = world_size;
    }

    return active_comm;
}


int main(int argc, char **argv) {
    init_MPI_threads_checks(argc, argv);
    int rank, number_of_nodes;
    MPI_Comm ACTIVE_COMM = setup_active_comm(rank, number_of_nodes);
    RunningParam running_param = parseCommandLine(argc, argv);
    int number_of_records = check_on_array_size(rank, running_param);
    vector<Record> records;
    if (rank == 0) {
        debug_params(running_param);
        records = generate_input_array(number_of_records,
                                       running_param.record_payload_size);
    }
    vector<Record> sorted_records(number_of_records);
    MPI_Datatype MPI_RECORD_TYPE = create_mpi_record_type();
    MPI_Barrier(ACTIVE_COMM);
    if (rank == 0) {
        fprintf(stderr, "STARTED\n");
    }
    double t_start = MPI_Wtime();
    vector<int> displs;
    vector<int> sendcounts;
    scatter_base_case(
        number_of_nodes, ACTIVE_COMM, number_of_records, records, displs, sendcounts, sorted_records,
        MPI_RECORD_TYPE);
    int max_level = static_cast<int>(log2(number_of_nodes));
    int level = 0;
    vector<int> my_senders = get_my_sender_at_all_level(rank, max_level);
    vector<MPI_Request> requests(my_senders.size());
    int base_case_size = sendcounts[rank];
    int levels_to_receive = static_cast<int>(my_senders.size());
    for (int i = 0; i < levels_to_receive; ++i) {
        int sender_level = my_senders[i];
        int receiving_point_level = displs[sender_level];
        //given the binary structure at level "i" I will receive at most base_case * 2^i element
        //and will be always <= array_size/2 so it will fit in an integer
        int max_elem_level = base_case_size * (1 << i);
        MPI_SAFE_CALL(MPI_Irecv(sorted_records.data() + receiving_point_level, max_elem_level,
                          MPI_RECORD_TYPE, my_senders[i], 0, ACTIVE_COMM, &requests[i]), ACTIVE_COMM);
    }
    // Sort the local base case in parallel with all the Irecv
    Record *my_records_begin = sorted_records.data() + displs[rank];
    MPI_TIME_AND_LOG("Base case sorting",
                     sort_base_case(running_param, my_records_begin, base_case_size), rank);
    // initially only my base case is sorted
    int sorted_partition_size = base_case_size;
    while (level < max_level) {
        if (am_i_sender(rank, level, max_level)) {
            int receiver = get_my_receiver_at_level(rank, level);
            //send to receiver at level "level" all the data of the partition for which
            // I am responsible
            MPI_SAFE_CALL(MPI_Send(my_records_begin, sorted_partition_size,
                              MPI_RECORD_TYPE,
                              receiver, 0, ACTIVE_COMM), ACTIVE_COMM);
            break; // A sender no longer participates in future merge steps
        }
        if (am_i_receiver(rank, level, max_level)) {
            MPI_Status status;
            MPI_SAFE_CALL(MPI_Wait(&requests[level], &status), ACTIVE_COMM);
            int real_received_count;
            MPI_Get_count(&status, MPI_RECORD_TYPE, &real_received_count);
            // At level "level", I receive a contiguous and independently sorted partition
            // from my sender. It starts immediately after my current sorted partition:
            // [my_records_begin, my_records_begin + sorted_partition_size - 1] is my current sorted range.
            // The incoming sorted range begins at my_records_begin + sorted_partition_size.
            Record *my_sorted_partition_end = my_records_begin + sorted_partition_size;
            inplace_merge(my_records_begin,
                          my_sorted_partition_end,
                          my_sorted_partition_end + real_received_count);
            sorted_partition_size += real_received_count;
        }
        level++;
    }

    if (rank == 0) {
        double t_stop = MPI_Wtime();
        cout << "# elapsed time (mpi_merge_sort): "
                << (t_stop - t_start) << "s" << endl;
        print_sort_res(sorted_records);
    }
    MPI_Type_free(&MPI_RECORD_TYPE);
    MPI_Finalize();
}
