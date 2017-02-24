#include "SuiteSettings.h"
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include <cstdlib>
#include <cwchar>
#include <shlobj.h>

#ifdef DEBUG
#include <iostream>
#endif

#define CHG_ONHOTKEYCFGPATH				(1<<0)
#define CHG_ONHOTKEYLONGPRESSCFGPATH	(1<<1)
#define	CHG_SNKPATH						(1<<2)
#define CHG_HOTKEYSCANCODE				(1<<3)
#define CHG_HOTKEYVIRTUALKEY			(1<<4)
#define CHG_HOTKEYMODIFIERKEY			(1<<5)
#define CHG_LONGPRESSENABLED			(1<<6)
#define CHG_ALLKEYS	(CHG_ONHOTKEYCFGPATH|CHG_ONHOTKEYLONGPRESSCFGPATH|CHG_SNKPATH|CHG_HOTKEYSCANCODE|CHG_HOTKEYVIRTUALKEY|CHG_HOTKEYMODIFIERKEY|CHG_LONGPRESSENABLED)

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

#define SUITE_REG_PATH			L"Software\\SnK HotkeySuite"	//Registry key path passed as lpSubKey param to functions like RegOpenKeyEx, hive-independent
#define SUITE_INI_SECTION		L"HotkeySuite"					//Section name in ini file, no restrictions
#define SUITE_APPDATA_DIR		L"SnK HotkeySuite\\"			//Shouldn't start with backslash, but should end with backslash, can have any number of subdirectories, will be prepedned with APPDATA path

extern pSHGetSpecialFolderPath fnSHGetSpecialFolderPath;

SuiteSettings::SuiteSettings(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &snk_path):
	long_press(false), mod_key(ModKeyType::CTRL_ALT), binded_key{DEFAULT_VK /* vk */, DEFAULT_SC /* sc */, DEFAULT_EXT /* ext */}, initial_hkl(GetKeyboardLayout(0)), stored(false), changed(0),
	shk_cfg_path(shk_cfg_path), lhk_cfg_path(lhk_cfg_path), snk_path(snk_path)
{
	SetEnvironmentVariable(L"HS_EXE_PATH", GetExecutableFileName(L"").c_str());
#ifdef DEBUG
	std::wcerr<<L"SET HS_EXE_PATH="<<GetExecutableFileName(L"")<<std::endl;
#endif
}

SuiteSettings::SuiteSettings():
	SuiteSettings(DEFAULT_SHK_CFG_PATH, DEFAULT_LHK_CFG_PATH, DEFAULT_SNK_PATH)
{}

void SuiteSettings::SetLongPress(bool enabled) 
{ 
	changed|=CHG_LONGPRESSENABLED;
	long_press=enabled; 
}

void SuiteSettings::SetModKey(ModKeyType new_key) 
{ 
	changed|=CHG_HOTKEYMODIFIERKEY;
	mod_key=new_key; 
}

void SuiteSettings::SetBindedKey(BINDED_KEY new_key_binding) 
{ 
	changed|=CHG_HOTKEYVIRTUALKEY|CHG_HOTKEYSCANCODE;
	binded_key=new_key_binding; 
}

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

