#include "SuiteCommon.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::wstring GetHexVk(DWORD vk)
{
	std::wstringstream hex_vk;
	hex_vk<<L"0x"<<std::hex<<std::noshowbase<<std::uppercase<<std::setfill(L'0')<<std::setw(2)<<vk;
	return hex_vk.str();
}

std::wstring GetOemChar(wchar_t def_char, wchar_t alt_char, DWORD oem_vk)
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
	
	//Code below checks if default char for OEM key is actually present on this key for any of the installed layouts
	//0xFF is invalid vk - it will force algorithm to think that default char wasn't found in case of GetKeyboardLayoutList() fails
	DWORD layout_vk=0xFF;
	if (int hkl_len=GetKeyboardLayoutList(0, NULL)) {
		HKL hkl_lst[hkl_len];
		GetKeyboardLayoutList(hkl_len, hkl_lst);
		while ((layout_vk=LOBYTE(VkKeyScanEx(def_char, hkl_lst[--hkl_len])))!=oem_vk&&hkl_len);
	}
	
	if (layout_vk==oem_vk)
		//If default char is found on OEM vk for one of the layouts - return it
		return {L'[', L' ', def_char, L' ', L']'};
	else {
		//If not found - try with alt char or return actual OEM char
		if (alt_char!=L'\0')
			return GetOemChar(alt_char, L'\0', oem_vk);
		else if (wchar_t mapped_char=(wchar_t)MapVirtualKey(oem_vk, MAPVK_VK_TO_CHAR))
			return {L'[', L' ', mapped_char, L' ', L']'};
		else
			return GetHexVk(oem_vk);
	}
}

