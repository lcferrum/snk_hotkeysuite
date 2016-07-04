#ifndef SUITESETTINGS_H
#define SUITESETTINGS_H

#include "SuiteCommon.h"
#include <string>
#include <windows.h>

class SuiteSettings {
private:
	bool long_press;
	ModKeyType mod_key;
	DWORD binded_vk;
	DWORD binded_sc;
	HKL initial_hkl;
	
	std::wstring shk_cfg_path;
	std::wstring lhk_cfg_path;
	std::wstring snk_path;
public:
	bool GetLongPress() { return long_press; }
	void SetLongPress(bool enabled) { long_press=enabled; }
	ModKeyType GetModKey() { return mod_key; }
	void SetModKey(ModKeyType new_key) { mod_key=new_key; }
	DWORD GetBindedSC() { return binded_sc; }
	DWORD GetBindedVK() { return binded_vk; }
/*		
		if (binded_vk==0xFF)
			return MapVirtualKeyEx(binded_sc, MAPVK_VSC_TO_VK, initial_hkl);
		else
			return binded_vk;
*/
	void SetBindedKey(DWORD new_vk_binding, DWORD new_sc_binding) { binded_vk=new_vk_binding; binded_sc=new_sc_binding; }
	std::wstring GetShkCfgPath() { return shk_cfg_path; }
	std::wstring GetLhkCfgPath() { return lhk_cfg_path; }
	std::wstring GetSnkPath() { return snk_path; }
	
	virtual void SaveSettings() {}	//Load should be in constructor and suppresses all erors like save
	
	SuiteSettings();
	~SuiteSettings();
};

class SuiteSettingsReg: public SuiteSettings {
private:

	void LoadSettingsFromReg();
public:
	virtual void SaveSettings();
	
	SuiteSettingsReg();
	~SuiteSettingsReg();
};

class SuiteSettingsIni: public SuiteSettings {
private:
	std::wstring ini_path;
	
	void LoadSettingsFromIni();
public:
	virtual void SaveSettings();

	SuiteSettingsIni();
	SuiteSettingsIni(const std::wstring &ini_path);
	~SuiteSettingsIni();
};

#endif //SUITESETTINGS_H