bool SuiteSettings::MapVkToSc(DWORD src_vk, BINDED_KEY &dst_key) const
{
	//MapVirtualKeyEx while being available since Win 95 expanded it's capabilities over time
	//Initially it supported only basic MAPVK_VK_TO_VSC, MAPVK_VSC_TO_VK and MAPVK_VK_TO_CHAR - it produced only ambidextrous mod VKs, single-byte SCs and didn't understand left/right-handed mod VKs and multi-byte SCs
	//Win NT added support for MAPVK_VSC_TO_VK_EX, function became able to produce left/right-handed mod VKs (using only MAPVK_VSC_TO_VK_EX) and uderstand left/right-handed mod VKs (in MAPVK_VK_TO_VSC and future MAPVK_VK_TO_VSC_EX)
	//Win Vista added support for MAPVK_VK_TO_VSC_EX, function became able to produce multi-byte SCs (using only MAPVK_VK_TO_VSC_EX) and understand multi-byte SCs (in MAPVK_VSC_TO_VK and MAPVK_VSC_TO_VK_EX)
	//In case of errors function returns 0 that is invalid value for both VK and SC

	UINT dst_sc;
	
	//First try to get multi-byte SC from whatever VK passed
	if ((dst_sc=MapVirtualKeyEx(src_vk, MAPVK_VK_TO_VSC_EX, initial_hkl))) {
		if (HIBYTE(dst_sc)==0xE0||HIBYTE(dst_sc)==0xE1) 
			dst_key.ext=true;
		else
			dst_key.ext=false;	
	//Previous attempt failed because of invalid VK or MAPVK_VK_TO_VSC_EX not being available (assuming latter)
	//Now try to get single-byte SC from whatever VK passed
	} else if ((dst_sc=MapVirtualKeyEx(src_vk, MAPVK_VK_TO_VSC, initial_hkl))) { 
		dst_key.ext=false;
	//Previous attempt failed because of invalid VK or function not being able to understand left/right-handed mod VKs
	//Check if latter was the case and repeat attempt with matching ambidextrous mod VK
	//N.B.: left/right-handed mod VK to ambidextrous mod VK conversion trick is the same one that MapVirtualKeyEx uses internally
	} else if (src_vk>=VK_LSHIFT&&src_vk<=VK_RMENU&&(dst_sc=MapVirtualKeyEx(((src_vk-VK_LSHIFT)/2+VK_SHIFT), MAPVK_VK_TO_VSC, initial_hkl))) {
		dst_key.ext=false;
	//We have invalid VK, now it's official, so fail miserably
	} else return false;
	
	dst_key.sc=LOBYTE(dst_sc);
	return true;
}

bool SuiteSettings::MapScToVk(DWORD src_sc, BINDED_KEY &dst_key) const
{
	//See notes about MapVirtualKeyEx in MapVkToSc above
	
	UINT dst_vk;
	
	//First try to get left/right-handed VK from whatever SC passed
	if ((dst_vk=MapVirtualKeyEx(src_sc, MAPVK_VSC_TO_VK_EX, initial_hkl))) {
	//Previous attempt failed because of invalid SC or MAPVK_VSC_TO_VK_EX not being available or function not being able to understand multi-byte SCs
	//Check if latter was the case and try with single-byte SC
	} else if (((src_sc&~(DWORD)0xFF)==0xE000||(src_sc&~(DWORD)0xFF)==0xE100)&&(src_sc=src_sc&0xFF, dst_vk=MapVirtualKeyEx(src_sc, MAPVK_VSC_TO_VK_EX, initial_hkl))) { 
	//Previous attempt failed because of invalid SC or MAPVK_VSC_TO_VK_EX not being available
	//Assuming latter and try to get ambidextrous VK with single-byte SC 
	//N.B.: src_sc is now single-byte or invalid after previous if-statement and if MAPVK_VSC_TO_VK_EX is not supported function won't understand multi-byte SCs anyway
	} else if ((dst_vk=MapVirtualKeyEx(src_sc, MAPVK_VSC_TO_VK, initial_hkl))) { 
	//We have invalid SC, now it's official, so fail miserably
	} else return false;
	
	dst_key.vk=LOBYTE(dst_vk);
	return true;
}

//------------------------------ INI -------------------------------

