#ifndef SUITECOMMON_H
#define SUITECOMMON_H

#include <string>
#include <windows.h>

#define DEFAULT_VK	VK_BACK	//Backspace vk and sc
#define DEFAULT_SC	0x0E

enum class HkStrType:char {FULL, MOD_KEY, VK};
enum class ModKeyType:unsigned char {CTRL_ALT=0, SHIFT_ALT, CTRL_SHIFT, DONT_CARE=CTRL_ALT};

std::wstring DwordToHexString(DWORD vk, int hex_width=2);	//hex_width is minimum width of hex string in characters not includin '0x' prefix
std::wstring GetHotkeyString(ModKeyType mod_key, DWORD vk, DWORD sc, HkStrType type, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);
std::wstring GetExecutableFileName(const wchar_t* replace_fname=NULL);	//If replace_fname is not NULL - replaces file name (including preceding backslash) in returned path with replace_fname

#endif //SUITECOMMON_H
