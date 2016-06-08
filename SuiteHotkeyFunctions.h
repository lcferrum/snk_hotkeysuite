#ifndef SUITEHOTKEYFUNCTIONS_H
#define SUITEHOTKEYFUNCTIONS_H

#include <functional>
#include <windows.h>

class KeyTriplet {
private:
	bool OnCtrlAlt(DWORD vk, bool key_up);
	bool OnShiftAlt(DWORD vk, bool key_up);
	bool OnCtrlShift(DWORD vk, bool key_up);
	bool OnTargetKey(DWORD vk, DWORD hk_binded_vk, bool key_up);
	
	std::function<bool(DWORD vk, bool key_up)> OnModKey;
	DWORD hk_binded_vk;
	bool hk_long_press;
	DWORD hk_state;
	DWORD hk_down_tick;
	bool hk_up;
public:
	KeyTriplet();

	void SetCtrlAlt() { OnModKey=std::bind(&KeyTriplet::OnCtrlAlt, this, std::placeholders::_1, std::placeholders::_2); }
	void SetShiftAlt() { OnModKey=std::bind(&KeyTriplet::OnShiftAlt, this, std::placeholders::_1, std::placeholders::_2); }
	void SetCtrlShift() { OnModKey=std::bind(&KeyTriplet::OnCtrlShift, this, std::placeholders::_1, std::placeholders::_2); }
	void SetBindedVK(DWORD new_binding) { hk_binded_vk=new_binding; }
	void SetLongPress(bool enabled) { hk_long_press=enabled; }
	bool OnKeyPress(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
};

#endif //SUITEHOTKEYFUNCTIONS_H