SuiteSettingsIni::SuiteSettingsIni(const std::wstring &shk_cfg_path, const std::wstring &lhk_cfg_path, const std::wstring &abs_ini_path, const std::wstring &ini_section):
	SuiteSettings(shk_cfg_path, lhk_cfg_path, L"%HS_EXE_PATH%\\" DEFAULT_SNK_PATH), ini_path(abs_ini_path), ini_section(ini_section)
{
	//We don't check if path is absolute because this constructor is protected and and it is by design that passed path should be absolute file path or empty string
	
	//Empty paths and sections are not allowed
	if (ini_path.empty()||ini_section.empty())
		return;
	
	//Actually this branch should always work because, once again, passed path should be absolute file path or empty string
	size_t last_backslash;
	if ((last_backslash=ini_path.find_last_of(L'\\'))!=std::wstring::npos) {
		SetEnvironmentVariable(L"HS_INI_PATH", ini_path.substr(0, last_backslash).c_str());
#ifdef DEBUG
		std::wcerr<<L"SET HS_INI_PATH="<<ini_path.substr(0, last_backslash).c_str()<<std::endl;
#endif
	}
	
	//Leave default values and mark all settings as changed if settings are not stored in ini file
	if (!CheckIfIniStored(ini_path, ini_section)) {
		changed=CHG_ALLKEYS;
		return;
	}
	
	stored=true;
		
	IniSzQueryValue(KEY_ONHOTKEYCFGPATH, this->shk_cfg_path);
	IniSzQueryValue(KEY_ONHOTKEYLONGPRESSCFGPATH, this->lhk_cfg_path);
	IniSzQueryValue(KEY_SNKPATH, snk_path);
	
	DWORD binded_sc;
	DWORD binded_vk;
	bool sc_found=IniDwordQueryValue(KEY_HOTKEYSCANCODE, binded_sc);
	bool vk_found=IniDwordQueryValue(KEY_HOTKEYVIRTUALKEY, binded_vk);
	if (sc_found) {
		if (HIBYTE(binded_sc)==0xE0||HIBYTE(binded_sc)==0xE1) 
			binded_key.ext=true;
		else
			binded_key.ext=false;	
		binded_key.sc=LOBYTE(binded_sc);
		//If SC was in ini file but VK wasn't - make VK from SC
		if (!vk_found) MapScToVk(binded_sc, binded_key);
	}
	if (vk_found) {
		binded_key.vk=LOBYTE(binded_vk);
		//If VK was in ini file but SC wasn't - make SC from VK
		if (!sc_found) MapVkToSc(binded_vk, binded_key);
	}
	//In case if both VK and SC wern't found - default values will be kept
	
	std::wstring mod_key_str;
	IniSzQueryValue(KEY_HOTKEYMODIFIERKEY, mod_key_str);
	if (!mod_key_str.compare(VAL_CTRLALT)) {
		mod_key=ModKeyType::CTRL_ALT;
	} else if (!mod_key_str.compare(VAL_SHIFTALT)) {
		mod_key=ModKeyType::SHIFT_ALT;
	} else if (!mod_key_str.compare(VAL_CTRLSHIFT)) {
		mod_key=ModKeyType::CTRL_SHIFT;
	}
	
	DWORD long_press_dw;
	if (IniDwordQueryValue(KEY_LONGPRESSENABLED, long_press_dw)) {
		long_press=long_press_dw;
	}
}

SuiteSettingsIni::SuiteSettingsIni(const std::wstring &rel_ini_path):
	SuiteSettingsIni(L"%HS_INI_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_INI_PATH%\\" DEFAULT_LHK_CFG_PATH, GetFullPathNameWrapper(rel_ini_path), SUITE_INI_SECTION)
{}

SuiteSettingsIni::SuiteSettingsIni():
	SuiteSettingsIni(L"%HS_EXE_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_EXE_PATH%\\" DEFAULT_LHK_CFG_PATH, GetExecutableFileName(L"\\" DEFAULT_INI_PATH), SUITE_INI_SECTION)
{}

bool SuiteSettingsIni::IniSzQueryValue(const wchar_t* key_name, std::wstring &var) const
{
	wchar_t data_buf[MAX_PATH+1];	//One more character is added so not to trigger ERROR_MORE_DATA if read string fits exactly in MAX_PATH
	
	//GetPrivateProfileString have an interesting behaviour
	//In case of any error (file not found, section not found, key not found) lpDefault will be copied to lpReturnedString and returned size will be set accordingly (length of lpDefault) and GetLastError will return ERROR_SUCCESS
	//If lpReturnedString buffer can't hold all of read string - it will be truncated and NULL-terminated and GetLastError will return some error
	//Setting lpDefault to NULL merely results in empty string being used for lpDefault
	
	if (GetPrivateProfileString(ini_section.c_str(), key_name, NULL, data_buf, MAX_PATH+1, ini_path.c_str())&&GetLastError()==ERROR_SUCCESS) {
		var=data_buf;
#ifdef DEBUG
		std::wcerr<<key_name<<L"="<<std::hex<<data_buf<<std::endl;
#endif
		return true;
	} else
		return false;
}

