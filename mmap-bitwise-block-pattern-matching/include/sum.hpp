#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <cstdint>
#include <memory>
#include <algorithm>

// pattern : the character to search broadcasted into a 64bit integer
// begin : points somewhere into the partition
// len : remaining length of the partition
// return : the position of the first matching character or -1 otherwise
ssize_t find_first(uint64_t pattern, const char* begin, size_t len);

// begin : points somewhere into the partition
// len : remaining length of the partition
// return : 64-bit integer representation of the provided numeric
int64_t read_numeric(const char* begin, size_t len);

// data_start : pointer to the first byte of the file
// partition_start: start of partition
// partition_size_hint : size of partition in bytes (must be corrected to end on
// a newline character).
// result : pointer to the variable where the result should be stored in
template <bool first_partition = false>
void sum_extendedprice(const char* data_start, const char* partition_start,
                       size_t partition_size_hint, int64_t* result) {
    int64_t sum = 0;

    // correct partition size
    if (!first_partition) {
        size_t offset = 0;
        const size_t max_offset = partition_start - data_start;
        while (offset < partition_size_hint) {
            if (offset > max_offset) {
                return;
            } else if (*(partition_start - offset) == '\n') {
                break;
            } else {
                offset++;
            }
        }
        partition_start -= offset;
        partition_size_hint += offset;
    }

    constexpr uint64_t bar_pattern = 0x7C7C7C7C7C7C7C7Cull;
    constexpr uint64_t newline_pattern = 0x0A0A0A0A0A0A0A0Aull;

    size_t i = 0;
    int64_t bar_cnt = 0;
    // for each line
    while (i < partition_size_hint) {
        auto pos = find_first(bar_pattern, partition_start + i,
                              partition_size_hint - i);
        bar_cnt += (pos >= 0);
        if (bar_cnt == 5) {
            auto bar_pos = i + pos + 1;
            auto len = find_first(bar_pattern, partition_start + bar_pos,
                                  partition_size_hint - bar_pos);
            assert(len >= 1);
            int64_t extendedprice =
                read_numeric(partition_start + bar_pos, len);
            sum += extendedprice;
            bar_cnt = 0;

            // skip to end of line
            i = bar_pos + len + 1;
            auto newline_pos = find_first(newline_pattern, partition_start + i,
                                          partition_size_hint - i);
            if (newline_pos < 0 || (newline_pos + i) > partition_size_hint) {
                // undo
                sum -= extendedprice;
                break;
            }
            i += newline_pos + 1;
        } else {
            // Check if we found a bar pattern (pos != -1) and increment the position. Otherwise, just increment by 1.
            ++i += std::max<ssize_t>(pos, 0);
        }
    }

    *result = sum;
}
