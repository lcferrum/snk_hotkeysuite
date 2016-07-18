#include "SuiteSettings.h"
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cwchar>

#ifdef DEBUG
#include <iostream>
#endif

#define KEY_ONHOTKEYCFGPATH				L"OnHotkeyCfgPath"
#define KEY_ONHOTKEYLONGPRESSCFGPATH	L"OnHotkeyLongPressCfgPath"
#define	KEY_SNKPATH						L"SnkPath"
#define KEY_HOTKEYSCANCODE				L"HotkeyScancode"
#define KEY_HOTKEYVIRTUALKEY			L"HotkeyVirtualKey"
#define KEY_HOTKEYMODIFIERKEY			L"HotkeyModifierKey"
#define KEY_LONGPRESSENABLED			L"LongPressEnabled"
#define VAL_CTRLALT						L"CtrlAlt"
#define VAL_SHIFTALT					L"ShiftAlt"
#define VAL_CTRLSHIFT					L"CtrlShift"

#define DEFAULT_SHK_CFG_PATH	L"on_hotkey.txt"
#define DEFAULT_LHK_CFG_PATH	L"on_hotkey_long_press.txt"
#define DEFAULT_INI_PATH		L"HotkeySuite.ini"
#define DEFAULT_SNK_PATH		L"SnKh.exe"

#define SUITE_REG_PATH			L"Software\\SnK HotkeySuite"
#define SUITE_INI_SECTION		L"HotkeySuite"
#define SUITE_APPDATA_DIR		L"SnK HotkeySuite"	//Single level dir only

SuiteSettings::SuiteSettings(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &snk_path):
	long_press(false), mod_key(ModKeyType::CTRL_ALT), binded_vk(DEFAULT_VK), binded_sc(DEFAULT_SC), initial_hkl(GetKeyboardLayout(0)), stored(false),
	shk_cfg_path(shk_cfg_path), lhk_cfg_path(lhk_cfg_path), snk_path(snk_path)
{
	SetEnvironmentVariable(L"HS_EXE_PATH", GetExecutableFileName(true).c_str());
#ifdef DEBUG
	std::wcerr<<L"SET HS_EXE_PATH="<<GetExecutableFileName(true)<<std::endl;
#endif
}

SuiteSettings::SuiteSettings():
	SuiteSettings(DEFAULT_SHK_CFG_PATH, DEFAULT_LHK_CFG_PATH, DEFAULT_SNK_PATH)
{}
	
std::wstring SuiteSettings::ExpandEnvironmentStringsWrapper(const std::wstring &path) const
{ 
	wchar_t dummy_buf;

	//Documentation says that lpDst parameter is optional but Win 95 version of this function actually fails if lpDst is NULL
	//So using dummy buffer to get needed buffer length (function returns length in characters including terminating NULL)
	//If returned length is 0 - it is error
	if (DWORD buf_len=ExpandEnvironmentStrings(path.c_str(), &dummy_buf, 0)) {
		wchar_t string_buf[buf_len];
		//Ensuring that returned length is expected length
		if (ExpandEnvironmentStrings(path.c_str(), string_buf, buf_len)==buf_len) 
			return string_buf;
	}
	
	return L"";
}

//------------------------------ INI -------------------------------

