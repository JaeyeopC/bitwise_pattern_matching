#include <fstream>
#include <iostream>
#include <string>
#include <charconv>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <immintrin.h>
#include <emmintrin.h>

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

static consteval uint64_t broadcast(uint8_t c) {
   return (static_cast<uint64_t>(c) << 0) | (static_cast<uint64_t>(c) << 8) | (static_cast<uint64_t>(c) << 16) | (static_cast<uint64_t>(c) << 24) | (static_cast<uint64_t>(c) << 32) | (static_cast<uint64_t>(c) << 40) | (static_cast<uint64_t>(c) << 48) | (static_cast<uint64_t>(c) << 56);
}

static const char* findNextLine(const char* iter, const char* limit) {
   auto limit32 = limit - 32;
   while (iter < limit32) {
      auto block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(iter));
      auto pattern = _mm256_set1_epi8 ('\n');
      auto hits = _mm256_movemask_epi8(_mm256_cmpeq_epi8(block, pattern));

      if (hits) {
         return iter + (__builtin_ctzll(hits));
      } else {
         iter+=8;
      }
   }

   while (iter<limit) {
      if ((*iter++)=='\n')
         break;
   }
   return iter;
}

static const char* findNthBar(const char* iter, const char* limit, unsigned nth) {
   auto limit32 = limit - 32;
   while (iter < limit32) {
      auto block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(iter));
      auto pattern = _mm256_set1_epi8 ('|');
      auto hits = _mm256_movemask_epi8(_mm256_cmpeq_epi8(block, pattern));

      if (hits) {
         unsigned count = __builtin_popcount(hits);
         if (count >= nth) {
            while (nth > 1) {
               hits = hits & (hits - 1);
               nth--;
            }
            return iter + (__builtin_ctzll(hits));
         } else {
            nth -= count;
         }
      } else {
         iter+=8;
      }
   }

   while (iter<limit) {
      if ((*iter++)=='|')
         if (!--nth)
            break;
   }
   return iter;
}

int main(int argc, char* argv[]) {
   if (argc!=2) return 1;

   MemoryMappedFile in(argv[1]);
   unsigned sum=0;
   for (auto iter = in.begin(), limit = in.end();iter<limit;) {
      auto last = findNthBar(iter, limit, 4) + 1;
      for (iter=last;*iter!='|';) ++iter;
      unsigned v;
      from_chars(last,iter,v);
      sum+=v;
      iter = findNextLine(iter, limit);
   }
   cout << sum << endl;
}