bool SuiteSettingsIni::IniDwordQueryValue(const wchar_t* key_name, DWORD &var) const
{
	wchar_t data_buf[15]; //This buffer size can hold dec, hex or oct representation of any signed or unsigned long, including minus sign and NULL terminator, while not triggering ERROR_MORE_DATA
	
	//GetPrivateProfileInt understands only decimals so using custom version for reading DWORDs
	//For more on GetPrivateProfileString behaviour see IniSzQueryValue function
	
	if (GetPrivateProfileString(ini_section.c_str(), key_name, NULL, data_buf, 15, ini_path.c_str())&&GetLastError()==ERROR_SUCCESS) {
		wchar_t* non_ul_char;
		DWORD dw_buf=wcstoul(data_buf, &non_ul_char, 0);
		//endptr will be set to the first non-integer character of str
		//So if it's points to '\0' - str is empty or doesn't have any illegal characters
		//If str doesn't have any illegal characters but doesn't fit into DWORD - errno is set to ERANGE
		if (errno!=ERANGE&&*non_ul_char==L'\0') {
			var=dw_buf;
#ifdef DEBUG
			std::wcerr<<key_name<<L"="<<std::hex<<DwordToHexString(dw_buf, 8)<<std::endl;
#endif
			return true;
		}
	}		
	
	return false;
}

bool SuiteSettingsIni::CheckIfIniStored(const std::wstring &path, const std::wstring &section)
{
	//Check if file exists
	DWORD dwAttrib=GetFileAttributes(path.c_str());
	if (dwAttrib==INVALID_FILE_ATTRIBUTES||(dwAttrib&FILE_ATTRIBUTE_DIRECTORY))
		return false;
	
	//Check if section exists
	//If lpKeyName is NULL GetPrivateProfileString copies all keys for lpAppName section to lpReturnedString buffer
	//If buffer is too small - returned list is truncated, terminated with two NULLs, returned size is nSize-2 and GetLastError returns some error
	//If lpAppName doesn't exists - returned size is 0 and GetLastError returns some error
	//If lpAppName exists but empty - returned size is 0 but GetLastError returns ERROR_SUCCESS
	wchar_t section_test_buf[8];	//8 - some dummy buffer size large enough so not to trigger false negative result if section exists and not empty
	if (GetPrivateProfileString(section.c_str(), NULL, NULL, section_test_buf, 8, path.c_str())==0&&GetLastError()!=ERROR_SUCCESS)
		return false;
	
	return true;
}

