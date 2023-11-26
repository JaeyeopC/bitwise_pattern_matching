#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <string_view>

int64_t ToInt(std::string_view s) {
    int64_t result = 0;
    for (auto c : s) {
        // First check if we reached the decimal point
        if (c == '.') {
            break;
        }
        result = result * 10 + (c - '0');
    }
    return result;
}

// pattern : the character to search broadcasted into a 64-bit integer
// block : memory block in which to search
// return : 64-bit integer with all the matches as seen in the lecture
inline uint64_t get_matches(uint64_t pattern, uint64_t block) {
    //-- TODO exercise 3.4 part 2
    // your code goes here
    //--
    constexpr uint64_t high = 0x8080808080808080ull;
    constexpr uint64_t low = 0x7F7F7F7F7F7F7F7Full;
    uint64_t asciiPos = (~block)&high;
    uint64_t found0A = ~((((block&low)^pattern)+low)&high);
    uint64_t match = found0A & asciiPos;

    return match;
}

// pattern : the character to search broadcasted into a 64bit integer
// begin : points somewhere into the partition
// len : remaining length of the partition
// return : the position of the first matching character or -1 otherwise
ssize_t find_first(uint64_t pattern, const char* begin, size_t len) {
    // locate the position of the following character (you'll have to use the
    // pattern argument directly in the second part of the exercise)
    const char to_search = pattern & 0xff;

    // Hint: You may assume that reads from 'begin' within [len, len + 8) yield
    // zero
    //-- TODO exercise 3.4
    // your code goes here
    //--
    size_t i = 0;
    while(i < len)
    {
        uint64_t block = *reinterpret_cast<const uint64_t*>(begin+i);
        uint64_t match = get_matches(pattern , block);
        if(match != 0)
        {
            uint64_t pos = __builtin_ctzll(match) / 8;
            if(pos < 8){
                return (i + pos+1);
            }   
        }
        i += 8;

    }
    return -1;
}

// "already at the extended price part"` !!!
// begin : points somewhere into the partition
// len : remaining length of the partition
// return : 64-bit integer representation of the provided numeric
int64_t read_numeric(const char* begin, size_t len) {
    constexpr uint64_t period_pattern = 0x2E2E2E2E2E2E2E2Eull;
    std::string_view numeric_view(begin, len);
    ssize_t dot_position = find_first(period_pattern, begin, len);
    assert(dot_position > 0);
    auto part1 = numeric_view.substr(0, dot_position);
    auto part2 = numeric_view.substr(dot_position + 1);
    int64_t numeric = ToInt(part1) * 100 + ToInt(part2);
    return numeric;
}
