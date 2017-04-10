#ifndef SUITESETTINGS_H
#define SUITESETTINGS_H

#include "SuiteCommon.h"
#include <string>
#include <windows.h>

//Some methods are defined as static here to indicate that their output is used in member initilization 
//They shouldn't set or query any of class members themselves because they are not initialized when method is called

class SuiteSettings {
private:
	std::wstring ExpandEnvironmentStringsWrapper(const std::wstring &path) const;
protected:
	bool long_press;
	ModKeyType mod_key;
	BINDED_KEY binded_key;
	HKL initial_hkl;
	//Stored indicates if external setting source exists - if it doesn't exist SaveSettings should create it before attempting to write settings
	//If SaveSettings was succesfull and setting wern't stored initially - stored is set to true (otherwise stored value remains the same)
	//Setting source doesn't necessary include settings themselves - it could be empty
	bool stored;
	//Each set bit in changed variable indicate which setting was changed since last succesfull SaveSettings call
	unsigned char changed;
	
	std::wstring shk_cfg_path;
	std::wstring lhk_cfg_path;
	std::wstring snk_path;
	
	bool MapVkToSc(DWORD src_vk, BINDED_KEY &dst_key) const;
	bool MapScToVk(DWORD src_sc, BINDED_KEY &dst_key) const;
	
	//Special constructor for use in derived classes - directly sets shk_cfg_path, lhk_cfg_path and snk_path without any modifications and checks
	SuiteSettings(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &snk_path);
public:
	bool GetLongPress() const { return long_press; }
	void SetLongPress(bool enabled);
	ModKeyType GetModKey() const { return mod_key; }
	void SetModKey(ModKeyType new_key);
	BINDED_KEY GetBindedKey() const { return binded_key; }
	void SetBindedKey(BINDED_KEY new_key_binding);
	std::wstring GetShkCfgPath() const { return ExpandEnvironmentStringsWrapper(shk_cfg_path); }
	std::wstring GetLhkCfgPath() const { return ExpandEnvironmentStringsWrapper(lhk_cfg_path); }
	std::wstring GetSnkPath() const { return ExpandEnvironmentStringsWrapper(snk_path); }
	bool IsStored() const { return stored; }
	
	virtual std::wstring GetStoredLocation() const { return snk_path; }
	virtual bool SaveSettings() { return true; }	//Load should be in constructor and suppresses all erors
	
	SuiteSettings();
};

class SuiteSettingsReg: public SuiteSettings {
private:
	enum class Hive:char {AUTO, CURRENT_USER, LOCAL_MACHINE};
	bool RegSzQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &var) const;
	bool RegDwordQueryValue(HKEY reg_key, const wchar_t* key_name, DWORD &var) const;
	SuiteSettingsReg(Hive hive);
protected:
	Hive hive;
public:
	virtual std::wstring GetStoredLocation() const;
	virtual bool SaveSettings();
	
	SuiteSettingsReg();						//Auto chooses hive - depending on what registry path is available (defaults to CURRENT_USER)
	SuiteSettingsReg(bool current_user);	//Forces hive - CURRENT_USER or LOCAL_MACHINE
};

class SuiteSettingsIni: public SuiteSettings {
private:
	bool IniSzQueryValue(const wchar_t* key_name, std::wstring &var) const;
	bool IniDwordQueryValue(const wchar_t* key_name, DWORD &var) const;
	static std::wstring GetFullPathNameWrapper(const std::wstring &rel_path);
protected:
	std::wstring ini_path;
	std::wstring ini_section;
	
	static bool CheckIfIniStored(const std::wstring &path, const std::wstring &section);
	
	//Special constructor for use in derived classes - directly sets shk_cfg_path, lhk_cfg_path, ini_section and ini_path without any modifications and checks
	//Warning: ini_path passed as third parameter to this constructor should be absolute file path or empty string
	SuiteSettingsIni(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &abs_ini_path, const std::wstring &ini_section);
public:
	virtual std::wstring GetStoredLocation() const { return ini_path; }
	virtual bool SaveSettings();

	SuiteSettingsIni(const std::wstring &rel_ini_path);
	SuiteSettingsIni();
};

class SuiteSettingsSection: public SuiteSettingsIni {
public:
	SuiteSettingsSection(const std::wstring &ini_section);
};

class SuiteSettingsAppData: public SuiteSettingsIni {
private:
	enum class Location:char {AUTO, CURRENT_USER, ALL_USERS};
	static std::wstring GetIniAppDataPath(Location loc);
public:
	virtual bool SaveSettings();

	SuiteSettingsAppData();						//Auto chooses appdata location - depending on what appdata path is available (defaults to CURRENT_USER)
	SuiteSettingsAppData(bool current_user);	//Forces appdata location - current or all users
};

#endif //SUITESETTINGS_H