std::wstring SuiteSettingsIni::GetFullPathNameWrapper(const std::wstring &rel_path)
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
	//Empty paths and sections are not allowed
	if (ini_path.empty()||ini_section.empty())
		return false;

	//WritePrivateProfileString can create file if it doesn't exists but only if all the directories in file path exist
	
	bool save_succeeded=true;
	
	if (changed&CHG_ONHOTKEYCFGPATH&&!WritePrivateProfileString(ini_section.c_str(), KEY_ONHOTKEYCFGPATH, shk_cfg_path.c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (changed&CHG_ONHOTKEYLONGPRESSCFGPATH&&!WritePrivateProfileString(ini_section.c_str(), KEY_ONHOTKEYLONGPRESSCFGPATH, lhk_cfg_path.c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (changed&CHG_SNKPATH&&!WritePrivateProfileString(ini_section.c_str(), KEY_SNKPATH, snk_path.c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (changed&CHG_HOTKEYSCANCODE&&!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYSCANCODE, DwordToHexString((binded_key.ext?0xE000:0x0)|binded_key.sc, 8).c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (changed&CHG_HOTKEYVIRTUALKEY&&!WritePrivateProfileString(ini_section.c_str(), KEY_HOTKEYVIRTUALKEY, DwordToHexString(binded_key.vk, 8).c_str(), ini_path.c_str()))
		save_succeeded=false;

	if (changed&CHG_HOTKEYMODIFIERKEY)
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
	
	if (changed&CHG_LONGPRESSENABLED) {
		DWORD long_press_dw=long_press?1:0;
		if (!WritePrivateProfileString(ini_section.c_str(), KEY_LONGPRESSENABLED, DwordToHexString(long_press_dw, 8).c_str(), ini_path.c_str()))
			save_succeeded=false;
	}
	
	if (save_succeeded) {
		stored=true;
		changed=0;
	}
	
	return save_succeeded;
}

//------------------------------ SECTION -------------------------------

SuiteSettingsSection::SuiteSettingsSection(const std::wstring &ini_section):
	SuiteSettingsIni(std::wstring(L"%HS_EXE_PATH%\\")+StringToLower(ini_section)+L"_" DEFAULT_SHK_CFG_PATH, std::wstring(L"%HS_EXE_PATH%\\")+StringToLower(ini_section)+L"_" DEFAULT_LHK_CFG_PATH, GetExecutableFileName(L"\\" DEFAULT_INI_PATH), ini_section)
{}

//------------------------------ APPDATA -------------------------------

SuiteSettingsAppData::SuiteSettingsAppData():
	SuiteSettingsIni(L"%HS_INI_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_INI_PATH%\\" DEFAULT_LHK_CFG_PATH, GetIniAppDataPath(Location::AUTO), SUITE_INI_SECTION)
{}

SuiteSettingsAppData::SuiteSettingsAppData(bool current_user):
	SuiteSettingsIni(L"%HS_INI_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_INI_PATH%\\" DEFAULT_LHK_CFG_PATH, GetIniAppDataPath(current_user?Location::CURRENT_USER:Location::ALL_USERS), SUITE_INI_SECTION)
{}

std::wstring SuiteSettingsAppData::GetIniAppDataPath(Location loc)
{
	//We have three functions that can retrieve APPDATA path: SHGetSpecialFolderPath (shell32 v4.71+), SHGetFolderPath (v5.0+) and SHGetKnownFolderPath (v6.0+)
	//By using SHGetSpecialFolderPath we can ensure that APPDATA path could be retreived on Win 98+ and Win 2000+ out of the box, and on Win 95 and Win NT4 with IE 4.0 installed
	//N.B.: Actually SHGetSpecialFolderPathW (i.e. UNICODE version) available since  v4.0 but is exported only by ordinal (175) in pre v4.71 versions
	if (fnSHGetSpecialFolderPath) {
		//SHGetSpecialFolderPath(NULL, buffer, CSIDL, TRUE) is equivalent to SHGetFolderPath(NULL, CSIDL|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, buffer)
		//Though SHGetSpecialFolderPath is considered deprecated it is still available even in most recent versions of Windows for backward compatibility
		auto fnCheckIfAppDataStored=[](int csidl, std::wstring& ret_path_str)->bool{
			//SHGetSpecialFolderPath's lpszPath should be MAX_PATH in length
			wchar_t path_buf[MAX_PATH];
			
			auto fnGetCommonAppDataPath=[path_buf, csidl]()->bool{
				bool ret=false;
				if (csidl==CSIDL_COMMON_APPDATA) {
					//In case of CSIDL_COMMON_APPDATA that is not available on shell32 v4.0-4.72 we still have the power to save the show
					//Windows stores common special folder paths under HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders
					//And even if CSIDL may not be known to this version of SHGetSpecialFolderPath, it's path can still be found in the registry
					//So why do we need SHGetSpecialFolderPath in first place if special folder paths are actually stored in the registry?
					//For two reasons (assuming that CSIDL is known to this version of SHGetSpecialFolderPath): 
					// Some CSIDLs are never stored in the registry (not CSIDL_COMMON_APPDATA case) - SHGetSpecialFolderPath gets paths elsewhere for these CSIDLs
					// Registry value may not be found in the registry (for one or another reason) - that's kind of error and in this case SHGetSpecialFolderPath returns localized CSIDL name appended to windows directory path
					//So we'll try to find CSIDL_COMMON_APPDATA in the registry where it is stored under value named (always unlocalized) "Common AppData"
					//If this value not found, we won't be able to emulate SHGetSpecialFolderPath error handling routine because localized CSIDL name won't be present in string table for this version of shell32
					HKEY reg_key;
					if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_READ, &reg_key)==ERROR_SUCCESS) {
						DWORD buf_len=MAX_PATH*sizeof(wchar_t);	//Buffer length is in bytes and because we use unicode build actual returned buffer type is wchar_t
						DWORD key_type;
						//If returned data is not NULL-terminated or not of single-string type - it's also an error 
						if (RegQueryValueEx(reg_key, L"Common AppData", NULL, &key_type, (LPBYTE)path_buf, &buf_len)==ERROR_SUCCESS&&(key_type==REG_EXPAND_SZ||key_type==REG_SZ)&&path_buf[buf_len/sizeof(wchar_t)-1]==L'\0')
							ret=true;
						RegCloseKey(reg_key);
					}
				}
				return ret;
			};
			
			//If SHGetSpecialFolderPath and fnGetCommonAppDataPath fails - ret_path_str not modified
			//If SHGetSpecialFolderPath or fnGetCommonAppDataPath succeeds - ret_path_str will contain valid path even if CheckIfIniStored fails
			if ((fnSHGetSpecialFolderPath(NULL, path_buf, csidl, TRUE)==TRUE)||fnGetCommonAppDataPath()) {
				ret_path_str=path_buf;
				
				//Only in case of drive's root returned string ends with backslash
				if (ret_path_str.back()!=L'\\')
					ret_path_str+=L'\\';
				
				ret_path_str+=SUITE_APPDATA_DIR DEFAULT_INI_PATH;
				return CheckIfIniStored(ret_path_str, SUITE_INI_SECTION);
			} else
				return false;
		};
		
		//Using CSIDL_APPDATA for user APPDATA instead of CSIDL_LOCAL_APPDATA because former is available from shell32 v4.71+ while latter from v5.0+
		//And because app configs should be roamed
		//System APPDATA is CSIDL_COMMON_APPDATA (shell32 v5.0+)
		//N.B.: Contradicting to what MSDN says, CSIDL_APPDATA is available since v4.0, but not CSIDL_LOCAL_APPDATA or CSIDL_COMMON_APPDATA - they are really only available since v5.0
		
		std::wstring user_path;		
		if (fnCheckIfAppDataStored(CSIDL_APPDATA, user_path)&&loc==Location::AUTO)
			loc=Location::CURRENT_USER;
		
		std::wstring system_path;		
		if (fnCheckIfAppDataStored(CSIDL_COMMON_APPDATA, system_path)&&loc==Location::AUTO)
			loc=Location::ALL_USERS;
		
		switch (loc) {
			case Location::CURRENT_USER:
			case Location::AUTO:
				return user_path;
			case Location::ALL_USERS:
				return system_path;
		}
	} else	
		return L"";
}

bool SuiteSettingsAppData::SaveSettings()
{
	//Don't bother creating directory tree if settings are alredy stored
	if (!stored) {
		if (!CreateDirTree(ini_path))	//SaveSettingsfails if directory tree wasn't created
			return false;
	}
	
	return SuiteSettingsIni::SaveSettings();
}

//------------------------------ REGISTRY -------------------------------

SuiteSettingsReg::SuiteSettingsReg(Hive hive):
	SuiteSettings(L"%HS_EXE_PATH%\\" DEFAULT_SHK_CFG_PATH, L"%HS_EXE_PATH%\\" DEFAULT_LHK_CFG_PATH, L"%HS_EXE_PATH%\\" DEFAULT_SNK_PATH), hive(hive)
{
	HKEY reg_key;
	
	//Leave default values and mark all settings as changed if reg key wasn't found in both HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE
	switch (hive) {
		case Hive::LOCAL_MACHINE:
		case Hive::CURRENT_USER:
			if (RegOpenKeyEx(hive==Hive::CURRENT_USER?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, KEY_READ, &reg_key)==ERROR_SUCCESS)
				stored=true;
			else {
				changed=CHG_ALLKEYS;
				return;
			}
			break;
		case Hive::AUTO:
			hive=Hive::CURRENT_USER;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, SUITE_REG_PATH, 0, KEY_READ, &reg_key)==ERROR_SUCCESS) {
				stored=true;
			} else {
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, KEY_READ, &reg_key)==ERROR_SUCCESS) {
					hive=Hive::LOCAL_MACHINE;
					stored=true;
				} else {
					changed=CHG_ALLKEYS;
					return;
				}
			}
			break;
	}
	
	RegSzQueryValue(reg_key, KEY_ONHOTKEYCFGPATH, shk_cfg_path);
	RegSzQueryValue(reg_key, KEY_ONHOTKEYLONGPRESSCFGPATH, lhk_cfg_path);
	RegSzQueryValue(reg_key, KEY_SNKPATH, snk_path);
	
	DWORD binded_sc;
	DWORD binded_vk;
	bool sc_found=RegDwordQueryValue(reg_key, KEY_HOTKEYSCANCODE, binded_sc);
	bool vk_found=RegDwordQueryValue(reg_key, KEY_HOTKEYVIRTUALKEY, binded_vk);
	if (sc_found) {
		if (HIBYTE(binded_sc)==0xE0||HIBYTE(binded_sc)==0xE1) 
			binded_key.ext=true;
		else
			binded_key.ext=false;	
		binded_key.sc=LOBYTE(binded_sc);
		//If SC was in registry but VK wasn't - make VK from SC
		if (!vk_found) MapScToVk(binded_sc, binded_key);
	}
	if (vk_found) {
		binded_key.vk=LOBYTE(binded_vk);
		//If VK was in registry but SC wasn't - make SC from VK
		if (!sc_found) MapVkToSc(binded_vk, binded_key);
	}
	//In case if both VK and SC wern't found - default values will be kept
	
	std::wstring mod_key_str;
	RegSzQueryValue(reg_key, KEY_HOTKEYMODIFIERKEY, mod_key_str);
	if (!mod_key_str.compare(VAL_CTRLALT)) {
		mod_key=ModKeyType::CTRL_ALT;
	} else if (!mod_key_str.compare(VAL_SHIFTALT)) {
		mod_key=ModKeyType::SHIFT_ALT;
	} else if (!mod_key_str.compare(VAL_CTRLSHIFT)) {
		mod_key=ModKeyType::CTRL_SHIFT;
	}
	
	DWORD long_press_dw;
	if (RegDwordQueryValue(reg_key, KEY_LONGPRESSENABLED, long_press_dw)) {
		long_press=long_press_dw;
	}
		
	RegCloseKey(reg_key);
}

