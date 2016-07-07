#include "SuiteSettings.h"
#include <cwchar>

#ifdef DEBUG
#include <iostream>
#endif

#define DEFAULT_SHK_CFG_PATH	L"on_hotkey.cfg"
#define DEFAULT_LHK_CFG_PATH	L"on_hotkey_long_press.cfg"
#define DEFAULT_INI_PATH		L"HotkeySuite.ini"
#define DEFAULT_SNK_PATH		L"SnKh.exe"

#define SUITE_REG_PATH			L"Software\\SnK HotkeySuite"

SuiteSettings::SuiteSettings():
	long_press(false), mod_key(ModKeyType::CTRL_ALT), binded_vk(DEFAULT_VK), binded_sc(DEFAULT_SC), initial_hkl(GetKeyboardLayout(0)), valid(true),
	shk_cfg_path(DEFAULT_SHK_CFG_PATH), lhk_cfg_path(DEFAULT_LHK_CFG_PATH), snk_path(DEFAULT_SNK_PATH)
{
	wchar_t exe_path[MAX_PATH];
	DWORD ret_len=GetModuleFileName(NULL, exe_path, MAX_PATH);	//Passing NULL as hModule to get current exe path
	
	//GetModuleFileName returns 0 on error and nSize (MAX_PATH) if buffer is unsufficient
	if (ret_len&&ret_len<MAX_PATH) {
		//GetModuleFileName always returns module's full path (not some relative-to-something-path even if it was passed to CreateProcess in first place)
		//So instead of using _wsplitpath/_makepath or PathRemoveFileSpec, which have additional code to deal with relative paths, just use wcsrchr to find last backslash occurrence
		//Also PathRemoveFileSpec doesn't strip trailing slash if file is at drive's root which isn't the thing we want in environment variable
		if (wchar_t* last_backslash=wcsrchr(exe_path, L'\\'))
			*last_backslash=L'\0';
		
		SetEnvironmentVariable(L"HS_PATH", exe_path);
#ifdef DEBUG
		std::wcerr<<L"SET HS_PATH="<<exe_path<<std::endl;
#endif
	}
}
	
SuiteSettings::~SuiteSettings()
{}

std::wstring SuiteSettings::ExpandEnvironmentStringsWrapper(const std::wstring &path) 
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

SuiteSettingsReg::SuiteSettingsReg():
	SuiteSettings()
{
	LoadSettingsFromReg();
}

SuiteSettingsReg::~SuiteSettingsReg()
{}

SuiteSettingsIni::SuiteSettingsIni():
	SuiteSettings(), ini_path(DEFAULT_INI_PATH)
{
	LoadSettingsFromIni();
}

SuiteSettingsIni::SuiteSettingsIni(const std::wstring &ini_path):
	SuiteSettings(), ini_path(ini_path)
{
	LoadSettingsFromIni();
}

SuiteSettingsIni::~SuiteSettingsIni()
{}

bool SuiteSettingsReg::RegSzQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &var)
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

bool SuiteSettingsReg::RegDwordQueryValue(HKEY reg_key, const wchar_t* key_name, DWORD &var)
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

void SuiteSettingsReg::LoadSettingsFromReg()
{
	HKEY reg_key;
	
	//Leave default values if reg key wasn't found in both HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE
	if (RegOpenKeyEx(HKEY_CURRENT_USER, SUITE_REG_PATH, 0, KEY_READ, &reg_key)!=ERROR_SUCCESS&&
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, KEY_READ, &reg_key)!=ERROR_SUCCESS)
		return;
		
	RegSzQueryValue(reg_key, L"OnHotkeyCfgPath", shk_cfg_path);
	RegSzQueryValue(reg_key, L"OnHotkeyLongPressCfgPath", lhk_cfg_path);
	RegSzQueryValue(reg_key, L"SnkPath", snk_path);
	
	bool sc_found=RegDwordQueryValue(reg_key, L"HotkeyScancode", binded_sc);
	bool vk_found=RegDwordQueryValue(reg_key, L"HotkeyVirtualKey", binded_vk);
	if (sc_found&&!vk_found) {
		//If SC was in registry but VK wasn't - make VK from SC
		binded_vk=MapVirtualKeyEx(binded_sc, MAPVK_VSC_TO_VK, initial_hkl);
	}
	if (!sc_found&&vk_found) {
		//If VK was in registry but SC wasn't - make SC from VK
		binded_sc=MapVirtualKeyEx(binded_vk, MAPVK_VK_TO_VSC, initial_hkl);
	}
	//In case if both VK and SC wern't found - default values will be kept
	
	std::wstring mod_key_str;
	RegSzQueryValue(reg_key, L"HotkeyModifierKey", mod_key_str);
	if (!mod_key_str.compare(L"Ctrl+Alt")) {
		mod_key=ModKeyType::CTRL_ALT;
	} else if (!mod_key_str.compare(L"Shift+Alt")) {
		mod_key=ModKeyType::SHIFT_ALT;
	} else if (!mod_key_str.compare(L"Ctrl+Shift")) {
		mod_key=ModKeyType::CTRL_SHIFT;
	}
	
	DWORD long_press_dw;
	if (RegDwordQueryValue(reg_key, L"LongPressEnabled", long_press_dw)) {
		long_press=long_press_dw;
	}
		
	RegCloseKey(reg_key);
}

void SuiteSettingsReg::SaveSettings()
{}

void SuiteSettingsIni::LoadSettingsFromIni()
{}

void SuiteSettingsIni::SaveSettings()
{}