SuiteSettingsIni::SuiteSettingsIni(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &abs_ini_path, const std::wstring &ini_section):
	SuiteSettings(shk_cfg_path, lhk_cfg_path, L"%HS_EXE_PATH%\\" DEFAULT_SNK_PATH), ini_path(abs_ini_path), ini_section(ini_section)
{
	//We don't check if path is absolute because this constructor is protected and and it is by design that passed path should be absolute file path
	
	//Empty path is only exception - derived constructor should pass it in case of error
	if (ini_path.empty())
		return;
	
	//Actually this branch should always work because, once again, passed path should be absolute file path or empty string
	size_t last_backslash;
	if ((last_backslash=ini_path.find_last_of(L'\\'))!=std::wstring::npos) {
		SetEnvironmentVariable(L"HS_INI_PATH", ini_path.substr(0, last_backslash).c_str());
	#ifdef DEBUG
		std::wcerr<<L"SET HS_INI_PATH="<<ini_path.substr(0, last_backslash).c_str()<<std::endl;
	#endif
	}
	
	//Leave default values if ini file doesn't exist
	DWORD dwAttrib=GetFileAttributes(ini_path.c_str());
	if (dwAttrib==INVALID_FILE_ATTRIBUTES||(dwAttrib&FILE_ATTRIBUTE_DIRECTORY))
		return;
	
	//Check if section exists
	//If lpKeyName is NULL GetPrivateProfileString copies all keys for lpAppName section to lpReturnedString buffer
	//If buffer is too small - returned list is truncated to buffer size, terminated with two NULLs and GetLastError is ERROR_MORE_DATA
	//If there are no keys in section and section exists - function succeeds (GetLastError=ERROR_SUCCESS) but return buffer should be at least 2 character in size so not to get ERROR_MORE_DATA
	//In all other cases (file or section doesn't exist) GetLastError=ERROR_FILE_NOT_FOUND
	wchar_t section_test_buf[2];
	GetPrivateProfileString(ini_section.c_str(), NULL, NULL, section_test_buf, 2, ini_path.c_str());
	if (GetLastError()==ERROR_FILE_NOT_FOUND)
		return;
	
	stored=true;
		
	IniSzQueryValue(KEY_ONHOTKEYCFGPATH, this->shk_cfg_path);
	IniSzQueryValue(KEY_ONHOTKEYLONGPRESSCFGPATH, this->lhk_cfg_path);
	IniSzQueryValue(KEY_SNKPATH, snk_path);
#ifdef DEBUG
	std::wcerr<<L"KEY_ONHOTKEYCFGPATH="<<this->shk_cfg_path<<std::endl;
	std::wcerr<<L"KEY_ONHOTKEYLONGPRESSCFGPATH="<<this->lhk_cfg_path<<std::endl;
	std::wcerr<<L"KEY_SNKPATH="<<snk_path<<std::endl;
#endif
	
	bool sc_found=IniDwordQueryValue(KEY_HOTKEYSCANCODE, binded_sc);
	bool vk_found=IniDwordQueryValue(KEY_HOTKEYVIRTUALKEY, binded_vk);
	if (sc_found&&!vk_found) {
		//If SC was in registry but VK wasn't - make VK from SC
		binded_vk=MapVirtualKeyEx(binded_sc, MAPVK_VSC_TO_VK, initial_hkl);
	}
	if (!sc_found&&vk_found) {
		//If VK was in registry but SC wasn't - make SC from VK
		binded_sc=MapVirtualKeyEx(binded_vk, MAPVK_VK_TO_VSC, initial_hkl);
	}
	//In case if both VK and SC wern't found - default values will be kept
#ifdef DEBUG
	std::wcerr<<L"KEY_HOTKEYSCANCODE="<<std::hex<<binded_sc<<std::endl;
	std::wcerr<<L"KEY_HOTKEYVIRTUALKEY="<<std::hex<<binded_vk<<std::endl;
#endif
	
	std::wstring mod_key_str;
	IniSzQueryValue(KEY_HOTKEYMODIFIERKEY, mod_key_str);
	if (!mod_key_str.compare(VAL_CTRLALT)) {
		mod_key=ModKeyType::CTRL_ALT;
#ifdef DEBUG
		std::wcerr<<L"KEY_HOTKEYMODIFIERKEY=VAL_CTRLALT"<<std::endl;
#endif
	} else if (!mod_key_str.compare(VAL_SHIFTALT)) {
		mod_key=ModKeyType::SHIFT_ALT;
#ifdef DEBUG
		std::wcerr<<L"KEY_HOTKEYMODIFIERKEY=VAL_SHIFTALT"<<std::endl;
#endif
	} else if (!mod_key_str.compare(VAL_CTRLSHIFT)) {
		mod_key=ModKeyType::CTRL_SHIFT;
#ifdef DEBUG
		std::wcerr<<L"KEY_HOTKEYMODIFIERKEY=VAL_CTRLSHIFT"<<std::endl;
#endif
	}
	
	DWORD long_press_dw;
	if (IniDwordQueryValue(KEY_LONGPRESSENABLED, long_press_dw)) {
		long_press=long_press_dw;
	}
#ifdef DEBUG
	std::wcerr<<L"KEY_LONGPRESSENABLED="<<(long_press?L"TRUE":L"FALSE")<<std::endl;
#endif
}

