#include "SuiteHotkeyFunctions.h"

#define LONG_PRESS_DURATION 3000	//3 sec (keep in mind that this define is compared with GetTickCount() that can have resolution up to 55ms)

KeyTriplet::KeyTriplet():
	OnModKey(std::bind(&KeyTriplet::OnCtrlAlt, this, std::placeholders::_1, std::placeholders::_2)), 
	hk_binded_vk(VK_BACK), hk_long_press(false), hk_state(0), hk_down_tick(0), hk_up(true)
{}

bool KeyTriplet::OnCtrlAlt(DWORD vk, bool key_up)
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

bool KeyTriplet::OnCtrlShift(DWORD vk, bool key_up)
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

bool KeyTriplet::OnShiftAlt(DWORD vk, bool key_up)
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

bool KeyTriplet::OnTargetKey(DWORD vk, DWORD hk_binded_vk, bool key_up)
{
	if (vk==hk_binded_vk) {
		if (key_up)	hk_state&=~0x4;
			else hk_state|=0x4;
		return true;
	} else
		return false;
}

bool KeyTriplet::OnKeyPress(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
{
	bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
	
	if (OnModKey(kb_event->vkCode, key_up)||OnTargetKey(kb_event->vkCode, hk_binded_vk, key_up)) {
		if (key_up) {
			if (!hk_up&&hk_long_press) {
				DWORD cur_tick=GetTickCount();
				if (cur_tick>=hk_down_tick) {	//Excludes moment of system timer wrap around after 49.7 days of uptime
					if (cur_tick-hk_down_tick>LONG_PRESS_DURATION)
						MessageBeep(MB_ICONERROR);	//Long hotkey press (>LONG_PRESS_DURATION msec)
					else
						MessageBeep(MB_ICONINFORMATION);	//Ordinary hotkey press
				}
			}
			hk_up=true;
		} else if (hk_state==0x7) {
			if (hk_up) {
				hk_up=false;
				if (hk_long_press)
					hk_down_tick=GetTickCount();
				else
					MessageBeep(MB_ICONINFORMATION);	//Ordinary hotkey press
			}
			return true;
		}
	}
	
	return false; 
}

bool BindKey(HWND dlg_hwnd, UINT bind_wm, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
{
	if (wParam==WM_KEYDOWN||wParam==WM_SYSKEYDOWN)
		//Don't bind ENTER and SPACE because these keys could have been used just to close dialog
		//Also ignoring Ctrl, Alt and Shift
		switch (kb_event->vkCode) {
			case VK_SPACE:
			case VK_RETURN:
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
				PostMessage(dlg_hwnd, bind_wm, kb_event->vkCode, 0);
		}
	return false;
}