std::wstring GetHotkeyString(SuiteSettings::ModKeyType mod_key, DWORD vk, GetHotkeyStringType type, const wchar_t* prefix, const wchar_t* postfix)
{
	std::wstring hk_str=L"";
	
	if (prefix)
		hk_str+=prefix;
	
	if (type==GetHotkeyStringType::FULL||type==GetHotkeyStringType::MOD_KEY) {
		switch (mod_key) {
			case SuiteSettings::CTRL_ALT:
				hk_str=L"Ctrl+Alt+";
				break;
			case SuiteSettings::SHIFT_ALT:
				hk_str=L"Shift+Alt+";
				break;
			case SuiteSettings::CTRL_SHIFT:
				hk_str=L"Ctrl+Shift+";
				break;
		}
	}
	
	//Mouse buttons, space, enter and mod keys (Alt, Shift, Ctrl) are excluded from the list because binding keyboard hook ignores them
	//Space and enter are still here because they can be set through register
	//Other excluded keys also can be set through register but in this case they will be displayed as hex characters signaling user that something is not right
	if (type==GetHotkeyStringType::FULL||type==GetHotkeyStringType::VK) {
		switch (vk) {
			case VK_SPACE:
				hk_str+=L"Space";
				break;
			case VK_RETURN:
				hk_str+=L"Enter";
				break;
			case VK_CANCEL:
				hk_str+=L"Break";
				break;
			case VK_BACK:
				hk_str+=L"BS";
				break;
			case VK_TAB:
				hk_str+=L"Tab";
				break;
			case VK_CLEAR:
				hk_str+=L"Clear";
				break;
			case VK_PAUSE:
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
				hk_str+=L"PgUp";
				break;
			case VK_NEXT:
				hk_str+=L"PgDn";
				break;
			case VK_END:
				hk_str+=L"End";
				break;
			case VK_HOME:
				hk_str+=L"Home";
				break;
			case VK_LEFT:
				hk_str+=L"Left";
				break;
			case VK_RIGHT:
				hk_str+=L"Right";
				break;
			case VK_UP:
				hk_str+=L"Up";
				break;
			case VK_DOWN:
				hk_str+=L"Down";
				break;
			case VK_SELECT:
				hk_str+=L"Select";
				break;
			case VK_PRINT:
				hk_str+=L"Print";
				break;
			case VK_EXECUTE:
				hk_str+=L"Execute";
				break;
			case VK_SNAPSHOT:
				hk_str+=L"PrtScn";
				break;
			case VK_INSERT:
				hk_str+=L"Ins";
				break;
			case VK_DELETE:
				hk_str+=L"Del";
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
				hk_str+=L"Num[ * ]";
				break;
			case VK_ADD:
				hk_str+=L"Num[ + ]";
				break;
			case VK_SEPARATOR:
				//Thousands separator, sometimes present on numpad and localized (so can be actually comma or period)
				hk_str+=L"Num";
				hk_str+={L'[', L' ', (wchar_t)MapVirtualKey(VK_SEPARATOR, MAPVK_VK_TO_CHAR), L' ', L']'};
				break;
			case VK_SUBTRACT:
				hk_str+=L"Num[ - ]";
				break;
			case VK_DECIMAL:
				//Decimal separator, localized (can be comma or period)
				hk_str+=L"Num";
				hk_str+={L'[', L' ', (wchar_t)MapVirtualKey(VK_DECIMAL, MAPVK_VK_TO_CHAR), L' ', L']'};
				break;
			case VK_DIVIDE:
				hk_str+=L"Num[ / ]";
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
				hk_str+=GetOemChar(L':', L';', VK_OEM_1);
				hk_str+=L"VK_OEM_1";
				break;
			case VK_OEM_PLUS:
				hk_str+=L"[ + ]";
				hk_str+=L"VK_OEM_PLUS";
				break;
			case VK_OEM_COMMA:
				hk_str+=L"[ , ]";
				hk_str+=L"VK_OEM_COMMA";
				break;
			case VK_OEM_MINUS:
				hk_str+=L"[ - ]";
				hk_str+=L"VK_OEM_MINUS";
				break;
			case VK_OEM_PERIOD:
				hk_str+=L"[ . ]";
				hk_str+=L"VK_OEM_PERIOD";
				break;
			case VK_OEM_2:
				hk_str+=GetOemChar(L'?', L'/', VK_OEM_2);
				hk_str+=L"VK_OEM_2";
				break;
			case VK_OEM_3:
				hk_str+=GetOemChar(L'~', L'`', VK_OEM_3);
				hk_str+=L"VK_OEM_3";
				break;
			case VK_OEM_4:
				hk_str+=GetOemChar(L'{', L'[', VK_OEM_4);
				hk_str+=L"VK_OEM_4";
				break;
			case VK_OEM_5:
				hk_str+=GetOemChar(L'|', L'\\', VK_OEM_5);
				hk_str+=L"VK_OEM_5";
				break;
			case VK_OEM_6:
				hk_str+=GetOemChar(L'}', L']', VK_OEM_6);
				hk_str+=L"VK_OEM_6";
				break;
			case VK_OEM_7:
				hk_str+=GetOemChar(L'"', L'\'', VK_OEM_7);
				hk_str+=L"VK_OEM_7";
				break;
			case VK_OEM_8:
				//MS defines this as "used for miscellaneous characters" but often it is [ § ! ] on AZERTY kb
				hk_str+=GetOemChar(L'§', L'!', VK_OEM_8);
				hk_str+=L"VK_OEM_8";
				break;
			case VK_OEM_102:
				//Used on 102 keyboard - often it is [ | \ ] on newer QWERTY kb or [ > < ] on QWERTZ kb
				//Also present on non-102 AZERTY kb as [ > < ]
				hk_str+=GetOemChar(L'|', L'>', VK_OEM_102);
				hk_str+=L"VK_OEM_102";
				break;
			case VK_OEM_AX:
				hk_str+=L"AX";
				break;	
			case VK_PROCESSKEY:
				hk_str+=L"Process";
				break;
			case VK_ATTN:
				hk_str+=L"Attn";
				break;
			case VK_CRSEL:
				hk_str+=L"CrSel";
				break;
			case VK_EXSEL:
				hk_str+=L"ExSel";
				break;
			case VK_EREOF:
				hk_str+=L"ErEOF";
				break;
			case VK_PLAY:
				hk_str+=L"Play";
				break;
			case VK_ZOOM:
				hk_str+=L"Zoom";
				break;
			case VK_PA1:
				hk_str+=L"PA1";
				break;
			case VK_OEM_CLEAR:
				hk_str+=L"Clear";
				break;
			default:
				if (vk>=0x30&&vk<=0x39) {
					//0-9 keys
					//Not using MapVirtualKey(MAPVK_VK_TO_CHAR) because some layouts assume that default map for numeric keys is their shifted-state
					hk_str+=L"[ "+std::to_wstring(vk-0x30)+L" ]";
				} else if (vk>=0x41&&vk<=0x5A) {
					//A-Z keys
					hk_str+={L'[', L' ', (wchar_t)MapVirtualKey(vk, MAPVK_VK_TO_CHAR), L' ', L']'};
				} else if (vk>=0x60&&vk<=0x69) {
					//Numpad 0-9 keys
					hk_str+=L"Num[ "+std::to_wstring(vk-0x60)+L" ]";
				} else if (vk>=0x70&&vk<=0x87) {
					//Function keys
					hk_str+=L"F"+std::to_wstring(vk-0x6F);
				} else {
					//Unknown, reserved and rest of OEM specific keys goes here
					hk_str+=GetHexVk(vk);
				}
		}
	}
	
	if (postfix)
		hk_str+=postfix;
	
	return hk_str;
}
