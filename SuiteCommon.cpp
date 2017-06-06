#include "SuiteCommon.h"
#include "SuiteVersion.h"
#include "SuiteExterns.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <commctrl.h>

extern pTaskDialog fnTaskDialog;
extern pSHCreateDirectoryEx fnSHCreateDirectoryEx;
extern pSHCreateDirectory fnSHCreateDirectory;

#ifdef __clang__
//Obscure clang++ bug - it reports "multiple definition" of std::setfill() and std::wstring() when statically linking with libstdc++
//Observed on LLVM 3.6.2 with MinGW 4.7.2
//This is a fix for the bug
extern template std::_Setfill<wchar_t> std::setfill(wchar_t);									//caused by use of std::setfill(wchar_t)
extern template std::wstring::basic_string(wchar_t*, wchar_t*, std::allocator<wchar_t> const&);	//caused by use of std::wstring(wchar_t*, wchar_t*)
#endif

inline std::wstring ToWstringFormatted(size_t count, const wchar_t *format, ...)
{
	wchar_t buf[count];
	va_list args;
	va_start(args, format);
	//_vsnwprintf differs from vswprintf:
	// vswprintf guarantees that buffer will be NULL-terminated and returns error if string length >= count
	// _vsnwprintf NULL-terminates only if there is room left for NULL terminator and returns error if string length > count
	//Both functions return number of actually written characters not including NULL-terminator if no error occured
	//Function uses _vsnwprintf to be independent of vswprintf availability
	//std::wstring(first_iter, last_iter) constructor is used, which copies [first, last) characters, so we don't have to worry if string is not NULL-terminated
	const int len=_vsnwprintf(buf, count, format, args);
	va_end(args);
	return std::wstring(buf, buf+len);
}

#ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
//Internally libstdc++ to_wstring implementation uses vswprintf
//Problem is that by C++ standard it should be defined as int(wchar_t*, size_t, const wchar_t*, va_list) but MSVCRT (which libstdc++ uses) exports it as int(wchar_t*, const wchar_t*, va_list)
//That's why to_wstring is guarded by !defined(_GLIBCXX_HAVE_BROKEN_VSWPRINTF) and is not available in MinGW (the same goes to Clang that uses MinGW's libstdc++)
//This is fixed in MinGW-w64 but needs patched headers or hack (i.e. reimplementation of to_wstring using _vsnwprintf) on MinGW/Clang
std::wstring hack::to_wstring(DWORD value)
{
	return ToWstringFormatted(10, L"%u", value);	//Maximum DWORD is 4294967295 - that is 10 characters and we don't need space for NULL-terminator because of _vsnwprintf
}
#endif

void ErrorMessage(const wchar_t* err_msg)
{
#ifdef DEBUG
	std::wcerr<<L"ERROR MESSAGE: "<<err_msg<<std::endl;
#endif
	std::wstring full_msg(L"Error: ");
	full_msg+=err_msg;
	if (fnTaskDialog) {
		int btn_clicked;
		fnTaskDialog(NULL, NULL, SNK_HS_TITLE, SNK_HS_TITLE L" encountered an error and will be closed", full_msg.c_str(), TDCBF_CLOSE_BUTTON, TD_ERROR_ICON, &btn_clicked);
	} else {
		full_msg+=L"\n\nPress 'OK' to close " SNK_HS_TITLE ".";
		MessageBox(NULL, full_msg.c_str(), SNK_HS_TITLE, MB_ICONERROR|MB_OK);
	}
}

std::wstring GetFullPathNameWrapper(const std::wstring &rel_path)
{
	wchar_t dummy_buf;
	wchar_t* fname_pos;

	//If returned length is 0 - it is error
	if (DWORD buf_len=GetFullPathName(rel_path.c_str(), 0, &dummy_buf, &fname_pos)) {
		wchar_t string_buf[buf_len];
		//Ensuring that returned length is expected length and resulting string contains file name (i.e. it's not directory)
		if (GetFullPathName(rel_path.c_str(), buf_len, string_buf, &fname_pos)+1<=buf_len&&fname_pos!=NULL) 
			return string_buf;
	}
	
	return L"";
}

