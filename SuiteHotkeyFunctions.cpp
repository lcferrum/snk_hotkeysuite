#include "SuiteHotkeyFunctions.h"
#include "SuiteCommon.h"

#ifdef DEBUG
#include <iostream>
#endif

#define FAKE_SC	0x200	//Undocumented windows fake scancode flag

#ifndef _WIN64
extern "C" bool __cdecl SinglePressCtrlAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool __cdecl SinglePressShiftAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool __cdecl SinglePressCtrlShiftEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool __cdecl LongPressCtrlAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool __cdecl LongPressShiftAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool __cdecl LongPressCtrlShiftEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
#else
extern "C" bool SinglePressCtrlAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool SinglePressShiftAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool SinglePressCtrlShiftEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool LongPressCtrlAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool LongPressShiftAltEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
extern "C" bool LongPressCtrlShiftEventHandler(KeyTriplet* this_ptr, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
#endif

//This version of KeyTriplet utilizes assembler-otimized callbacks for lowlevel keyboard hook HOOKPROC
//For each modkey pair variation and single/long press mode we have it's own callback (6 in total)
//It's somehow a code bloat but in the end we get less branching
//Funny thing is that resulting compiled object is smaller than pure-C++ version anyway
//Some 57% smaller, and 14 times faster (though we are still talking about nanoseconds of execution time with both versions)
//Resulting exe packed w/ UPX is also smaller but not considerably (around half-Kb)

KeyTriplet::KeyTriplet(wchar_t* cmdline_s, wchar_t* cmdline_l):
	hk_sc(DEFAULT_SC), hk_ext(DEFAULT_EXT), hk_state(0), hk_engaged(0), hk_down_tick(0), hk_cmdline_s(cmdline_s), hk_cmdline_l(cmdline_l), hk_pi{}, hk_si{sizeof(STARTUPINFO)}
{}

inline void KeyTriplet::ResetEventHandler()
{ 
	hk_state=0; 
	hk_engaged=0; 
	hk_down_tick=0; 
}

KeyTriplet::EventHandlerFn KeyTriplet::CreateEventHandler(const SuiteSettings *settings) 
{
	ResetEventHandler();
	hk_sc=settings->GetBindedKey().sc;
	hk_ext=settings->GetBindedKey().ext;	//By C++ standard it is guaranteed that true will be converted to 1 and false to 0
	switch (settings->GetModKey()) {
		case ModKeyType::CTRL_ALT:
			return settings->GetLongPress()?LongPressCtrlAltEventHandler:SinglePressCtrlAltEventHandler;
			break;
		case ModKeyType::SHIFT_ALT:
			return settings->GetLongPress()?LongPressShiftAltEventHandler:SinglePressShiftAltEventHandler;
			break;
		case ModKeyType::CTRL_SHIFT:
			return settings->GetLongPress()?LongPressCtrlShiftEventHandler:SinglePressCtrlShiftEventHandler;
			break;
	}
}

#ifdef DEBUG
bool DebugEventHandler(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
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

	wchar_t key_buf[MAX_PATH];
	std::wcerr<<std::hex<<
				((wParam==WM_KEYUP||wParam==WM_SYSKEYUP)?L"KEYUP":L"KEYDOWN")<<
				L" VK: "<<kb_event->vkCode<<
				L" SC: "<<kb_event->scanCode<<
				L" KEY: \""<<(GetKeyNameText((kb_event->scanCode&0xFF)<<16|(kb_event->flags&LLKHF_EXTENDED)<<24, key_buf, MAX_PATH)?key_buf:L"UNKNOWN")<<L"\""<<
				(kb_event->scanCode&FAKE_SC?L" FAKE":L"")<<
				(kb_event->scanCode?L"":L" NULL")<<
				((wParam==WM_SYSKEYDOWN||wParam==WM_SYSKEYUP)?L" SYS":L"")<<
				//0x00000002 is LLKHF_LOWER_IL_INJECTED not found in MinGW-w64 4.9.2 winuser.h
				(kb_event->flags&(0x00000002|LLKHF_INJECTED)?L" INJ":L"")<<
				(kb_event->flags&LLKHF_EXTENDED?L" EXT":L"")<<
				(kb_event->flags&LLKHF_ALTDOWN?L" ALT":L"")<<
				std::dec<<std::endl;
}
#endif

bool BindKeyEventHandler(HWND dlg_hwnd, UINT bind_wm, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)
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
				PostMessage(dlg_hwnd, bind_wm, (WPARAM)binded_key.tuple, 0);
				return true;
		}
	}
	
	return false;
}
