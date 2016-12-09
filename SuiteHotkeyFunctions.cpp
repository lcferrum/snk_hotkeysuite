#include "SuiteHotkeyFunctions.h"
#include "SuiteCommon.h"

#ifdef DEBUG
#include <iostream>
#endif

#define FLAG_HK_1ST	(1<<0)
#define FLAG_HK_2ND	(1<<1)
#define FLAG_HK_3RD	(1<<2)
#define FLAG_HK_ALL	(FLAG_HK_1ST|FLAG_HK_2ND|FLAG_HK_3RD)
#define LONG_PRESS_DURATION 3000	//3 sec (keep in mind that this define is compared with derivative of GetTickCount() that can have resolution up to 55ms)

#define FAKE_SC	0x200	//Undocumented windows fake scancode flag

KeyTriplet::KeyTriplet():
	OnModKey(std::bind(&KeyTriplet::OnCtrlAlt, this, std::placeholders::_1, std::placeholders::_2)), 
	hk_binded_key{DEFAULT_VK /* vk */, DEFAULT_SC /* sc */, DEFAULT_EXT /* ext */}, hk_long_press(false), hk_state(0), hk_down_tick(0), hk_up(true)
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

bool KeyTriplet::OnTargetKey(DWORD vk, DWORD sc, bool ext, bool key_up)
{
	if (sc==hk_binded_key.sc&&ext==hk_binded_key.ext) {
		if (key_up)	hk_state&=~FLAG_HK_3RD;
			else hk_state|=FLAG_HK_3RD;

		return true;
	} else
		return false;
}

bool KeyTriplet::OnKeyPress(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
{
	//Good paper on scancodes can be found here http://www.quadibloc.com/comp/scan.htm or in "Windows Platform Design Notes / Keyboard Scan Code Specification"
	//Over time IBM PC keyboards evolved, received new keys but maintained backward compatibility with software that expects some old as fuck 83-key keyboard
	//This backward compatibility is achieved by sending (on keyboard side) fake key sequences and using escape scancodes (E0, E1, etc.)
	//Also each key have two scancodes: "make" scancode to indicate KEYDOWN event and "break" scancode to indicate KEYUP event
	//Scancodes that are received in KBDLLHOOKSTRUCT with low level keyboard hook actually not original scancodes sent by keyboard
	//These hooked scancodes are result of Windows processing keyboard scancodes (translated to Set 1 by PS/2 controller or USB driver) and then sending some kind of aggregate result to applications
	//First of all scancodes are really BYTE in size while KBDLLHOOKSTRUCT allocates whole DWORD for each scancode (it actually has it's purpose - see below)
	//We don't get "break" scancodes - only "make" ones, with difference between them being mapped to wParam
	//Instead of escape scancodes we get LLKHF_EXTENDED flag which indicates that E0 was present in the original sequence (with one exception - Right Shift), but all other escapes are dropped
	//Fake scancodes are dropped but their presence reflected in resulting virtual key code (vk also reflects mod key states, NumLock state, selected layout and often LLKHF_EXTENDED flag)
	
	//Totally undocumented but Windows actually generates it's own fake scancodes which, in contrast with original fake scancodes, are hooked
	//Observed Windows' fake scancodes:
	//	Fake Shift UP before numpad control key (arrows, Home, End, etc.) DOWN while Shift is pressed and NumLock is on
	//	Fake Ctrl DOWN before AltGr DOWN
	//	Fake Ctrl DOWN when activating window with keyboard layout that differs from previous active window's layout
	//All these fakes have 0x200 ORed to their scancode
	//That's why it's DWORD in size - to accomodate this "fake" flag (though no other flags were ever observed in KBDLLHOOKSTRUCT's scancode field)
	
	//Another undocumented thing is that Right Shift events always have LLKHF_EXTENDED flag set for unknown purpose
	//Scientific guess: that was done for the uniformity of detecting (on software level) right mod keys (Right Shift has it's own scancode, while Right Alt and Ctrl keys havn't and relied on E0 scancode)
	//By design E0 escape scancode (which LLKHF_EXTENDED presumably represents) when used with Shifts indicates that Shift is fake one (in original keyboard terms)
	//Fake Shift UPs are sent when pressing non-numpad ("grey") control keys (arrows, Home, End, etc.) while holding Shift
	//Fake Shift DOWNs are sent when pressing non-numpad ("grey") control keys (arrows, Home, End, etc.) while NumLock is on
	//And, as was said earlier, such fake Shifts are dropped by the hook and already incorporated into virtual key
	//So while Right Shift events have LLKHF_EXTENDED set it doesn't mean that they are fake
	//But nevertheless this cause GetKeyNameText() to go awry and produce wrong results

	bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
	
#ifdef DEBUG
	//0x00000002 is LLKHF_LOWER_IL_INJECTED not found in MinGW-w64 4.9.2 winuser.h
	std::wcerr<<std::hex<<(key_up?L"KEYUP":L"KEYDOWN")<<L" VK: "<<kb_event->vkCode<<L" SC: "<<kb_event->scanCode<<(kb_event->scanCode?L"":L" NULL")<<(kb_event->scanCode&FAKE_SC?L" FAKE":L"")<<((wParam==WM_SYSKEYDOWN||wParam==WM_SYSKEYUP)?L" SYS":L"")<<(kb_event->flags&(0x00000002|LLKHF_INJECTED)?L" INJ":L"")<<(kb_event->flags&LLKHF_EXTENDED?L" EXT":L"")<<(kb_event->flags&LLKHF_ALTDOWN?L" ALT":L"")<<std::endl;
#endif
	
	//It is possible to receive zero scancode if some other program artificially creates keyboard events
	//Fake windows scancodes are discarded along with these zero scancodes
	if (kb_event->scanCode&&!(kb_event->scanCode&FAKE_SC)&&(OnModKey(kb_event->vkCode, key_up)||OnTargetKey(kb_event->vkCode, kb_event->scanCode, kb_event->flags&LLKHF_EXTENDED, key_up))) {
		if (key_up) {
			if (!hk_up&&hk_long_press) {
				if (kb_event->time>=hk_down_tick) {	//Excludes moment of system timer wrap around after 49.7 days of uptime
					if (kb_event->time-hk_down_tick>LONG_PRESS_DURATION) {
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
					hk_down_tick=kb_event->time;
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
	//For a long story behind scancodes see KeyTriplet::OnKeyPress above

	//It is possible to receive zero scancode if some other program artificially creates keyboard events
	//Fake windows scancodes are discarded along with these zero scancodes
	if (kb_event->scanCode&&!(kb_event->scanCode&FAKE_SC)&&(wParam==WM_KEYDOWN||wParam==WM_SYSKEYDOWN)) {
		//Ignoring Ctrl, Alt and Shift
		//This also helps detecting Break because of additonal Ctrl sent before every Break
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
				BINDED_KEY binded_key={LOBYTE(kb_event->vkCode) /* vk */, LOBYTE(kb_event->scanCode) /* sc */, kb_event->flags&LLKHF_EXTENDED /* ext */};
				//We are passing BINDED_KEY as WPARAM because, even taking align into account, it spans 3 byte which is less than WPARAM size on both x86 and x86_64
				static_assert(sizeof(BINDED_KEY)<=sizeof(WPARAM), L"sizeof(BINDED_KEY) should be less or equal sizeof(WPARAM)");
				PostMessage(dlg_hwnd, bind_wm, (WPARAM)binded_key.tuple, (LPARAM)kb_event->time);
				return true;
		}
	}
	
	return false;
}