SuiteSettingsReg::SuiteSettingsReg():
	SuiteSettingsReg(Hive::AUTO)
{}

SuiteSettingsReg::SuiteSettingsReg(bool current_user):
	SuiteSettingsReg(current_user?Hive::CURRENT_USER:Hive::LOCAL_MACHINE)
{}

bool SuiteSettingsReg::RegSzQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &var) const
{
	DWORD buf_len;
	DWORD key_type;
	
	//If key not found or some other error occured - return to keep default var value
	if (RegQueryValueEx(reg_key, key_name, NULL, &key_type, NULL, &buf_len)!=ERROR_SUCCESS)
		return false;
	
	//If key is not of REG_EXPAND_SZ type - return to keep default var value 
	if (key_type!=REG_EXPAND_SZ)
		return false;
	
	//Returned buffer length is in bytes and because we use unicode build actual returned buffer type is wchar_t
	wchar_t data_buf[buf_len/sizeof(wchar_t)];
	
	//If for some reason we get read error - return to keep default var value
	if (RegQueryValueEx(reg_key, key_name, NULL, &key_type, (LPBYTE)data_buf, &buf_len)!=ERROR_SUCCESS)
		return false;
	
	//If key is not of REG_EXPAND_SZ type or returned data is not NULL-terminated - return to keep default var value 
	if (key_type!=REG_EXPAND_SZ||data_buf[buf_len/sizeof(wchar_t)-1]!=L'\0')
		return false;
	
	var=data_buf;
#ifdef DEBUG
	std::wcerr<<key_name<<L"="<<data_buf<<std::endl;
#endif
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
#ifdef DEBUG
	std::wcerr<<key_name<<L"="<<std::hex<<DwordToHexString(data_buf, 8)<<std::endl;
#endif
	return true;
}

