@set lib=../../lib
gcc -c -O3 -I%lib% cels-lz4.cpp
dllwrap --driver-name c++ cels-lz4.o -def %lib%/CELS.def -s -o cels-lz4.dll
@del *.o