SuiteSettingsIni::SuiteSettingsIni(const std::wstring &rel_ini_path):
	SuiteSettingsIni(L"%HS_INI_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_INI_PATH%\\" DEFAULT_LHK_CFG_PATH, GetFullPathNameWrapper(rel_ini_path), SUITE_INI_SECTION)
{}

SuiteSettingsIni::SuiteSettingsIni():
	SuiteSettingsIni(L"%HS_EXE_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_EXE_PATH%\\" DEFAULT_LHK_CFG_PATH, GetExecutableFileName(true)+L"\\" DEFAULT_INI_PATH, SUITE_INI_SECTION)
{}

bool SuiteSettingsIni::IniSzQueryValue(const wchar_t* key_name, std::wstring &var) const
{
	wchar_t data_buf[MAX_PATH+1];	//One more character is added so not to trigger ERROR_MORE_DATA if read string fits exactly in MAX_PATH
	
	//GetPrivateProfileString have an interesting behaviour
	//In case of any error (file not found, section not found, key not found) lpDefault will be copied to lpReturnedString and returned size will be set accordingly (length of lpDefault)
	//If lpReturnedString buffer can't hold all of read string - it will be truncated to nSize-1 characters and NULL-terminated
	//So to check if there really was any error while reading key it's necessary to check GetLastError returned value which will be ERROR_FILE_NOT_FOUND if anything bad happened
	//If read string (not including NULL-terminator) equals or more than nSize-1 - GetLastError will return ERROR_MORE_DATA
	//Setting lpDefault to NULL merely results in empty string being used for lpDefault
	GetPrivateProfileString(ini_section.c_str(), key_name, NULL, data_buf, MAX_PATH+1, ini_path.c_str());
	
	if (GetLastError()==ERROR_SUCCESS) {
		var=data_buf;
		return true;
	} else
		return false;
}

bool SuiteSettingsIni::IniDwordQueryValue(const wchar_t* key_name, DWORD &var) const
{
	wchar_t data_buf[15]; //This buffer size can hold dec, hex or oct representation of any (unsigned) long, including minus sign and NULL terminator, while not triggering ERROR_MORE_DATA
	
	//GetPrivateProfileInt understands only decimals so using custom version for reading DWORDs
	//For more on GetPrivateProfileString behaviour see IniSzQueryValue function
	GetPrivateProfileString(ini_section.c_str(), key_name, NULL, data_buf, 15, ini_path.c_str());
	
	DWORD derr;
	if (GetLastError()==ERROR_SUCCESS) {
		wchar_t* non_ul_char;
		DWORD dw_buf=wcstoul(data_buf, &non_ul_char, 0);
		//endptr will be set to the first non-integer character of str
		//So if it's points to NULL - str is empty or doesn't have any illegal characters
		//If str doesn't have any illegal characters but doesn't fit into DWORD - errno is set to ERANGE
		if (errno!=ERANGE&&*non_ul_char==L'\0') {
			var=dw_buf;
			return true;
		}
	}		
	
	return false;
}

std::wstring SuiteSettingsIni::GetFullPathNameWrapper(const std::wstring &rel_path) const
{
	wchar_t dummy_buf;
	wchar_t* fname_pos;

	//If returned length is 0 - it is error
	if (DWORD buf_len=GetFullPathName(rel_path.c_str(), 0, &dummy_buf, &fname_pos)) {
		wchar_t string_buf[buf_len];
		//Ensuring that returned length is expected length and resulting string contains file name (i.e. it's not directory)
		if (GetFullPathName(rel_path.c_str(), buf_len, string_buf, &fname_pos)+1==buf_len&&fname_pos!=NULL) 
			return string_buf;
	}
	
	return L"";
}