std::wstring GetExecutableFileName(const wchar_t* replace_fname)
{
	wchar_t exe_path[MAX_PATH];
	DWORD ret_len=GetModuleFileName(NULL, exe_path, MAX_PATH);	//Passing NULL as hModule to get current exe path
	
	//GetModuleFileName returns 0 on error and nSize (MAX_PATH) if buffer is unsufficient
	if (ret_len&&ret_len<MAX_PATH) {
		//GetModuleFileName always returns module's full path (not some relative-to-something-path even if it was passed to CreateProcess in first place)
		//So instead of using _wsplitpath/_makepath or PathRemoveFileSpec, which have additional code to deal with relative paths, just use wcsrchr to find last backslash occurrence
		//Also PathRemoveFileSpec doesn't strip trailing backslash if file is at drive's root which isn't the thing we want in environment variable
		if (replace_fname) {
			if (wchar_t* last_backslash=wcsrchr(exe_path, L'\\')) {
				*last_backslash=L'\0';
				return std::wstring(exe_path)+replace_fname;
			}
		} else
			return exe_path;
	}
	
	return L"";
}

std::wstring GetDirPath(const std::wstring &trg_pth)
{
	size_t last_backslash;
	if ((last_backslash=trg_pth.find_last_of(L'\\'))!=std::wstring::npos)
		return trg_pth.substr(0, last_backslash);
	else
		return std::wstring();
}

bool CreateDirTreeForFile(const std::wstring trg_pth) 
{
	//Starting from shell32 v5.0 (Win 2000) we can use SHCreateDirectoryEx to build directory tree
	//Intresting thing is that there is SHCreateDirectory that does the same: it is exported as ordinal (165) from v4.0 and by name from v6.0
	//So we'll use SHCreateDirectoryEx where it is available and SHCreateDirectory otherwise

	if (trg_pth.empty())
		return false;
	
	std::wstring dir=GetDirPath(trg_pth);
	DWORD dir_attr=GetFileAttributes(dir.c_str());
 
	if (dir_attr==INVALID_FILE_ATTRIBUTES) {
		if (fnSHCreateDirectoryEx)
			return ERROR_SUCCESS==fnSHCreateDirectoryEx(NULL, dir.c_str(), NULL);
		else if (fnSHCreateDirectory)
			return ERROR_SUCCESS==fnSHCreateDirectory(NULL, dir.c_str());
		else
			return false;
	} else {
		return dir_attr&FILE_ATTRIBUTE_DIRECTORY;
	}
}

std::wstring DwordToHexString(DWORD dw, int hex_width)
{
	return ToWstringFormatted(10, L"0x%0*X", std::min(8, std::max(0, hex_width)), dw);	//Maximum DWORD is 0xFFFFFFFF - that is 10 characters and we don't need space for NULL-terminator because of _vsnwprintf
}

//Based on "Everyone quotes command line arguments the wrong way": 
// Written by Daniel Colascione <dancol@dancol.org>
// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
std::wstring QuoteArgument(const wchar_t* arg)
{
	if (!arg||!wcslen(arg)) {
		return L"\"\"";
	} else if (!wcspbrk(arg, L" \t\n\v\"")) {
		return arg;
	} else {
		std::wstring qarg=L"\"";
		for (;;) {
			size_t backslash_num=0;

			while (*arg!=L'\0'&&*arg==L'\\') {
				arg++;
				backslash_num++;
			}

			if (*arg==L'\0') {
				qarg.append(backslash_num*2, L'\\');
				break;
			} else if (*arg==L'"') {
				qarg.append(backslash_num*2+1, L'\\');
				qarg.push_back(*arg);
			} else {
				qarg.append(backslash_num, L'\\');
				qarg.push_back(*arg);
			}
			arg++;
		}
		qarg.push_back(L'"');
		return qarg;
	}
}

