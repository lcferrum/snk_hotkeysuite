#!/bin/sh
i686-w64-mingw32-g++ -std=c++11 -Wno-write-strings -Wl,--enable-stdcall-fixup -D_WIN32_WINNT=0x0600 -DNOMINMAX -DUNICODE -D_UNICODE -static-libgcc -static-libstdc++ -lole32 profile.cpp SuiteHotkeyFunctions.cpp SuiteHotkeyFunctions.S SuiteCommon.cpp -o profile.exe

rm -rf profile.csv

max=100
for i in `seq 1 $max`
do
    ./profile.exe a 2>> profile.csv
done

echo "" >> profile.csv

for i in `seq 1 $max`
do
    ./profile.exe b 2>> profile.csv
done

echo "" >> profile.csv

for i in `seq 1 $max`
do
    ./profile.exe c 2>> profile.csv
done

echo "" >> profile.csv

for i in `seq 1 $max`
do
    ./profile.exe d 2>> profile.csv
done

echo "" >> profile.csv