bool SuiteSettingsIni::SaveSettings()
{
	//Empty paths are not allowed
	if (ini_path.empty())
		return false;

	//WritePrivateProfileString can create file if it doesn't exists but only if all the directories in file path exist
	
	bool save_succeeded=true;
	
	if (!WritePrivateProfileString(ini_section.c_str(), KEY_ONHOTKEYCFGPATH, shk_cfg_path.c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (!WritePrivateProfileString(ini_section.c_str(), KEY_ONHOTKEYLONGPRESSCFGPATH, lhk_cfg_path.c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (!WritePrivateProfileString(ini_section.c_str(), KEY_SNKPATH, snk_path.c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYSCANCODE, DwordToHexString(binded_sc, 8).c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYVIRTUALKEY, DwordToHexString(binded_vk, 8).c_str(), ini_path.c_str()))
		save_succeeded=false;

	switch (mod_key) {
		case ModKeyType::CTRL_ALT:
			if (!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYMODIFIERKEY, VAL_CTRLALT, ini_path.c_str()))
				save_succeeded=false;
			break;
		case ModKeyType::SHIFT_ALT:
			if (!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYMODIFIERKEY, VAL_SHIFTALT, ini_path.c_str()))
				save_succeeded=false;
			break;
		case ModKeyType::CTRL_SHIFT:
			if (!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYMODIFIERKEY, VAL_CTRLSHIFT, ini_path.c_str()))
				save_succeeded=false;
			break;
	}
	
	DWORD long_press_dw=long_press?1:0;
	if (!WritePrivateProfileString(ini_section.c_str(), KEY_LONGPRESSENABLED, DwordToHexString(long_press_dw, 8).c_str(), ini_path.c_str()))
		save_succeeded=false;
	
	if (!stored&&save_succeeded)
		stored=true;
	
	return save_succeeded;
}

//------------------------------ SECTION -------------------------------

SuiteSettingsSection::SuiteSettingsSection(const std::wstring &ini_section):
	SuiteSettingsIni(std::wstring(L"%HS_EXE_PATH%\\")+StringToLower(ini_section)+L"_" DEFAULT_SHK_CFG_PATH, std::wstring(L"%HS_EXE_PATH%\\")+StringToLower(ini_section)+L"_" DEFAULT_LHK_CFG_PATH, GetExecutableFileName(true)+L"\\" DEFAULT_INI_PATH, ini_section)
{}

std::wstring SuiteSettingsSection::StringToLower(std::wstring str) const
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

//------------------------------ APPDATA -------------------------------

SuiteSettingsAppData::SuiteSettingsAppData():
	SuiteSettingsIni(L"%HS_INI_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_INI_PATH%\\" DEFAULT_LHK_CFG_PATH, GetIniAppDataPath(), SUITE_INI_SECTION)
{}

std::wstring SuiteSettingsAppData::GetIniAppDataPath() const
{
	//TODO: choose AppData path (system or user) and return L"" in case of error
	return L"";
}

bool SuiteSettingsAppData::SaveSettings()
{
	//TODO: create directory tree: return SaveSettings in case of success or return false in case of error
	return SuiteSettingsIni::SaveSettings();
}

//------------------------------ REGISTRY -------------------------------

SuiteSettingsReg::SuiteSettingsReg():
	SuiteSettings(L"%HS_EXE_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_EXE_PATH%\\" DEFAULT_LHK_CFG_PATH, L"%HS_EXE_PATH%\\" DEFAULT_SNK_PATH), user(false)
{
	HKEY reg_key;
	
	//Leave default values if reg key wasn't found in both HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE
	if (RegOpenKeyEx(HKEY_CURRENT_USER, SUITE_REG_PATH, 0, KEY_READ, &reg_key)==ERROR_SUCCESS) {
		user=true;
		stored=true;
	} else {
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, KEY_READ, &reg_key)==ERROR_SUCCESS)
			stored=true;
		else
			return;
	}
		
	RegSzQueryValue(reg_key, KEY_ONHOTKEYCFGPATH, shk_cfg_path);
	RegSzQueryValue(reg_key, KEY_ONHOTKEYLONGPRESSCFGPATH, lhk_cfg_path);
	RegSzQueryValue(reg_key, KEY_SNKPATH, snk_path);
