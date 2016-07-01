#ifndef SUITECOMMON_H
#define SUITECOMMON_H

#include "SuiteSettings.h"
#include <string>
#include <windows.h>

std::wstring GetHexVk(DWORD vk);
std::wstring GetOemChar(wchar_t def_char, wchar_t alt_char, DWORD oem_vk);

enum class GetHotkeyStringType:char {FULL, MOD_KEY, VK};
std::wstring GetHotkeyString(SuiteSettings::ModKeyType mod_key, DWORD vk, GetHotkeyStringType type, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);

#endif //SUITECOMMON_H
