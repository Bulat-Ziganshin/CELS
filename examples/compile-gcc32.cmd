@set lib=../lib
gcc -m32 -O3 -I%lib% %lib%/CELS.c simple_host.cpp -o simple_host.exe
gcc -m32 -O3 -I%lib% -DCELS_REGISTER_CODECS %lib%/CELS.c simple_host.cpp easy_codec.cpp -o simple_host_with_easy_codec.exe
gcc -m32 -c -O3 -I%lib% easy_codec.cpp
dllwrap -m32 --driver-name c++ easy_codec.o -def %lib%/CELS.def -s -o cels-test.dll
@del *.o
