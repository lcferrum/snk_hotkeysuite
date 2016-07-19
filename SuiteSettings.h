#ifndef SUITESETTINGS_H
#define SUITESETTINGS_H

#include "SuiteCommon.h"
#include <string>
#include <windows.h>

class SuiteSettings {
protected:
	bool long_press;
	ModKeyType mod_key;
	DWORD binded_vk;
	DWORD binded_sc;
	HKL initial_hkl;
	//Stored indicates if external setting source exists - if it doesn't exist SaveSettings should create it before attempting to write settings
	//If SaveSettings was succesfull and setting wern't stored initially - stored is set to true (otherwise stored value remains the same)
	//Setting source doesn't necessary includes settings themselves - it could be empty
	bool stored;
	
	std::wstring shk_cfg_path;
	std::wstring lhk_cfg_path;
	std::wstring snk_path;
	
	std::wstring ExpandEnvironmentStringsWrapper(const std::wstring &path) const;
	
	//Special constructor for use in derived classes - directly sets shk_cfg_path, lhk_cfg_path and snk_path without any modifications and checks
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
	bool IsStored() { return stored; }
	
	virtual bool SaveSettings() { return true; }	//Load should be in constructor and suppresses all erors like save
	
	SuiteSettings();
};

class SuiteSettingsReg: public SuiteSettings {
private:
	bool user;

	bool RegSzQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &var) const;
	bool RegDwordQueryValue(HKEY reg_key, const wchar_t* key_name, DWORD &var) const;
public:
	std::wstring GetRegKey();
	virtual bool SaveSettings();
	
	SuiteSettingsReg();
};

class SuiteSettingsIni: public SuiteSettings {
private:
	std::wstring ini_path;
	std::wstring ini_section;
	
	bool IniSzQueryValue(const wchar_t* key_name, std::wstring &var) const;
	bool IniDwordQueryValue(const wchar_t* key_name, DWORD &var) const;
protected:
	std::wstring GetFullPathNameWrapper(const std::wstring &rel_path) const;
	bool CheckIfIniStored(const std::wstring &path, const std::wstring &section) const;
	
	//Special constructor for use in derived classes - directly sets shk_cfg_path, lhk_cfg_path, ini_section and ini_path without any modifications and checks
	//Warning: ini_path passed as third parameter to this constructor should be absolute file path or empty string
	SuiteSettingsIni(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &abs_ini_path, const std::wstring &ini_section);
public:
	std::wstring GetIniPath() { return ini_path; }
	virtual bool SaveSettings();

	SuiteSettingsIni(const std::wstring &rel_ini_path);
	SuiteSettingsIni();
};

class SuiteSettingsSection: public SuiteSettingsIni {
private:
	std::wstring StringToLower(std::wstring str) const;
public:
	SuiteSettingsSection(const std::wstring &ini_section);
};

class SuiteSettingsAppData: public SuiteSettingsIni {
private:
	std::wstring GetIniAppDataPath() const;
public:
	virtual bool SaveSettings();

	SuiteSettingsAppData();
};

#endif //SUITESETTINGS_H
