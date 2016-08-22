#ifndef SUITEHOTKEYFUNCTIONS_H
#define SUITEHOTKEYFUNCTIONS_H

#include "SuiteCommon.h"
#include <functional>
#include <windows.h>

class KeyTriplet {
private:
	bool OnCtrlAlt(DWORD vk, bool key_up);
	bool OnShiftAlt(DWORD vk, bool key_up);
	bool OnCtrlShift(DWORD vk, bool key_up);
	bool OnTargetKey(DWORD vk, DWORD sc, bool ext, bool key_up);
	
	//Using std::function with std::bind is alternative to this:
	//	bool (KeyTriplet::*OnModKey)(DWORD vk, bool key_up);
	//	OnModKey=OnCtrlAlt;
	//Sure assignment here is cleaner than std::bind monstrosity but pointers to member function are ugly
	//And calling these pointers is even uglier:
	//	(this->*OnModKey)(vk, key_up);
	std::function<bool(DWORD vk, bool key_up)> OnModKey;
	BINDED_KEY hk_binded_key;
	bool hk_long_press;
	DWORD hk_state;
	DWORD hk_down_tick;
	bool hk_up;
public:
	KeyTriplet();

	void SetCtrlAlt() { OnModKey=std::bind(&KeyTriplet::OnCtrlAlt, this, std::placeholders::_1, std::placeholders::_2); }
	void SetShiftAlt() { OnModKey=std::bind(&KeyTriplet::OnShiftAlt, this, std::placeholders::_1, std::placeholders::_2); }
	void SetCtrlShift() { OnModKey=std::bind(&KeyTriplet::OnCtrlShift, this, std::placeholders::_1, std::placeholders::_2); }
	void SetBindedKey(BINDED_KEY new_key_binding) { hk_binded_key=new_key_binding; }
	void SetLongPress(bool enabled) { hk_long_press=enabled; }
	bool OnKeyPress(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
};

bool BindKey(HWND dlg_hwnd, UINT bind_wm, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);

#endif //SUITEHOTKEYFUNCTIONS_H
