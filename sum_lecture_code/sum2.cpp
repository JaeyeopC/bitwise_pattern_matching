#include <fstream>
#include <iostream>
#include <string>
#include <charconv>
#if 0
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
#include <bits/stdc++.h>

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
               while (iter<limit) {
                  if ((*iter++)=='\n')
                     break;
               }
               break;
            } else {
               last=iter+1;
            }
         }
      }
   }
   cout << sum << endl;
}
