@set lib=../lib
cl -O2 /EHsc -I%lib% -DCELS_REGISTER_CODECS %lib%/CELS.c simple_host.cpp easy_codec.cpp -o simple_host_with_easy_codec.exe
cl -O2 /EHsc -I%lib% easy_codec.cpp
link -DLL -DEF:%lib%/CELS.def -out:cls-test.dll easy_codec.obj
@del *.obj
