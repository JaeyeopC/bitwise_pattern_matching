#include <fstream>
#include <iostream>
#include <string>
#include <charconv>

using namespace std;

int main(int argc, char* argv[]) {
   if (argc!=2) return 1;

   ifstream in(argv[1]);
   unsigned sum=0;
   string line;
   while (getline(in, line)) {
      unsigned column=0;
      for (auto iter=line.data(), limit=iter+line.size(),last=iter;;++iter) {
         if ((*iter)=='|') {
            if ((++column)==5) {
               unsigned v;
               from_chars(last,iter,v);
               sum+=v;
               break;
            } else {
               last=iter+1;
            }
         }
      }
   }
   cout << sum << endl;
}
