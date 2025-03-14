#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <hpc_helpers.hpp>
#include <avx_mathfun.h>

// Static table for fast retrieval of the correct mask to properly
// handle values of K that are not multiples of 8
static const __m256i remaining_mask_table[7] = {
    _mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, -1),
    _mm256_set_epi32(0, 0, 0, 0, 0, 0, -1, -1),
    _mm256_set_epi32(0, 0, 0, 0, 0, -1, -1, -1),
    _mm256_set_epi32(0, 0, 0, 0, -1, -1, -1, -1),
    _mm256_set_epi32(0, 0, 0, -1, -1, -1, -1, -1),
    _mm256_set_epi32(0, 0, -1, -1, -1, -1, -1, -1),
    _mm256_set_epi32(0, -1, -1, -1, -1, -1, -1, -1)
};

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

inline float unrolled_max_inside_reg(__m256 reg) {
    alignas(32) float tmp[8];
    _mm256_store_ps(tmp, reg);

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

float avx_max(const float *data, size_t length) {
    __m256 max_reg = _mm256_set1_ps(-INFINITY);
    size_t i = 0;
    // Find the max value in groups of 8 floats
    // We maintain the maximum at a stride of 8 positions
    for (i = 0; i + 8 <= length; i += 8) {
        __m256 reg_block = _mm256_load_ps(&data[i]);
        max_reg = _mm256_max_ps(max_reg, reg_block);
    }
    size_t remaining = length - i;
    // Handling of remaining elements that do not form complete groups of 8
    if (remaining > 0) {
        __m256i mask = remaining_mask_table[remaining - 1];
        //load element using mask
        __m256 remaining_reg = _mm256_maskload_ps(data + i, mask);
        __m256 neg_inf_vec = _mm256_set1_ps(-INFINITY);
        //use blend to maintain only significant element in the registry for in max search
        __m256 vec = _mm256_blendv_ps(neg_inf_vec, remaining_reg, _mm256_castsi256_ps(mask));
        max_reg = _mm256_max_ps(max_reg, vec);
    }
    return unrolled_max_inside_reg(max_reg);
}

void divide_output_by_sum(float *output, size_t K, float sum) {
    __m256 divisor = _mm256_set1_ps(sum);
    size_t i;
    for (i = 0; i + 8 <= K; i += 8) {
        __m256 reg_block = _mm256_loadu_ps(output + i);
        __m256 reg_block_res = _mm256_div_ps(reg_block, divisor);
        _mm256_storeu_ps(output + i, reg_block_res);
    }
    size_t remaining = K - i;
    if (remaining > 0) {
        __m256i mask = remaining_mask_table[remaining - 1];
        // load with mask
        __m256 remaining_reg = _mm256_maskload_ps(output + i, mask);
        __m256 reg_block_res = _mm256_div_ps(remaining_reg, divisor);
        //store with mask to avoid overflow
        _mm256_maskstore_ps(output + i, mask, reg_block_res);
    }
}

float calculate_output_and_sum(const float *input, float *output, size_t K, float max_val) {
    __m256 max_reg = _mm256_set1_ps(max_val);
    __m256 sum_reg = _mm256_setzero_ps();
    size_t i;
    for (i = 0; i + 8 <= K; i += 8) {
        __m256 current_reg = _mm256_loadu_ps(input + i);
        // Subtraction of max and exponentiation
        __m256 res_reg = exp256_ps(_mm256_sub_ps(current_reg, max_reg));
        _mm256_storeu_ps(output + i, res_reg);
        sum_reg = _mm256_add_ps(res_reg, sum_reg);
    }
    size_t remaining = K - i;
    if (remaining > 0) {
        __m256i mask = remaining_mask_table[remaining - 1];
        __m256 remaining_reg = _mm256_maskload_ps(input + i, mask);
        __m256 res_reg = exp256_ps(_mm256_sub_ps(remaining_reg, max_reg));
        _mm256_maskstore_ps(output + i, mask, res_reg);
        //for sum we need to reset to 0 non-relevant value
        __m256 zero_vec = _mm256_set1_ps(0);
        __m256 vec = _mm256_blendv_ps(zero_vec, res_reg, _mm256_castsi256_ps(mask));
        sum_reg = _mm256_add_ps(sum_reg, vec);
    }
    return hsum_avx(sum_reg);
}

void softmax_avx(const float *input, float *output, size_t K) {
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