#ifdef DEBUG
	std::wcerr<<L"KEY_ONHOTKEYCFGPATH="<<shk_cfg_path<<std::endl;
	std::wcerr<<L"KEY_ONHOTKEYLONGPRESSCFGPATH="<<lhk_cfg_path<<std::endl;
	std::wcerr<<L"KEY_SNKPATH="<<snk_path<<std::endl;
#endif
	
	bool sc_found=RegDwordQueryValue(reg_key, KEY_HOTKEYSCANCODE, binded_sc);
	bool vk_found=RegDwordQueryValue(reg_key, KEY_HOTKEYVIRTUALKEY, binded_vk);
	if (sc_found&&!vk_found) {
		//If SC was in registry but VK wasn't - make VK from SC
		binded_vk=MapVirtualKeyEx(binded_sc, MAPVK_VSC_TO_VK, initial_hkl);
	}
	if (!sc_found&&vk_found) {
		//If VK was in registry but SC wasn't - make SC from VK
		binded_sc=MapVirtualKeyEx(binded_vk, MAPVK_VK_TO_VSC, initial_hkl);
	}
	//In case if both VK and SC wern't found - default values will be kept
#ifdef DEBUG
	std::wcerr<<L"KEY_HOTKEYSCANCODE="<<std::hex<<binded_sc<<std::endl;
	std::wcerr<<L"KEY_HOTKEYVIRTUALKEY="<<std::hex<<binded_vk<<std::endl;
#endif
	
	std::wstring mod_key_str;
	RegSzQueryValue(reg_key, KEY_HOTKEYMODIFIERKEY, mod_key_str);
	if (!mod_key_str.compare(VAL_CTRLALT)) {
		mod_key=ModKeyType::CTRL_ALT;
#ifdef DEBUG
		std::wcerr<<L"KEY_HOTKEYMODIFIERKEY=VAL_CTRLALT"<<std::endl;
#endif
	} else if (!mod_key_str.compare(VAL_SHIFTALT)) {
		mod_key=ModKeyType::SHIFT_ALT;
#ifdef DEBUG
		std::wcerr<<L"KEY_HOTKEYMODIFIERKEY=VAL_SHIFTALT"<<std::endl;
#endif
	} else if (!mod_key_str.compare(VAL_CTRLSHIFT)) {
		mod_key=ModKeyType::CTRL_SHIFT;
#ifdef DEBUG
		std::wcerr<<L"KEY_HOTKEYMODIFIERKEY=VAL_CTRLSHIFT"<<std::endl;
#endif
	}
	
	DWORD long_press_dw;
	if (RegDwordQueryValue(reg_key, KEY_LONGPRESSENABLED, long_press_dw)) {
		long_press=long_press_dw;
	}
#ifdef DEBUG
	std::wcerr<<L"KEY_LONGPRESSENABLED="<<(long_press?L"TRUE":L"FALSE")<<std::endl;
#endif
		
	RegCloseKey(reg_key);
}

bool SuiteSettingsReg::RegSzQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &var) const
{
	DWORD buf_len;
	DWORD key_type;
	
	//If key not found or some other error occured - return to keep default var value
	if (RegQueryValueEx(reg_key, key_name, NULL, &key_type, NULL, &buf_len)!=ERROR_SUCCESS)
		return false;
	
	//If key is not of REG_SZ type - return to keep default var value 
	if (key_type!=REG_SZ)
		return false;
	
	//Returned buffer length is in bytes and because we use unicode build actual returned buffer type is wchar_t
	wchar_t data_buf[buf_len/2];
	
	//If for some reason we get read error - return to keep default var value
	if (RegQueryValueEx(reg_key, key_name, NULL, &key_type, (LPBYTE)&data_buf, &buf_len)!=ERROR_SUCCESS)
		return false;
	
	//If key is not of REG_SZ type or returned data is not NULL-terminated - return to keep default var value 
	if (key_type!=REG_SZ||data_buf[buf_len/2-1]!=L'\0')
		return false;
	
	var=data_buf;
	return true;
}

