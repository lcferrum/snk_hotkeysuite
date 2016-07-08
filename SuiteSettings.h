#ifndef SUITESETTINGS_H
#define SUITESETTINGS_H

#include "SuiteCommon.h"
#include <string>
#include <windows.h>

class SuiteSettings {
private:
	std::wstring ExpandEnvironmentStringsWrapper(const std::wstring &path);
protected:
	enum StorageType:char {USER, SYSTEM, CUSTOM, NONE};

	bool long_press;
	ModKeyType mod_key;
	DWORD binded_vk;
	DWORD binded_sc;
	HKL initial_hkl;
	bool valid;
	StorageType storage;
	
	std::wstring shk_cfg_path;
	std::wstring lhk_cfg_path;
	std::wstring snk_path;
	
	SuiteSettings(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &snk_path);
public:
	bool GetLongPress() { return long_press; }
	void SetLongPress(bool enabled) { long_press=enabled; }
	ModKeyType GetModKey() { return mod_key; }
	void SetModKey(ModKeyType new_key) { mod_key=new_key; }
	DWORD GetBindedSC() { return binded_sc; }
	DWORD GetBindedVK() { return binded_vk; }
	void SetBindedKey(DWORD new_vk_binding, DWORD new_sc_binding) { binded_vk=new_vk_binding; binded_sc=new_sc_binding; }
	std::wstring GetShkCfgPath() { return ExpandEnvironmentStringsWrapper(shk_cfg_path); }
	std::wstring GetLhkCfgPath() { return ExpandEnvironmentStringsWrapper(lhk_cfg_path); }
	std::wstring GetSnkPath() { return ExpandEnvironmentStringsWrapper(snk_path); }
	bool IsValid() { return valid; }
	
	virtual void SaveSettings() {}	//Load should be in constructor and suppresses all erors like save
	
	SuiteSettings();
};

class SuiteSettingsReg: public SuiteSettings {
private:
	bool RegSzQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &var);
	bool RegDwordQueryValue(HKEY reg_key, const wchar_t* key_name, DWORD &var);
	void LoadSettingsFromReg();
public:
	virtual void SaveSettings();
	
	SuiteSettingsReg();
};

class SuiteSettingsIni: public SuiteSettings {
private:
	std::wstring ini_path;
	
	std::wstring MakeIniPrefixFromPath(const std::wstring &path);
	void LoadSettingsFromIni();
protected:
	std::wstring GetFullPathNameWrapper(const std::wstring &rel_path);
	
	SuiteSettingsIni(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &abs_ini_path);
public:
	virtual void SaveSettings();

	SuiteSettingsIni(const std::wstring &rel_ini_path);
};

class SuiteSettingsPortable: public SuiteSettingsIni {
public:
	SuiteSettingsPortable();
};

class SuiteSettingsAppData: public SuiteSettingsIni {
private:
	std::wstring GetIniAppDataPath();
public:
	SuiteSettingsAppData();
};

#endif //SUITESETTINGS_H
