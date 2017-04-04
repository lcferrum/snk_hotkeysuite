#include <iostream>
#include <cstddef>
#include <cstdio>
#include <conio.h>
#include <windows.h>
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"
#include "SuiteHotkeyFunctions.h"

pTaskDialog fnTaskDialog=NULL;
pSHGetFolderPath fnSHGetFolderPathShfolder=NULL;
pSHGetFolderPath fnSHGetFolderPathShell32=NULL;
pSHGetSpecialFolderPath fnSHGetSpecialFolderPath=NULL;

unsigned long long cycles=0;
WPARAM wParam=WM_KEYUP;
bool lng=false;
KBDLLHOOKSTRUCT kb_eventW={0x57, 0x11};
KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
KBDLLHOOKSTRUCT *kb_event=&kb_eventW;

wchar_t np[]=L"notepad.exe"; 
wchar_t cl[]=L"calc.exe"; 

SuiteSettings settings;
KeyTriplet OnKeyTriplet(np, cl);
KeyTriplet *ptOnKeyTriplet=&OnKeyTriplet;
bool (__cdecl *OnEventHandler)(KeyTriplet*, WPARAM, KBDLLHOOKSTRUCT*);

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
	//Complete exe packed w/ UPX 3.93w: 254464 bytes
	
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
	//57% smaller!
	//Complete exe packed w/ UPX 3.93w: 253952 bytes
	//0.01% smaller...
	
	if (lng) {
		if (i%100) wParam=WM_KEYDOWN;
			else wParam=WM_KEYUP;
	} else {
		if (wParam==WM_KEYUP) wParam=WM_KEYDOWN;
			else wParam=WM_KEYUP;
	}
	
	unsigned int a1, d1, a2, d2; 
	
	asm volatile("xor %%eax, %%eax; rdtsc; movl %%eax, %%ebx; xor %%eax, %%eax":"=b"(a1), "=d"(d1)); 
	
	OnEventHandler(ptOnKeyTriplet, wParam, kb_event);

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
	
	OnEventHandler=OnKeyTriplet.CreateEventHandler(&settings);
	
	std::wcout<<L"SHEIZE"<<std::endl;
	
	OnEventHandler(ptOnKeyTriplet, WM_KEYDOWN, &kb_eventCTRL);
	OnEventHandler(ptOnKeyTriplet, WM_KEYDOWN, &kb_eventALT);
	OnEventHandler(ptOnKeyTriplet, WM_KEYDOWN, &kb_eventBS);
	OnEventHandler(ptOnKeyTriplet, WM_KEYUP, &kb_eventBS);
	OnEventHandler(ptOnKeyTriplet, WM_KEYUP, &kb_eventALT);
	OnEventHandler(ptOnKeyTriplet, WM_KEYUP, &kb_eventCTRL);
}

void Test3()
{
	KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
	KBDLLHOOKSTRUCT kb_eventBS2={0x08, 0x0E, 0x00, 3000};
	KBDLLHOOKSTRUCT kb_eventCTRL={0xFFFFFFFF, 0x1D};
	KBDLLHOOKSTRUCT kb_eventALT={0xFFFFFFFF, 0x38};
	
	/*OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventCTRL);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventBS2);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventCTRL);*/
}

void Test4()
{
	KBDLLHOOKSTRUCT kb_eventBS={0x08, 0x0E};
	KBDLLHOOKSTRUCT kb_eventBS2={0x08, 0x0E, 0x00, 1000};
	KBDLLHOOKSTRUCT kb_eventCTRL={0xFFFFFFFF, 0x1D};
	KBDLLHOOKSTRUCT kb_eventALT={0xFFFFFFFF, 0x38};
	
	/*OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventCTRL);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYDOWN, &kb_eventBS);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventBS2);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventALT);
	OnKeyTriplet2.LongPressCtrlAltEventHandler(WM_KEYUP, &kb_eventCTRL);*/
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
	
	OnEventHandler=OnKeyTriplet.CreateEventHandler(&settings);
	
	for (int i=1; i<=1000; i++)
		Test1(i);
	
	std::wcout<<L"RESULT = "<<cycles/1000<<std::endl;
	std::cerr<<cycles/1000<<";";
	
	return 0;
}

