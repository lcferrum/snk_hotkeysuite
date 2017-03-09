#ifndef SUITECOMMON_H
#define SUITECOMMON_H

#include <string>
#include <windows.h>

#define DEFAULT_VK			VK_BACK	//Backspace vk and sc
#define DEFAULT_SC			0x0E
#define DEFAULT_EXT			false

#define DLGBX_FN_INV_PARAM	0
#define DLGBX_FN_FAILED		-1

#define ERR_SUITE			0x0100
#define ERR_SUITEMAIN		0x0200
#define ERR_SUITEEXTREL		0x0300

typedef struct {
	union {
		struct {
			BYTE vk;
			BYTE sc;
			bool ext;
		};
		DWORD tuple;
	};
} BINDED_KEY;

enum class HkStrType:char {FULL, MOD_KEY, VK};
enum class ModKeyType:unsigned char {CTRL_ALT=0, SHIFT_ALT, CTRL_SHIFT, DONT_CARE=CTRL_ALT};

std::wstring DwordToHexString(DWORD dw, int hex_width);	//hex_width is minimum width of hex string in characters not including '0x' prefix
std::wstring GetHotkeyWarning(ModKeyType mod_key, BINDED_KEY key, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL, const wchar_t* defval=NULL);
std::wstring GetHotkeyString(ModKeyType mod_key, BINDED_KEY key, HkStrType type, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);
std::wstring GetExecutableFileName(const wchar_t* replace_fname=NULL);	//If replace_fname is not NULL - replaces file name (including preceding backslash) in returned path with replace_fname
std::wstring StringToLower(std::wstring str);
std::wstring QuoteArgument(const wchar_t* arg);
bool CreateDirTree(const std::wstring trg_pth);
void ErrorMessage(const wchar_t* err_msg);

#endif //SUITECOMMON_H
