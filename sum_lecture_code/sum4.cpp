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
   auto limit16 = limit - 16;
   while (iter < limit16) {
      auto block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(iter));
      auto pattern = _mm_set1_epi8 ('\n');
      auto hits = _mm_movemask_epi8(_mm_cmpeq_epi8(block, pattern));

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

int main(int argc, char* argv[]) {
   if (argc!=2) return 1;

   MemoryMappedFile in(argv[1]);
   unsigned sum=0;
   for (auto iter = in.begin(), limit = in.end();iter<limit;) {
      unsigned column=0;
      for (auto last=iter;;++iter) {
         if ((*iter)=='|') {
            if ((++column)==5) {
               unsigned v;
               from_chars(last,iter,v);
               sum+=v;
               iter = findNextLine(iter, limit);
               break;
            } else {
               last=iter+1;
            }
         }
      }
   }
   cout << sum << endl;
}