bool SuiteSettingsReg::RegDwordQueryValue(HKEY reg_key, const wchar_t* key_name, DWORD &var) const
{
	DWORD buf_len=sizeof(DWORD);
	DWORD key_type;
	DWORD data_buf;
	
	//If key not found or some other error occured - return to keep default var value
	if (RegQueryValueEx(reg_key, key_name, NULL, &key_type, (LPBYTE)&data_buf, &buf_len)!=ERROR_SUCCESS)
		return false;
	
	//If key is not of REG_DWORD type or returned len is not of DWORD size - return to keep default var value 
	if (key_type!=REG_DWORD||buf_len!=sizeof(DWORD))
		return false;
	
	var=data_buf;
	return true;
}

std::wstring SuiteSettingsReg::GetRegKey()
{
	return std::wstring(user?L"HKEY_CURRENT_USER":L"HKEY_LOCAL_MACHINE")+L"\\" SUITE_REG_PATH;
}

bool SuiteSettingsReg::SaveSettings()
{
	HKEY reg_key;
	
	if (stored) {
		if (RegOpenKeyEx(user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, KEY_SET_VALUE, &reg_key)!=ERROR_SUCCESS)
			return false;
	} else {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, SUITE_REG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, NULL)!=ERROR_SUCCESS)
			return false;
	}
	
	bool save_succeeded=true;
	
	if (RegSetValueEx(reg_key, KEY_ONHOTKEYCFGPATH, 0, REG_SZ, (BYTE*)shk_cfg_path.c_str(), (shk_cfg_path.length()+1)*2)!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (RegSetValueEx(reg_key, KEY_ONHOTKEYLONGPRESSCFGPATH, 0, REG_SZ, (BYTE*)lhk_cfg_path.c_str(), (lhk_cfg_path.length()+1)*2)!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (RegSetValueEx(reg_key, KEY_SNKPATH, 0, REG_SZ, (BYTE*)snk_path.c_str(), (snk_path.length()+1)*2)!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (RegSetValueEx(reg_key, KEY_HOTKEYSCANCODE, 0, REG_DWORD, (BYTE*)&binded_sc, sizeof(DWORD))!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (RegSetValueEx(reg_key, KEY_HOTKEYVIRTUALKEY, 0, REG_DWORD, (BYTE*)&binded_vk, sizeof(DWORD))!=ERROR_SUCCESS)
		save_succeeded=false;
	
	switch (mod_key) {
		case ModKeyType::CTRL_ALT:
			if (RegSetValueEx(reg_key, KEY_HOTKEYMODIFIERKEY, 0, REG_SZ, (BYTE*)VAL_CTRLALT, (wcslen(VAL_CTRLALT)+1)*2)!=ERROR_SUCCESS)
				save_succeeded=false;
			break;
		case ModKeyType::SHIFT_ALT:
			if (RegSetValueEx(reg_key, KEY_HOTKEYMODIFIERKEY, 0, REG_SZ, (BYTE*)VAL_SHIFTALT, (wcslen(VAL_SHIFTALT)+1)*2)!=ERROR_SUCCESS)
				save_succeeded=false;
			break;
		case ModKeyType::CTRL_SHIFT:
			if (RegSetValueEx(reg_key, KEY_HOTKEYMODIFIERKEY, 0, REG_SZ, (BYTE*)VAL_CTRLSHIFT, (wcslen(VAL_CTRLSHIFT)+1)*2)!=ERROR_SUCCESS)
				save_succeeded=false;
			break;
	}
	
	DWORD long_press_dw=long_press?1:0;
	if (RegSetValueEx(reg_key, KEY_LONGPRESSENABLED, 0, REG_DWORD, (BYTE*)&long_press_dw, sizeof(DWORD))!=ERROR_SUCCESS)
		save_succeeded=false;
	
	RegCloseKey(reg_key);
	
	if (!stored&&save_succeeded)
		stored=true;
	
	return save_succeeded;
}
