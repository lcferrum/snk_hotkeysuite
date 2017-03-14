#include <iostream>
#include <cstdio>
#include <windows.h>
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include "SuiteHotkeyFunctions.h"

pTaskDialog fnTaskDialog=NULL;

unsigned long long cycles=0;
KeyTriplet OnKeyTriplet;
WPARAM wParam=WM_KEYUP;
bool lng=false;
KBDLLHOOKSTRUCT kb_eventW={0x57, 0x11};
KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
KBDLLHOOKSTRUCT *kb_event=&kb_eventW;

class KeyTriplet2 {
private:
	DWORD hk_sc;
	DWORD hk_ext;
	DWORD hk_state;
	DWORD hk_engaged;
	DWORD hk_down_tick;
public:
	KeyTriplet2(): hk_sc(DEFAULT_SC), hk_ext(DEFAULT_EXT), hk_state(0), hk_engaged(0), hk_down_tick(0) {}
	void SetBindedKey(BINDED_KEY new_key_binding) { hk_sc=new_key_binding.sc; hk_ext=new_key_binding.ext; }
	void ResetEventHandler() { hk_state=0; hk_engaged=0; hk_down_tick=0; }

#ifndef _WIN64
	bool __stdcall SinglePressCtrlAltEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_SinglePressCtrlAltEventHandler@12");
	bool __stdcall SinglePressShiftAltEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_SinglePressShiftAltEventHandler@12");
	bool __stdcall SinglePressCtrlShiftEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_SinglePressCtrlShiftEventHandler@12");
#endif
};

KeyTriplet2 OnKeyTriplet2;

void HotkeyEventHandler(wchar_t* snk_cmdline_buf) {
	STARTUPINFO si={sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi={};
	if (CreateProcess(NULL, snk_cmdline_buf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

void Test1(int i)
{
	//LongPress disabled x86
	//A cycles = ~456
	//B cycles = ~465
	//C cycles = ~502
	//D cycles = ~469
	//Maximum: 0.251us per iteration (@2GHz)
	
	//LongPress enabled x86
	//A cycles = ~456
	//B cycles = ~460
	//C cycles = ~507
	//D cycles = ~473
	//Maximum: 0.254us per iteration (@2GHz)
	
	//Estimated LongPress disabled x86 with ASM = ~39
	//13 times faster!
	
	if (lng) {
		if (i%100) wParam=WM_KEYDOWN;
			else wParam=WM_KEYUP;
	} else {
		if (wParam==WM_KEYUP) wParam=WM_KEYDOWN;
			else wParam=WM_KEYUP;
	}
	
	unsigned int a1, d1, a2, d2; 
	
	asm volatile("xor %%eax, %%eax; rdtsc; movl %%eax, %%ebx; xor %%eax, %%eax":"=b"(a1), "=d"(d1)); 
	
	//OnKeyTriplet.KeyPressEventHandler(wParam, kb_event);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(wParam, kb_event);

	asm volatile("xor %%eax, %%eax; rdtsc; movl %%eax, %%ebx; xor %%eax, %%eax":"=b"(a2), "=d"(d2)); 
	
	unsigned long long res1=((unsigned long long)a1)|(((unsigned long long)d1)<<32);
	unsigned long long res2=((unsigned long long)a2)|(((unsigned long long)d2)<<32);
	cycles+=res2-res1;
}

int main(int argc, char* argv[])
{
	if (argc<=1) {
		std::wcout<<L"FUCK YOU"<<std::endl;
		return 0;
	}
	
	if (!strcmp(argv[1], "b")) {
		std::wcout<<L"TEST B"<<std::endl;
		lng=true;
	} else if (!strcmp(argv[1], "c")) {
		std::wcout<<L"TEST C"<<std::endl;
		kb_event=&kb_eventBS;
	} else if (!strcmp(argv[1], "d")) {
		std::wcout<<L"TEST D"<<std::endl;
		kb_event=&kb_eventBS;
		lng=true;
	} else {
		std::wcout<<L"TEST A"<<std::endl;
	}
	
	std::wstring none;
	
	SetProcessAffinityMask(GetCurrentProcess(), (1<<0));
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	
	OnKeyTriplet.SetOnShortHotkey(std::bind(&HotkeyEventHandler, const_cast<wchar_t*>(none.c_str())));
	OnKeyTriplet.SetOnLongHotkey(std::bind(&HotkeyEventHandler, const_cast<wchar_t*>(none.c_str())));
	OnKeyTriplet.SetLongPress(true);
	
	for (int i=1; i<=1000; i++)
		Test1(i);
	
	std::wcout<<L"RESULT = "<<cycles/1000<<std::endl;
	std::cerr<<cycles/1000<<";";
	
	return 0;
}

