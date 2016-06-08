#ifndef SUITESETTINGS_H
#define SUITESETTINGS_H

#include <windows.h>

class SuiteSettings {
public:
	enum ModKeyType:char {CTRL_ALT, SHIFT_ALT, CTRL_SHIFT};
private:
	bool long_press;
	ModKeyType mod_key;
	DWORD binded_vk;
public:
	bool GetLongPress() { return long_press; }
	void SetLongPress(bool enabled) { long_press=enabled; }
	ModKeyType GetModKey() { return mod_key; }
	void SetModKey(ModKeyType new_key) { mod_key=new_key; }
	DWORD GetBindedVK() { return binded_vk; }
	void SetBindedVK(DWORD new_binding) { binded_vk=new_binding; }
	
	SuiteSettings();
	~SuiteSettings();
};

#endif //SUITESETTINGS_H
