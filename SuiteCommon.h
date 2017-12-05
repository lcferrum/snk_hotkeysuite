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
#define ERR_SUITEEXTREL		0x0400
#define ERR_RESTART			0x8001
#define ERR_ELEVATE			0x8002

#define WM_HSTNAICO			(WM_USER+1)
#define WM_BINDSC			(WM_USER+2)

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

enum class ModKeyType:unsigned char {CTRL_ALT, SHIFT_ALT, CTRL_SHIFT};

#ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
//See cpp file for comments
namespace hack {
	std::wstring to_wstring(DWORD value);
}
#define to_wstring_wrapper(x)	hack::to_wstring(x)
#else
#define to_wstring_wrapper(x)	std::to_wstring(x)
#endif

std::wstring DwordToHexString(DWORD dw, int hex_width=0);	//hex_width (capped at 8) is minimum width of hex string in characters not including '0x' prefix
std::wstring GetHotkeyWarning(ModKeyType mod_key, BINDED_KEY key, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL, const wchar_t* defval=NULL);
std::wstring GetHotkeyString(BINDED_KEY key, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);
std::wstring GetHotkeyString(ModKeyType mod_key, BINDED_KEY key, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);
std::wstring GetExecutableFileName(const wchar_t* replace_fname=NULL);	//If replace_fname is not NULL - replaces file name (including preceding backslash) in returned path with replace_fname
std::wstring GetFullPathNameWrapper(const std::wstring &rel_path);
std::wstring GetDirPath(const std::wstring &trg_pth);	//Returned path doesn't contain trailing backslash (if it's not a disc root)
std::wstring QuoteArgument(const wchar_t* arg);
std::wstring SearchPathWrapper(const wchar_t* fname, const wchar_t* spath, const wchar_t* ext);
bool CheckIfFileExists(const std::wstring &fpath);
bool CreateDirTreeForFile(const std::wstring trg_pth);	//Allowed trg_pth: absolute file path, absolute directory path w/ trailing backslash or empty string
void ErrorMessage(const wchar_t* err_msg);

#endif //SUITECOMMON_H
