#include <charconv>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <thread>
#include <mutex>
#include <vector>

using namespace std;

class MemoryMappedFile {
   int handle;
   void* mapping;
   size_t size;

   public:
   explicit MemoryMappedFile(const char* fileName) {
      handle = ::open(fileName, O_RDONLY);
      lseek(handle, 0, SEEK_END);
      size = lseek(handle, 0, SEEK_CUR);
      mapping=mmap(nullptr, size, PROT_READ, MAP_SHARED, handle, 0);
   }

   ~MemoryMappedFile() {
      munmap(mapping, size);
      close(handle);
   }

   const char* begin() const { return static_cast<char*>(mapping); }
   const char* end() const { return static_cast<char*>(mapping) + size; }
};


static constexpr uint64_t buildPattern(char c) {
  // Convert 00000000000000CC -> CCCCCCCCCCCCCCCC
  uint64_t v = c;
  return (v << 56 | v << 48 | v << 40 | v << 32 | v << 24 | v << 16 | v << 8) |
         v;
}

template <char separator>
static const char *findPattern(const char *iter, const char *end)
// Returns the position after the pattern within [iter, end[, or end if not
// found
{
   /*****************************************************/
   // Potential Improvements : using SIMD instructions ( Machine Sepcific )   
   // 
   // 1. For x86 machine                                 
   //    : Reading 32 bytes with one instruction instead of 8bytes
   //     auto limit32 = limit - 32;
   //     auto block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(iter)); 
   //     auto pattern = _mm256_set1_epi8 ('\n');
   //     auto match = _mm256_movemask_epi8(_mm256_cmpeq_epi8(block, pattern));    // "1"000 0000 "0"123 4567 "1"234 5678 -> 101
   //
   // 2. For Arm machine ( Not worked on my machine. Need some researches )
   //    : #include <arm_neon.h> 
   //     auto block = vld1q_u8(reinterpret_cast<const uint8_t*>(iter));
   //     auto pattern = vmovq_n_u8('\n');
   //     uint8x16_t result = vceqq_u8(block, pattern);
   //     uint64x2_t paired = vpaddlq_u32(vreinterpretq_u32_u8(result));
   //     auto hits = vgetq_lane_u64(paired, 0) | vgetq_lane_u64(paired, 1);
   //  
   /*****************************************************/
  // Loop over the content in blocks of 8 characters
  auto end8 = end - 8;
  constexpr uint64_t pattern = buildPattern(separator);
  for (; iter < end8; iter += 8) {
    // Check the next 8 characters for the pattern
    uint64_t block = *reinterpret_cast<const uint64_t *>(iter);
    constexpr uint64_t high = 0x8080808080808080ull;
    constexpr uint64_t low = ~high;
    uint64_t lowChars = (~block) & high;
    uint64_t foundPattern = ~((((block & low) ^ pattern) + low) & high);
    uint64_t matches = foundPattern & lowChars;
    if (matches)
      return iter + (__builtin_ctzll(matches) >> 3) + 1;
  }

  // Check the last few characters explicitly
  while ((iter < end) && ((*iter) != separator))
    ++iter;
  if (iter < end)
    ++iter;
  return iter;
}

template <char separator>
static const char *findNthPattern(const char *iter, const char *end, unsigned n)
// Returns the position after the pattern within [iter, end[, or end if not
// found
{
  // Loop over the content in blocks of 8 characters
  auto end8 = end - 8;
  constexpr uint64_t pattern = buildPattern(separator);
  for (; iter < end8; iter += 8) {
    // Check the next 8 characters for the pattern
    uint64_t block = *reinterpret_cast<const uint64_t *>(iter);
    constexpr uint64_t high = 0x8080808080808080ull;
    constexpr uint64_t low = ~high;
    uint64_t lowChars = (~block) & high;
    uint64_t foundPattern = ~((((block & low) ^ pattern) + low) & high);
    uint64_t matches = foundPattern & lowChars;
    if (matches) {
      unsigned hits = __builtin_popcountll(matches);
      if (hits >= n) {
        for (; n > 1; n--)
          matches &= matches - 1;
        return iter + (__builtin_ctzll(matches) >> 3) + 1;
      }
      n -= hits;
    }
  }

  // Check the last few characters explicitly
  for (; iter < end; ++iter)
    if ((*iter) == separator)
      if ((--n) == 0)
        return iter + 1;

  return end;
}

static unsigned computeSum(const char* begin, const char* end){
   unsigned sum = 0;
  for (auto iter = begin; iter < end;) {
    auto last = findNthPattern<'|'>(iter, end, 4);
    iter = last;
    unsigned v = 0;
    while (iter < end) {
      char c = *(iter++);
      if ((c >= '0') && (c <= '9'))
        v = 10 * v + c - '0';
      else
        break;
    }
    sum += v;
    iter = findPattern<'\n'>(iter, end);
  }

  return sum;
}

static const char* getBoundary(const char* begin, const char* end, unsigned numberOfThreads, unsigned idx){
   if(!idx) return begin;
   if(idx >= numberOfThreads) return end;
   auto size = end - begin;
   return findPattern<'\n'>(begin + (size * idx)/numberOfThreads, end );
}

static unsigned threadComputeSum(MemoryMappedFile in){
   atomic<unsigned> sum = 0;
   unsigned threadCount = thread::hardware_concurrency();
   vector<thread> threads;

   mutex m;

   for(unsigned index = 0 ; index != threadCount ; ++index){
      threads.emplace_back(
         [&sum, &in, index, threadCount, &m]()
         {
            auto from = getBoundary(in.begin(), in.end(), threadCount, index);
            auto to = getBoundary(in.begin(), in.end(), threadCount, index+1);
            unsigned localSum = computeSum(from, to);
            // without mutex
            // when several threads attemp to write to sum, the final value of sum can be overwritten,
            // thus not the correct value ( ex : sum = 100, t1 = 20, t2 = 30, sum = 120? or 130? ) 
            {
               unique_lock lock(m);
               sum+=localSum;
            }  
         }
      );
   }

   for(auto& t:threads) t.join();
   sum.load();

   return sum;
}


int main(int argc, char *argv[]) {
  if (argc != 2)
    return 1;

   MemoryMappedFile in(argv[1]);
   unsigned sum = 0;
   sum += computeSum(in.begin(), in.end());
   std::cout << "No Thread Sum : " << sum << std::endl;
   std::cout << "Thread Sum : " << threadComputeSum(in) << std::endl;
}
