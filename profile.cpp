#include <iostream>
#include <cstddef>
#include <cstdio>
#include <windows.h>
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"

pTaskDialog fnTaskDialog=NULL;

unsigned long long cycles=0;
WPARAM wParam=WM_KEYUP;
bool lng=false;
KBDLLHOOKSTRUCT kb_eventW={0x57, 0x11};
KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
KBDLLHOOKSTRUCT *kb_event=&kb_eventW;

class KeyTriplet2 {
	typedef bool (KeyTriplet2::*EventHandlerFn)(WPARAM, KBDLLHOOKSTRUCT*);
private:
	DWORD hk_sc;
	DWORD hk_ext;
	DWORD hk_state;
	DWORD hk_engaged;
	DWORD hk_down_tick;
	wchar_t* hk_cmdline_s;
	wchar_t* hk_cmdline_l;
	//Creating reusable structs needed for CreateProcess here to save few cycles in event handler
	PROCESS_INFORMATION hk_pi;
	STARTUPINFO hk_si;
	//Above is a struct that assembler part depends on
	//If it is modified, assembler code should be modified accordingly
	EventHandlerFn hk_event_handler;
public:
	KeyTriplet2(wchar_t* cmdline_s, wchar_t* cmdline_l): 
		hk_sc(DEFAULT_SC), hk_ext(DEFAULT_EXT), hk_state(0), hk_engaged(0), hk_down_tick(0), hk_cmdline_s(cmdline_s), hk_cmdline_l(cmdline_l), hk_si{sizeof(STARTUPINFO)}, hk_event_handler(SinglePressCtrlAltEventHandler) {}
	void ResetEventHandler() { hk_state=0; hk_engaged=0; hk_down_tick=0; }
	EventHandlerFn LastEventHandler() { 
		ResetEventHandler();
		return hk_event_handler; 
	}
	EventHandlerFn CreateEventHandler(SuiteSettings *settings) {
		ResetEventHandler();
		hk_sc=settings->GetBindedKey().sc;
		hk_ext=settings->GetBindedKey().ext;	//By C++ standard it is guaranteed that true will be converted to 1 and false to 0
		switch (settings->GetModKey()) {
			case ModKeyType::CTRL_ALT:
				hk_event_handler=settings->GetLongPress()?&LongPressCtrlAltEventHandler:&SinglePressCtrlAltEventHandler;
				break;
			case ModKeyType::SHIFT_ALT:
				hk_event_handler=settings->GetLongPress()?&LongPressShiftAltEventHandler:&SinglePressShiftAltEventHandler;
				break;
			case ModKeyType::CTRL_SHIFT:
				hk_event_handler=settings->GetLongPress()?&LongPressCtrlShiftEventHandler:&SinglePressCtrlShiftEventHandler;
				break;
		}
		return hk_event_handler; 
	}
	
#ifndef _WIN64
	bool __cdecl SinglePressCtrlAltEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_SinglePressCtrlAltEventHandler");
	bool __cdecl SinglePressShiftAltEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_SinglePressShiftAltEventHandler");
	bool __cdecl SinglePressCtrlShiftEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_SinglePressCtrlShiftEventHandler");
	bool __cdecl LongPressCtrlAltEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_LongPressCtrlAltEventHandler");
	bool __cdecl LongPressShiftAltEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_LongPressShiftAltEventHandler");
	bool __cdecl LongPressCtrlShiftEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event) asm("_LongPressCtrlShiftEventHandler");
#endif
};

wchar_t np[]=L"notepad.exe"; 
wchar_t cl[]=L"calc.exe"; 

KeyTriplet2 OnKeyTriplet2(np, cl);
bool (KeyTriplet2::*OnEventHandler)(WPARAM, KBDLLHOOKSTRUCT*);

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
	
	//6.84 KB
	//Complete exe packed w/ UPX 3.93w: ?
	
	//ASM version:
	
	//LongPress disabled x86
	//A cycles = ~26
	//B cycles = ~35
	//C cycles = ~28
	//D cycles = ~34
	//Maximum: 0.017us per iteration (@2GHz)
	//15 times faster!
	
	//LongPress enabled x86
	//A cycles = ~26
	//B cycles = ~36
	//C cycles = ~28
	//D cycles = ~35
	//Maximum: 0.018us per iteration (@2GHz)
	//14 times faster!
	
	//2.99 KB
	//43% smaller!
	//Complete exe packed w/ UPX 3.93w: 253952 bytes
	
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
	//OnKeyTriplet2.SinglePressCtrlAltEventHandler(wParam, kb_event);
	//OnKeyTriplet2.LongPressCtrlAltEventHandler(wParam, kb_event);
	(OnKeyTriplet2.*OnEventHandler)(wParam, kb_event);

	asm volatile("xor %%eax, %%eax; rdtsc; movl %%eax, %%ebx; xor %%eax, %%eax":"=b"(a2), "=d"(d2)); 
	
	unsigned long long res1=((unsigned long long)a1)|(((unsigned long long)d1)<<32);
	unsigned long long res2=((unsigned long long)a2)|(((unsigned long long)d2)<<32);
	cycles+=res2-res1;
}

void Test2()
{
	KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
	KBDLLHOOKSTRUCT kb_eventCTRL={0xFFFFFFFF, 0x1D};
	KBDLLHOOKSTRUCT kb_eventALT={0xFFFFFFFF, 0x38};
	
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventCTRL);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventALT);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYUP, &kb_eventBS);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYUP, &kb_eventALT);
	OnKeyTriplet2.SinglePressCtrlAltEventHandler(WM_KEYUP, &kb_eventCTRL);
}

void Test3()
{
	KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
	KBDLLHOOKSTRUCT kb_eventBS2={0x08, 0x0E, 0x00, 3000};
	KBDLLHOOKSTRUCT kb_eventCTRL={0xFFFFFFFF, 0x1D};
	KBDLLHOOKSTRUCT kb_eventALT={0xFFFFFFFF, 0x38};
	
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventCTRL);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventBS2);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventCTRL);
}

void Test4()
{
	KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
	KBDLLHOOKSTRUCT kb_eventBS2={0x08, 0x0E, 0x00, 1000};
	KBDLLHOOKSTRUCT kb_eventCTRL={0xFFFFFFFF, 0x1D};
	KBDLLHOOKSTRUCT kb_eventALT={0xFFFFFFFF, 0x38};
	
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventCTRL);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventBS2);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventCTRL);
}

int main(int argc, char* argv[])
{
	if (argc<=1) {
		std::wcout<<L"FUCK YOU"<<std::endl;
		return 0;
	}
	
	SetProcessAffinityMask(GetCurrentProcess(), (1<<0));
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	
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
	} else if (!strcmp(argv[1], "e")) {
		std::wcout<<L"TEST E"<<std::endl;
		Test2();
		return 0;
	} else if (!strcmp(argv[1], "f")) {
		std::wcout<<L"TEST F"<<std::endl;
		Test3();
		return 0;
	} else if (!strcmp(argv[1], "g")) {
		std::wcout<<L"TEST G"<<std::endl;
		Test4();
		return 0;
	} else {
		std::wcout<<L"TEST A"<<std::endl;
	}
	
	std::wstring none;
	
	OnEventHandler=OnKeyTriplet2.LastEventHandler();
	
	for (int i=1; i<=1000; i++)
		Test1(i);
	
	std::wcout<<L"RESULT = "<<cycles/1000<<std::endl;
	std::cerr<<cycles/1000<<";";
	
	return 0;
}

