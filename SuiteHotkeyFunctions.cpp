#include "SuiteHotkeyFunctions.h"

#ifdef DEBUG
#include <iostream>
#endif

#define FLAG_HK_1ST	(1<<0)
#define FLAG_HK_2ND	(1<<1)
#define FLAG_HK_3RD	(1<<2)
#define FLAG_HK_ALL	(FLAG_HK_1ST|FLAG_HK_2ND|FLAG_HK_3RD)
#define LONG_PRESS_DURATION 3000	//3 sec (keep in mind that this define is compared with GetTickCount() that can have resolution up to 55ms)

KeyTriplet::KeyTriplet():
	OnModKey(std::bind(&KeyTriplet::OnCtrlAlt, this, std::placeholders::_1, std::placeholders::_2)), 
	hk_binded_vk(VK_BACK), hk_binded_sc(0x0E), hk_long_press(false), hk_state(0), hk_down_tick(0), hk_up(true)
{}

bool KeyTriplet::OnCtrlAlt(DWORD vk, bool key_up)
{
	switch (vk) {
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_CONTROL:
			//Ctrl
			if (key_up)	hk_state&=~FLAG_HK_1ST;
				else hk_state|=FLAG_HK_1ST;
			return true;
		case VK_LMENU:
		case VK_RMENU:
		case VK_MENU:
			//Alt
			if (key_up)	hk_state&=~FLAG_HK_2ND;
				else hk_state|=FLAG_HK_2ND;
			return true;			
		default:
			return false;
	}
}

bool KeyTriplet::OnCtrlShift(DWORD vk, bool key_up)
{
	switch (vk) {
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_CONTROL:
			//Ctrl
			if (key_up)	hk_state&=~FLAG_HK_1ST;
				else hk_state|=FLAG_HK_1ST;
			return true;
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_SHIFT:
			//Shift
			if (key_up)	hk_state&=~FLAG_HK_2ND;
				else hk_state|=FLAG_HK_2ND;
			return true;			
		default:
			return false;
	}
}

bool KeyTriplet::OnShiftAlt(DWORD vk, bool key_up)
{
	switch (vk) {
		case VK_LMENU:
		case VK_RMENU:
		case VK_MENU:
			//Alt
			if (key_up)	hk_state&=~FLAG_HK_1ST;
				else hk_state|=FLAG_HK_1ST;
			return true;		
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_SHIFT:
			//Shift
			if (key_up)	hk_state&=~FLAG_HK_2ND;
				else hk_state|=FLAG_HK_2ND;
			return true;			
		default:
			return false;
	}
}

bool KeyTriplet::OnTargetKey(DWORD vk, DWORD sc, bool key_up)
{
	//For legacy reasons Break, Pause, PrtScn and Num[/] have scancodes shared respectively with ScrLock, NumLock, Num[*] and [?/]
	//So for these keys we should also compare virtual keys to distinguish them from one another
	
	if (sc==hk_binded_sc) {
		switch (sc) {
			case 0x46:	//Break/ScrLock
			case 0x45:	//Pause/NumLock
			case 0x37:	//PrtScn/Num[*]
				if (vk!=hk_binded_vk)
					return false;
				break;
			case 0x35:	//Num[/]/[?/]
				//For Num[/]/[?/] it's a bit special
				//Num[/] always have VK=VK_DIVIDE and SC=0x35
				//[?/]'s SC is always 0x35 but VK is one of the OEM's VKs depending on currently selected layout
				if ((hk_binded_vk==VK_DIVIDE||vk==VK_DIVIDE)&&vk!=hk_binded_vk)
					return false;
				break;
		}
		//Don't forget that Ctrl+Pause=Break, Ctrl+ScrLock=Break, Ctrl+NumLock=Pause and Alt+PrtScn=SysRq
		//So hotkeys like Ctrl+Alt+Pause won't work because Ctrl+Alt+Break will be generated instead
		
		if (key_up)	hk_state&=~FLAG_HK_3RD;
			else hk_state|=FLAG_HK_3RD;

		return true;
	} else
		return false;
}

bool KeyTriplet::OnKeyPress(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
{
	bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
	
#ifdef DEBUG
	std::wcerr<<std::hex<<(wParam==WM_KEYDOWN||wParam==WM_SYSKEYDOWN?L"KEYDOWN":L"KEYUP")<<L" VK: "<<kb_event->vkCode<<L" SC: "<<kb_event->scanCode<<std::endl;
#endif
	
	if (OnModKey(kb_event->vkCode, key_up)||OnTargetKey(kb_event->vkCode, kb_event->scanCode, key_up)) {
		if (key_up) {
			if (!hk_up&&hk_long_press) {
				DWORD cur_tick=GetTickCount();
				if (cur_tick>=hk_down_tick) {	//Excludes moment of system timer wrap around after 49.7 days of uptime
					if (cur_tick-hk_down_tick>LONG_PRESS_DURATION) {
						MessageBeep(MB_ICONERROR);	//Long hotkey press (>LONG_PRESS_DURATION msec)
#ifdef DEBUG
						std::wcerr<<L"LONG PRESS KEYUP HOTKEY ENGAGED"<<std::endl;
#endif
					} else {
						MessageBeep(MB_ICONINFORMATION);	//Ordinary hotkey press
#ifdef DEBUG
						std::wcerr<<L"SINGLE PRESS KEYUP HOTKEY ENGAGED"<<std::endl;
#endif
					}
				}
			}
			hk_up=true;
		} else if (hk_state==FLAG_HK_ALL) {
			if (hk_up) {
				hk_up=false;
				if (hk_long_press)
					hk_down_tick=GetTickCount();
				else {
					MessageBeep(MB_ICONINFORMATION);	//Ordinary hotkey press
#ifdef DEBUG
					std::wcerr<<L"SINGLE PRESS KEYDOWN HOTKEY ENGAGED"<<std::endl;
#endif
				}
			}
			return true;
		}
	}
	
	return false; 
}

bool BindKey(HWND dlg_hwnd, UINT bind_wm, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
{
	if (wParam==WM_KEYDOWN||wParam==WM_SYSKEYDOWN) {
#ifdef DEBUG
		std::wcerr<<std::hex<<"KEYDOWN VK: "<<kb_event->vkCode<<L" SC: "<<kb_event->scanCode<<std::endl;
#endif
		//Ignoring Ctrl, Alt and Shift
		switch (kb_event->vkCode) {
			case VK_LMENU:
			case VK_RMENU:
			case VK_MENU:
			case VK_LSHIFT:
			case VK_RSHIFT:
			case VK_SHIFT:
			case VK_LCONTROL:
			case VK_RCONTROL:
			case VK_CONTROL:
				break;
			default:
				PostMessage(dlg_hwnd, bind_wm, kb_event->vkCode, kb_event->scanCode);
				return true;
		}
	}
	return false;
}