std::wstring GetOemChar(wchar_t def_char, wchar_t alt_char, DWORD oem_vk, DWORD oem_sc)
{
	//Actual purpose of GetOemChar is to force default OEM vk meaning to maintain some consistency between various installed layouts
	//Also we have to ensure that this OEM vk stays on the same physical place on hw kb
	//For example we can have user with QWERTY/ЙЦУКЕН layout for which VK_OEM_3 can be either "~`" or "Ё" depending on layout
	//Using this function we can force VK_OEM_3 to be always displayed as [ ~ ] for such user
	//And if he opts to uninstall QWERTY layout this function will display VK_OEM_3 as [ Ё ]
	
	//Note on virtual keys VS scan codes
	//Scan code represents unique ID of key on hw keyboard and it is independent from layout
	//E.g. QWERTY layout 'Q' key has scan code 0x10, while on AZERTY this code results in 'A' key, which is indeed occupies QWERTY's 'Q' key space
	//Virtual key represents meaning of the key pressed but position of this key on hw kb depends on selected layout
	//E.g. pressing 'Q' key on QWERTY layout we get 0x51 vk and on AZERTY layout 'Q' key will have the same 0x51 vk, though their scan codes (and physical position) will be different
	//And now we have OEM virtual keys
	//Not only their physical postion on hw kb will depend on currently selected layout, but their actual meaning will also change
	//E.g. VK_OEM_6 on QWERTY US layout represents "}]" key and is located under backspace
	//But on German QWERTZ layout VK_OEM_6 represents "`'" key and is relocated to the left of backspace (switching places with QWERTY's VK_OEM_PLUS)
	//MapVirtualKey(MAPVK_VK_TO_CHAR) actually takes in account keyboard layout, so passing it VK_OEM_6 while German QWERTZ layout selected will result in proper [ ' ] key
	
	//Code below checks if default char for OEM key is actually present on this hardware key for any of the installed layouts
	if (int hkl_len=GetKeyboardLayoutList(0, NULL)) {
		HKL hkl_lst[hkl_len];
		if (GetKeyboardLayoutList(hkl_len, hkl_lst)) {
			DWORD layout_vk;
			while (--hkl_len>=0)
				if ((layout_vk=LOBYTE(VkKeyScanEx(def_char, hkl_lst[hkl_len])))==MapVirtualKeyEx(oem_sc, MAPVK_VSC_TO_VK, hkl_lst[hkl_len]))
					//Default char is found on OEM vk for one of the layouts - return it
					return {def_char};
		}
	}
	
	//If not found - try with alt char or return actual OEM char
	if (alt_char!=L'\0')
		return GetOemChar(alt_char, L'\0', oem_vk, oem_sc);
	else if (wchar_t mapped_char=(wchar_t)MapVirtualKey(oem_vk, MAPVK_VK_TO_CHAR))
		return {mapped_char};
	else
		return DwordToHexString(oem_vk, 2);
}

std::wstring GetHotkeyWarning(ModKeyType mod_key, BINDED_KEY key, const wchar_t* prefix, const wchar_t* postfix, const wchar_t* defval)
{
	//Function is designed to make a warning for user that this binding may not work if some of modifier keys are pressed
	//This is because historically for some keys modifier keys affect not only vk but also sc
	std::wstring wrn_str;
	
	if ((mod_key==ModKeyType::SHIFT_ALT||mod_key==ModKeyType::CTRL_ALT)&&key.sc==0x37&&key.ext) {
		//PrtScn (E037) with Alt pressed results in SysRq sc (84)
		wrn_str+=L"Alt";
	} else if ((mod_key==ModKeyType::CTRL_SHIFT||mod_key==ModKeyType::CTRL_ALT)&&key.sc==0x45) {
		//Pause (45) with Ctrl pressed results in Break sc (E046)
		wrn_str+=L"Ctrl";
	}
	
	if (wrn_str.length()) {		
		if (prefix)
			wrn_str.insert(0, prefix);
		if (postfix)
			wrn_str+=postfix;
		return wrn_str;
	} else
		return defval?defval:wrn_str;
}

