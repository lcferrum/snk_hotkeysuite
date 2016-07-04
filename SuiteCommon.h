#ifndef SUITECOMMON_H
#define SUITECOMMON_H

#include <string>
#include <windows.h>

#define DEFAULT_VK	VK_BACK	//Backspace vk and sc
#define DEFAULT_SC	0x0E

enum class HkStrType:char {FULL, MOD_KEY, VK};
enum class ModKeyType:unsigned char {CTRL_ALT=0, SHIFT_ALT, CTRL_SHIFT, DONT_CARE=CTRL_ALT};

std::wstring GetHotkeyString(ModKeyType mod_key, DWORD vk, DWORD sc, HkStrType type, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);

#endif //SUITECOMMON_H
