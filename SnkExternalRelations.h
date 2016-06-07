#ifndef SNKEXTERNALRELATIONS_H
#define SNKEXTERNALRELATIONS_H

#include <functional>
#include <windows.h>

namespace SnkHkFn {
	bool OnCtrlAlt(DWORD vk, bool key_up);
	bool OnCtrlShift(DWORD vk, bool key_up);
	bool OnShiftAlt(DWORD vk, bool key_up);
	bool OnTriplet(std::function<bool(DWORD vk, bool key_up)> OnModKey, DWORD hk_target_vk, bool hk_allow_lp, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event);
}

#endif //SNKEXTERNALRELATIONS_H
