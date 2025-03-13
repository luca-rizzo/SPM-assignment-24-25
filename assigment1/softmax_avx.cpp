#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <hpc_helpers.hpp>
#include <avx_mathfun.h>

// Horizontal sum using SSE3 (4 floats)
inline float hsum_sse3(__m128 v) {
    __m128 shuf = _mm_movehdup_ps(v);
    __m128 maxs = _mm_add_ps(v, shuf);
    shuf = _mm_movehl_ps(shuf, maxs);
    maxs = _mm_add_ss(maxs, shuf);
    return _mm_cvtss_f32(maxs);
}

// Horizontal sum using AVX (8 floats)
inline float hsum_avx(__m256 v) {
    __m128 lo = _mm256_castps256_ps128(v);
    __m128 hi = _mm256_extractf128_ps(v, 1);
    lo = _mm_add_ps(lo, hi);
    return hsum_sse3(lo);
}

float avx_max(const float *data, uint64_t length) {
    __m256 maxVec = _mm256_set1_ps(-INFINITY);
    uint64_t i = 0;
    for (i = 0; i + 8 <= length; i += 8) {
        __m256 vec = _mm256_load_ps(&data[i]);
        maxVec = _mm256_max_ps(maxVec, vec);
    }

    // **Gestione degli elementi rimanenti** (se length non è multiplo di 8)
    alignas(32) float remaining[8] = {-INFINITY, -INFINITY, -INFINITY, -INFINITY,
                                  -INFINITY, -INFINITY, -INFINITY, -INFINITY};
    for (; i < length; i++) {
        remaining[i % 8] = data[i];
    }
    __m256 vec = _mm256_load_ps(remaining);
    maxVec = _mm256_max_ps(maxVec, vec);


    alignas(32) float tmp[8];

    _mm256_store_ps(tmp, maxVec);

    float max_0 = tmp[0];
    float max_1 = tmp[1];
    float max_2 = tmp[2];
    float max_3 = tmp[3];
    max_0 = std::max(max_0, tmp[4]);
    max_1 = std::max(max_1, tmp[5]);
    max_2 = std::max(max_2, tmp[6]);
    max_3 = std::max(max_3, tmp[7]);
    max_0 = std::max(max_0, max_1);
    max_2 = std::max(max_2, max_3);

    return std::max(max_0, max_2);
}

void divide_output_by_sum(float *output, size_t K, float sum) {
    __m256 divisor = _mm256_set1_ps(sum);
    for (size_t i = 0; i + 8 <= K; i += 8) {
        __m256 reg_block = _mm256_loadu_ps(output + i);
        __m256 reg_block_res = _mm256_div_ps(reg_block, divisor);
        _mm256_storeu_ps(output + i, reg_block_res);
    }
    for (size_t i = K - (K % 8); i < K; i++) {
        output[i] /= sum;
    }
}

#if 0
inline float avx_handle_remaining(const float *input, float *output, size_t K, __m256 max_vals, __m256 sum_vec) {
    if (K % 8 != 0) {
        alignas(32) float remaining[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        alignas(32) float present[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
        for (size_t i = K - (K % 8); i < K; i++) {
            remaining[i % 8] = input[i];
            present[i % 8] = 1;
        }
        __m256 reg_block = _mm256_load_ps(remaining);
        __m256 exp_vals = _mm256_sub_ps(reg_block, max_vals);
        __m256 reg_block_res = exp256_ps(exp_vals);
        _mm256_store_ps(remaining, reg_block_res);
        for (size_t i = K - (K % 8); i < K; i++) {
            output[i] = remaining[i % 8];
        }
        __m256 reg_present = _mm256_load_ps(present);
        __m256 zero = _mm256_set1_ps(0.0f);
        // create the mask considering elements LT zero
        __m256 mask = _mm256_cmp_ps(reg_present, zero, _CMP_LT_OS);
        __m256 blend = _mm256_blendv_ps(reg_block_res, zero, mask);
        sum_vec = _mm256_add_ps(blend, sum_vec);
    }
    return hsum_avx(sum_vec);
}
#endif

float calculate_output_and_sum(const float *input, float *output, size_t K, float max_val) {
    __m256 max_vals = _mm256_set1_ps(max_val);
    __m256 sum_vec = _mm256_setzero_ps();
    for (size_t i = 0; i + 8 <= K; i += 8) {
        __m256 reg_block = _mm256_loadu_ps(input + i);
        __m256 exp_vals = _mm256_sub_ps(reg_block, max_vals);
        __m256 reg_block_res = exp256_ps(exp_vals);
        _mm256_storeu_ps(output + i, reg_block_res);
        sum_vec = _mm256_add_ps(reg_block_res, sum_vec);
    }
    //simple handle remaining
    float sum = hsum_avx(sum_vec);
    for (size_t i = K - (K % 8); i < K; i++) {
        output[i] = std::exp(input[i] - max_val);
        sum += output[i];
    }
    return sum;
}

void softmax_avx(const float *input, float *output, size_t K) {
    // Find the maximum to stabilize the computation of the exponential
    float max_val = avx_max(input, K);
    float sum = calculate_output_and_sum(input, output, K, max_val);
    divide_output_by_sum(output, K, sum);
}

std::vector<float> generate_random_input(size_t K, float min = -1.0f, float max = 1.0f) {
    std::vector<float> input(K);
    //std::random_device rd;
    //std::mt19937 gen(rd());
    std::mt19937 gen(5489); // fixed seed for reproducible results
    std::uniform_real_distribution<float> dis(min, max);
    for (size_t i = 0; i < K; ++i) {
        input[i] = dis(gen);
    }
    return input;
}

void printResult(std::vector<float> &v, size_t K) {
    for (size_t i = 0; i < K; ++i) {
        std::fprintf(stderr, "%f\n", v[i]);
    }
}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::printf("use: %s K [1]\n", argv[0]);
        return 0;
    }
    size_t K = 0;
    if (argc >= 2) {
        K = std::stol(argv[1]);
    }
    bool print = false;
    if (argc == 3) {
        print = true;
    }
    std::vector<float> input = generate_random_input(K);
    std::vector<float> output(K);

    TIMERSTART(softime_avx);
    softmax_avx(input.data(), output.data(), K);
    TIMERSTOP(softime_avx);

    // print the results on the standard output
    if (print) {
        printResult(output, K);
    }
}
