### bitwise mapping technique
Mapping the program virtual address to the address specified as data.    
Pointer to the file will read 8 Byte per operation. ( 64bit CPU )   

high : 0x8080808080808080 is used to find the position of the matched pattern.     
low : 0x7F7F7F7F7F7F7F7F is to set the highest bit for not mathced characters.  
(~block)&high : to filter out non-ascii charaters ( ascii characters range from 0 to 127, 0111 1111 ).  
(block&low)^pattern : sets the matched bit 0 ( 0000 0000 ), while considering that the block is all ascii code.  
((block&low)^pattern) + low : sets the high bit for not matched characters.  
(((block&low)^pattern) + low) & high : check if the matched characters are from ascii characters.  

uint64_t pos = __builtin_ctzll(match) / 8; : it works for the little endian architectures.  
Ex) a b c  ( hex : 61 62 63, binary 0110 0001 / 0110 0002 / 0110 0003 )     
memory order : low memory - LSB c b a MSB - high memory   
it comes as like this in binary : 0110 0003 / 0110 0002 / 0110 0001.  
Thus uint64_t pos = __builtin_ctzll(match) / 8; gives the position from the beginning of each byte of the line read instead of from the end.  


### Build
A configuration file is provided to build this project with CMake.
This allows you to build the project in the terminal but also
provides the option to use Jetbrains CLion or Microsoft Visual Studio
and other IDEs.

Building from Terminal:
Start in the project directory.
```
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
```

This creates the binaries test_all and main.

Make sure your builds are not failing! <br/>
*Left Sidebar > CI /CD > Pipelines*