std::wstring SuiteSettingsReg::GetStoredLocation()
{
	return std::wstring(hive!=Hive::LOCAL_MACHINE?L"HKEY_CURRENT_USER":L"HKEY_LOCAL_MACHINE")+L"\\" SUITE_REG_PATH;
}

bool SuiteSettingsReg::SaveSettings()
{
	HKEY reg_key;
	
	if (stored) {
		if (RegOpenKeyEx(hive!=Hive::LOCAL_MACHINE?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, KEY_SET_VALUE, &reg_key)!=ERROR_SUCCESS)
			return false;
	} else {
		if (RegCreateKeyEx(hive!=Hive::LOCAL_MACHINE?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, SUITE_REG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, NULL)!=ERROR_SUCCESS)
			return false;
	}
	
	bool save_succeeded=true;
	
	if (changed&CHG_ONHOTKEYCFGPATH&&RegSetValueEx(reg_key, KEY_ONHOTKEYCFGPATH, 0, REG_EXPAND_SZ, (BYTE*)shk_cfg_path.c_str(), (shk_cfg_path.length()+1)*sizeof(wchar_t))!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (changed&CHG_ONHOTKEYLONGPRESSCFGPATH&&RegSetValueEx(reg_key, KEY_ONHOTKEYLONGPRESSCFGPATH, 0, REG_EXPAND_SZ, (BYTE*)lhk_cfg_path.c_str(), (lhk_cfg_path.length()+1)*sizeof(wchar_t))!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (changed&CHG_SNKPATH&&RegSetValueEx(reg_key, KEY_SNKPATH, 0, REG_EXPAND_SZ, (BYTE*)snk_path.c_str(), (snk_path.length()+1)*sizeof(wchar_t))!=ERROR_SUCCESS)
		save_succeeded=false;
	
	if (changed&CHG_HOTKEYSCANCODE) {
		DWORD binded_sc=(binded_key.ext?0xE000:0x0)|binded_key.sc;
		if (RegSetValueEx(reg_key, KEY_HOTKEYSCANCODE, 0, REG_DWORD, (BYTE*)&binded_sc, sizeof(DWORD))!=ERROR_SUCCESS)
			save_succeeded=false;
	}
	
	if (changed&CHG_HOTKEYVIRTUALKEY) {
		DWORD binded_vk=binded_key.vk;
		if (RegSetValueEx(reg_key, KEY_HOTKEYVIRTUALKEY, 0, REG_DWORD, (BYTE*)&binded_vk, sizeof(DWORD))!=ERROR_SUCCESS)
			save_succeeded=false;
	}
	
	if (changed&CHG_HOTKEYMODIFIERKEY)
		switch (mod_key) {
			case ModKeyType::CTRL_ALT:
				if (RegSetValueEx(reg_key, KEY_HOTKEYMODIFIERKEY, 0, REG_EXPAND_SZ, (BYTE*)VAL_CTRLALT, sizeof(VAL_CTRLALT))!=ERROR_SUCCESS)
					save_succeeded=false;
				break;
			case ModKeyType::SHIFT_ALT:
				if (RegSetValueEx(reg_key, KEY_HOTKEYMODIFIERKEY, 0, REG_EXPAND_SZ, (BYTE*)VAL_SHIFTALT, sizeof(VAL_SHIFTALT))!=ERROR_SUCCESS)
					save_succeeded=false;
				break;
			case ModKeyType::CTRL_SHIFT:
				if (RegSetValueEx(reg_key, KEY_HOTKEYMODIFIERKEY, 0, REG_EXPAND_SZ, (BYTE*)VAL_CTRLSHIFT, sizeof(VAL_CTRLSHIFT))!=ERROR_SUCCESS)
					save_succeeded=false;
				break;
		}
	
	if (changed&CHG_LONGPRESSENABLED) {
		DWORD long_press_dw=long_press?1:0;
		if (RegSetValueEx(reg_key, KEY_LONGPRESSENABLED, 0, REG_DWORD, (BYTE*)&long_press_dw, sizeof(DWORD))!=ERROR_SUCCESS)
			save_succeeded=false;
	}
	
	RegCloseKey(reg_key);
	
	if (save_succeeded) {
		stored=true;
		changed=0;
	}
	
	return save_succeeded;
}
