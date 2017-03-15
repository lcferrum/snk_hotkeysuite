#!/bin/sh
i686-w64-mingw32-g++ -std=c++11 -Wno-write-strings -Wl,--enable-stdcall-fixup -D_WIN32_WINNT=0x0600 -DNOMINMAX -DUNICODE -D_UNICODE -static-libgcc -static-libstdc++ -lole32 profile.cpp SuiteHotkeyFunctions.cpp SuiteHotkeyFunctions.S SuiteCommon.cpp -o profile.exe