std::wstring GetHotkeyString(BINDED_KEY key, const wchar_t* prefix, const wchar_t* postfix)
{
	std::wstring hk_str;
	
	if (prefix)
		hk_str=prefix;
	
	//Mouse buttons and mod keys (Alt, Shift, Ctrl) are excluded from the list because binding keyboard hook ignores them
	//Other excluded keys also can be set through register but in this case they will be displayed as hex characters signaling user that something is not right
	//Function is trying to name keys more positionwise - e.g. independent of Shift, NumLock state and selected layout
	switch (key.vk) {
		case VK_SPACE:
			hk_str+=L"Space";
			break;
		case VK_RETURN:
			hk_str+=L"Enter";
			break;
		case VK_CANCEL:
			//Break is a special case
			//It is always a two-key combination: Control followed by Break
			//Break's scan code is shared between Break and ScrLock virtual keys (because historically it first came as ScrLock's alternative on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Break";
			else
				hk_str+=L"ScrLock";
			break;
		case VK_BACK:
			hk_str+=L"BS";
			break;
		case VK_TAB:
			hk_str+=L"Tab";
			break;
		case VK_CLEAR:
			//It's a less known alternative function of Num[5]
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Clear";
			else
				hk_str+=L"Num5";
			break;
		case VK_PAUSE:
			//Pause's scan code is shared between Pause and NumLock virtual keys (because historically it first came as NumLock's alternative on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"NumLock";
			else
				hk_str+=L"Pause";
			break;
		case VK_CAPITAL:
			hk_str+=L"CapsLock";
			break;
		case VK_KANA:
			hk_str+=L"Kana/Hangul";
			break;
		case VK_JUNJA:
			hk_str+=L"Junja";
			break;
		case VK_KANJI:
			hk_str+=L"Kanji/Hanja";
			break;
		case VK_ESCAPE:
			hk_str+=L"Esc";
			break;
		case VK_CONVERT:
			hk_str+=L"Convert";
			break;
		case VK_NONCONVERT:
			hk_str+=L"NonConvert";
			break;
		case VK_ACCEPT:
			hk_str+=L"Accept";
			break;
		case VK_MODECHANGE:
			hk_str+=L"ModeChange";
			break;
		case VK_PRIOR:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"PgUp";
			else
				hk_str+=L"Num9";
			break;
		case VK_NEXT:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"PgDn";
			else
				hk_str+=L"Num3";
			break;
		case VK_END:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"End";
			else
				hk_str+=L"Num1";
			break;
		case VK_HOME:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Home";
			else
				hk_str+=L"Num7";
			break;
		case VK_LEFT:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Left";
			else
				hk_str+=L"Num4";
			break;
		case VK_RIGHT:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Right";
			else
				hk_str+=L"Num6";
			break;
		case VK_UP:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Up";
			else
				hk_str+=L"Num8";
			break;
		case VK_DOWN:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Down";
			else
				hk_str+=L"Num2";
			break;
		case VK_SELECT:
			hk_str+=L"Select";
			break;
		case VK_PRINT:
			//Used on old Nokia Data 121-key keyboards
			hk_str+=L"Print";
			break;
		case VK_EXECUTE:
			//Marked as "non used" in docs
			hk_str+=L"Execute";
			break;
		case VK_SNAPSHOT:
			//Starting with Windows 3.0 VK_SNAPSHOT is shared between SysRq and PrtScn scancodes
			//Also PrtScn's scan code is shared between PrtScn and Num[*] virtual keys (because historically it first came as Num[*]'s alternative on IBM PC/XT/AT keyboard)
			if (key.sc==0x54)
				hk_str+=L"SysRq";
			else
				hk_str+=L"PrtScn";
			break;
		case VK_INSERT:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Ins";
			else
				hk_str+=L"Num0";
			break;
		case VK_DELETE:
			//Num keys' scancodes are shared with cursor and system keys (because historically there were no separate cursor and system keys and they all came as alternatives to num keys on IBM PC/XT/AT keyboard)
			if (key.ext)
				hk_str+=L"Del";
			else
				hk_str+=L"NumDecSep";
			break;
		case VK_HELP:
			hk_str+=L"Help";
			break;
		case VK_LWIN:
			hk_str+=L"LWin";
			break;
		case VK_RWIN:
			hk_str+=L"RWin";
			break;
		case VK_APPS:
			hk_str+=L"Menu";
			break;
		case VK_SLEEP:
			hk_str+=L"Sleep";
			break;
		case VK_MULTIPLY:
			hk_str+=L"Num*";
			break;
		case VK_ADD:
			hk_str+=L"Num+";
			break;
		case 0xC2:	
			//VK_ABNT_C2, replaces VK_SEPARATOR on Brazilian kb
		case VK_SEPARATOR:
			//Thousands separator, sometimes present on numpad and localized (so can be actually comma or period)
			hk_str+=L"NumTnd";
			break;
		case VK_SUBTRACT:
			hk_str+=L"Num-";
			break;
		case VK_DECIMAL:
			//Decimal separator, localized (can be comma or period)
			hk_str+=L"NumDec";
			break;
		case VK_DIVIDE:
			//Num[/]'s scan code is shared between Num[/] and [?/] virtual keys (because historically at first there was no Num[/] on IBM PC/XT/AT keyboard)
			hk_str+=L"Num/";
			break;
		case VK_NUMLOCK:
			hk_str+=L"NumLock";
			break;
		case VK_SCROLL:
			hk_str+=L"ScrLock";
			break;
		case VK_BROWSER_BACK:
			hk_str+=L"BrowserBack";
			break;
		case VK_BROWSER_FORWARD:
			hk_str+=L"BrowserForward";
			break;
		case VK_BROWSER_REFRESH:
			hk_str+=L"BrowserRefresh";
			break;
		case VK_BROWSER_STOP:
			hk_str+=L"BrowserStop";
			break;
		case VK_BROWSER_SEARCH:
			hk_str+=L"BrowserSearch";
			break;
		case VK_BROWSER_FAVORITES:
			hk_str+=L"BrowserFavorites";
			break;
		case VK_BROWSER_HOME:
			hk_str+=L"BrowserHome";
			break;
		case VK_VOLUME_MUTE:
			hk_str+=L"VolumeMute";
			break;				
		case VK_VOLUME_DOWN:
			hk_str+=L"VolumeDown";
			break;
		case VK_VOLUME_UP:
			hk_str+=L"VolumeUp";
			break;
		case VK_MEDIA_NEXT_TRACK:
			hk_str+=L"MediaTrackNext";
			break;
		case VK_MEDIA_PREV_TRACK:
			hk_str+=L"MediaTrackPrevious";
			break;
		case VK_MEDIA_STOP:
			hk_str+=L"MediaStop";
			break;
		case VK_MEDIA_PLAY_PAUSE:
			hk_str+=L"MediaPlayPause";
			break;
		case VK_LAUNCH_MAIL:
			hk_str+=L"LaunchMail";
			break;
		case VK_LAUNCH_MEDIA_SELECT:
			hk_str+=L"MediaSelect";
			break;
		case VK_LAUNCH_APP1:
			hk_str+=L"LaunchApp1";
			break;
		case VK_LAUNCH_APP2:
			hk_str+=L"LaunchApp2";
			break;
		case VK_OEM_1:
			hk_str+=GetOemChar(L':', L';', key.vk, key.sc);
			break;
		case VK_OEM_PLUS:
			hk_str+={L'+'};
			break;
		case VK_OEM_COMMA:
			hk_str+={L','};
			break;
		case VK_OEM_MINUS:
			hk_str+={L'-'};
			break;
		case VK_OEM_PERIOD:
			hk_str+={L'.'};
			break;
		case VK_OEM_2:
			hk_str+=GetOemChar(L'?', L'/', key.vk, key.sc);
			break;
		case VK_OEM_3:
			//[ ~ ` ] on US kb and [ @ ' ] on UK kb
			hk_str+=GetOemChar(L'~', L'@', key.vk, key.sc);
			break;
		case VK_OEM_4:
			hk_str+=GetOemChar(L'{', L'[', key.vk, key.sc);
			break;
		case VK_OEM_5:
			hk_str+=GetOemChar(L'|', L'\\', key.vk, key.sc);
			break;
		case VK_OEM_6:
			hk_str+=GetOemChar(L'}', L']', key.vk, key.sc);
			break;
		case VK_OEM_7:
			//[ " ' ] on US kb and [ ~ # ] on UK kb
			hk_str+=GetOemChar(L'"', L'~', key.vk, key.sc);
			break;
		case VK_OEM_8:
			//MS defines this as "used for miscellaneous characters" but often it is [ § ! ] on AZERTY kb and [ ¬ ` ] on UK QWERTY kb
			hk_str+=GetOemChar(L'§', L'¬', key.vk, key.sc);
			break;
		case VK_OEM_102:
			//Used on 102 keyboard - often it is [ | \ ] on newer QWERTY kb or [ > < ] on QWERTZ kb
			//Also present on non-102 AZERTY kb as [ > < ]
			hk_str+=GetOemChar(L'|', L'>', key.vk, key.sc);
			break;
		case 0xC1:	//VK_ABNT_C1
			//Additional OEM key on Brazilian kb
			hk_str+=GetOemChar(L'?', L'/', key.vk, key.sc);
			break;
		case VK_OEM_AX:
			hk_str+=L"AX";
			break;	
		case VK_PROCESSKEY:
			hk_str+=L"Process";
			break;
		case VK_ATTN:
			//Present on 122-key keyboard
			hk_str+=L"Attn";
			break;
		case VK_CRSEL:
			//Present on 122-key keyboard
			hk_str+=L"CrSel";
			break;
		case VK_EXSEL:
			//Present on 122-key keyboard
			hk_str+=L"ExSel";
			break;
		case VK_EREOF:
			//Present on 122-key keyboard
			hk_str+=L"ErEOF";
			break;
		case VK_PLAY:
			//Present on 122-key keyboard
			hk_str+=L"Play";
			break;
		case VK_ZOOM:
			//Present on 122-key keyboard
			hk_str+=L"Zoom";
			break;
		case VK_PA1:
			//Present on 122-key keyboard
			hk_str+=L"PA1";
			break;
		case VK_OEM_CLEAR:
			//Present on 122-key keyboard
			hk_str+=L"Clear";
			break;
		case VK_NONAME:
			//Present on 122-key keyboard
			hk_str+=L"Noname";
			break;
		case VK_ICO_HELP:
			//Present on Olivetti keyboard
			hk_str+=L"Help";
			break;
		case VK_ICO_00:
			//Present on Olivetti keyboard
			hk_str+=L"00";
			break;
		case VK_ICO_CLEAR:
			//Present on Olivetti keyboard
			hk_str+=L"Clear";
			break;
		default:
			if (key.vk>=0x30&&key.vk<=0x39) {
				//0-9 keys
				//Using standard UTF-8 CP (0-9 are 0x30-0x39)
				hk_str+={(wchar_t)key.vk /* vk-0x30+0x30 */};
			} else if (key.vk>=0x41&&key.vk<=0x5A) {
				//A-Z keys
				//Using standard UTF-8 CP (A-Z are 0x41-0x5A)
				hk_str+={(wchar_t)key.vk /* vk-0x41+0x41 */};
			} else if (key.vk>=0x60&&key.vk<=0x69) {
				//Numpad 0-9 keys
				//Using standard UTF-8 CP (0-9 are 0x30-0x39)
				hk_str+={L'N', L'u', L'm', (wchar_t)(key.vk-0x30) /* vk-0x60+0x30 */};
			} else if (key.vk>=0x70&&key.vk<=0x87) {
				//Function F1-F24 keys
				hk_str+=L"F"+to_wstring_wrapper(key.vk-0x6F);
			} else if (key.sc==0x5E&&key.ext) {
				//Power management Power key
				hk_str+=L"Power";
			} else if (key.sc==0x5F&&key.ext) {
				//Power management Sleep key
				hk_str+=L"Sleep";
			} else if (key.sc==0x63&&key.ext) {
				//Power management Wake key
				hk_str+=L"Wake";
			} else {
				//Unknown, reserved and rest of OEM specific keys goes here
				hk_str+=DwordToHexString(key.vk, 2);
			}
	}
	
	if (postfix)
		hk_str+=postfix;
	
	return hk_str;
}

std::wstring GetHotkeyString(ModKeyType mod_key, BINDED_KEY key, const wchar_t* prefix, const wchar_t* postfix)
{
	std::wstring hk_str;
	
	if (prefix)
		hk_str=prefix;

	switch (mod_key) {
		case ModKeyType::CTRL_ALT:
			hk_str+=L"Ctrl + Alt + ";
			break;
		case ModKeyType::SHIFT_ALT:
			hk_str+=L"Shift + Alt + ";
			break;
		case ModKeyType::CTRL_SHIFT:
			hk_str+=L"Ctrl + Shift + ";
			break;
	}
	
	return hk_str+GetHotkeyString(key, NULL, postfix);
}
