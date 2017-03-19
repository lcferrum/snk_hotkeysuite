#ifndef SUITEHOTKEYFUNCTIONS_H
#define SUITEHOTKEYFUNCTIONS_H

#include "SuiteSettings.h"
#include <windows.h>

class KeyTriplet {
	typedef bool (__cdecl *EventHandlerFn)(KeyTriplet*, WPARAM, KBDLLHOOKSTRUCT*);
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
public:
	KeyTriplet(wchar_t* cmdline_s, wchar_t* cmdline_l);
	void ResetEventHandler();
	EventHandlerFn CreateEventHandler(SuiteSettings *settings);
};

bool BindKeyEventHandler(HWND dlg_hwnd, UINT bind_wm, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);

#ifdef DEBUG
bool DebugEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
#endif

#endif //SUITEHOTKEYFUNCTIONS_H
