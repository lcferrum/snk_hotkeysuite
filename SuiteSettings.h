#ifndef SUITESETTINGS_H
#define SUITESETTINGS_H

#include "SuiteCommon.h"
#include <windows.h>

class SuiteSettings {
private:
	bool long_press;
	ModKeyType mod_key;
	DWORD binded_vk;
	DWORD binded_sc;
	HKL initial_hkl;
public:
	bool GetLongPress() { return long_press; }
	void SetLongPress(bool enabled) { long_press=enabled; }
	ModKeyType GetModKey() { return mod_key; }
	void SetModKey(ModKeyType new_key) { mod_key=new_key; }
	DWORD GetBindedSC() { return binded_sc; }
	DWORD GetBindedVK() { 
		if (binded_vk==0xFF)
			return MapVirtualKeyEx(binded_sc, MAPVK_VSC_TO_VK, initial_hkl);
		else
			return binded_vk;
	}
	void SetBindedKey(DWORD new_vk_binding, DWORD new_sc_binding) { binded_vk=new_vk_binding; binded_sc=new_sc_binding; }
	
	SuiteSettings();
	~SuiteSettings();
};

#endif //SUITESETTINGS_H
