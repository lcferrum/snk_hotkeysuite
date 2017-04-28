#ifndef SUITEHOTKEYFUNCTIONS_H
#define SUITEHOTKEYFUNCTIONS_H

#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include <windows.h>

class KeyTriplet {
	typedef bool (HKECALL *EventHandlerFn)(LPARAM, WPARAM, KBDLLHOOKSTRUCT*);
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
	EventHandlerFn CreateEventHandler(const SuiteSettings *settings);
};

bool HKECALL BindKeyEventHandler(LPARAM event_param, WPARAM kb_param, KBDLLHOOKSTRUCT* kb_event);

#ifdef DEBUG
bool HKECALL DebugEventHandler(LPARAM event_param, WPARAM kb_param, KBDLLHOOKSTRUCT* kb_event);
#endif

#endif //SUITEHOTKEYFUNCTIONS_H
