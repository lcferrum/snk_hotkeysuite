#include "SnkExternalRelations.h"

namespace SnkHkFn {
	DWORD hk_state=0;
	DWORD hk_down_tick=0;
	bool hk_up=true;
	bool OnTargetKey(DWORD vk, DWORD hk_target_vk, bool key_up);
}

bool SnkHkFn::OnCtrlAlt(DWORD vk, bool key_up)
{
	switch (vk) {
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_CONTROL:
			//Ctrl
			if (key_up)	hk_state&=~0x1;
				else hk_state|=0x1;
			return true;
		case VK_LMENU:
		case VK_RMENU:
		case VK_MENU:
			//Alt
			if (key_up)	hk_state&=~0x2;
				else hk_state|=0x2;
			return true;			
		default:
			return false;
	}
}

bool SnkHkFn::OnCtrlShift(DWORD vk, bool key_up)
{
	switch (vk) {
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_CONTROL:
			//Ctrl
			if (key_up)	hk_state&=~0x1;
				else hk_state|=0x1;
			return true;
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_SHIFT:
			//Shift
			if (key_up)	hk_state&=~0x2;
				else hk_state|=0x2;
			return true;			
		default:
			return false;
	}
}

bool SnkHkFn::OnShiftAlt(DWORD vk, bool key_up)
{
	switch (vk) {
		case VK_LMENU:
		case VK_RMENU:
		case VK_MENU:
			//Alt
			if (key_up)	hk_state&=~0x1;
				else hk_state|=0x1;
			return true;		
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_SHIFT:
			//Shift
			if (key_up)	hk_state&=~0x2;
				else hk_state|=0x2;
			return true;			
		default:
			return false;
	}
}

bool SnkHkFn::OnTargetKey(DWORD vk, DWORD hk_target_vk, bool key_up)
{
	if (vk==hk_target_vk) {
		if (key_up)	hk_state&=~0x4;
			else hk_state|=0x4;
		return true;
	} else
		return false;
}

bool SnkHkFn::OnTriplet(std::function<bool(DWORD vk, bool key_up)> OnModKey, DWORD hk_target_vk, bool hk_allow_lp, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
{
		bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
		
		if (OnModKey(kb_event->vkCode, key_up)||OnTargetKey(kb_event->vkCode, hk_target_vk, key_up)) {
			if (key_up) {
				if (!hk_up&&hk_allow_lp) {
					DWORD cur_tick=GetTickCount();
					if (cur_tick>=hk_down_tick) {	//Excludes moment of system timer wrap around after 49.7 days of uptime
						if (cur_tick-hk_down_tick>3000)
							MessageBeep(MB_ICONERROR);	//Long hotkey press (>3 secs)
						else
							MessageBeep(MB_ICONINFORMATION);	//Ordinary hotkey press
					}
				}
				hk_up=true;
			} else if (hk_state==0x7) {
				if (hk_up) {
					hk_up=false;
					if (hk_allow_lp)
						hk_down_tick=GetTickCount();
					else
						MessageBeep(MB_ICONINFORMATION);	//Ordinary hotkey press
				}
				return true;
			}
		}
		
		return false; 
}
