#include "TaskbarNotificationAreaIcon.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include "Res.h"
#include <memory>
#include <tuple>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <windows.h>

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam);

enum class GetHotkeyStringType:char {FULL, MOD_KEY, VK};
std::wstring GetHotkeyString(SuiteSettings::ModKeyType mod_key, DWORD vk, GetHotkeyStringType type);

#ifdef OBSOLETE_WWINMAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
	LPWSTR lpCmdLine=GetCommandLineW();
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#endif
	TskbrNtfAreaIcon* SnkIcon=NULL;
	HotkeyEngine* SnkHotkey=NULL;
	SuiteSettings Settings;
	KeyTriplet OnKeyTriplet;
	
	//It's ok to pass reference to NULL HotkeyEngine to OnWmCommand - see IconMenuProc comments
	//std::bind differs from lamda captures in that you can't pass references by normal means - object will be copied anyway
	//To pass a reference you should wrap referenced object in std::ref
	SnkIcon=TskbrNtfAreaIcon::MakeInstance(hInstance, WM_HSTNAICO, L"SNK_HS: RUNNING", IDI_HSTNAICO, L"SnK_HotkeySuite_IconClass", IDR_ICONMENU, IDM_STOP_START, 
		std::bind(IconMenuProc, std::ref(SnkHotkey), &Settings, &OnKeyTriplet, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	if (!SnkIcon->IsValid()) {
		MessageBox(NULL, L"Failed to create icon!", L"SNK_HS", MB_ICONERROR|MB_OK);
		return 1;
	}
	
	//At this point taskbar icon is already visible but unusable - it doesn't respond to any clicks and can't show popup menu
	//So it's ok to customize menu here and initialize everything else
	SnkHotkey=HotkeyEngine::MakeInstance(hInstance);
	if (Settings.GetLongPress()) {
		SnkIcon->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED); 
		OnKeyTriplet.SetLongPress(true);
	}
	switch (Settings.GetModKey()) {
		case SuiteSettings::CTRL_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			OnKeyTriplet.SetCtrlAlt();
			break;
		case SuiteSettings::SHIFT_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			OnKeyTriplet.SetShiftAlt();
			break;
		case SuiteSettings::CTRL_SHIFT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			OnKeyTriplet.SetCtrlShift();
			break;
	}
	//OnKeyTriplet can be passed as reference (wrapped in std::ref) or as pointer
	if (!SnkHotkey->StartNew(std::bind(&KeyTriplet::OnKeyPress, std::ref(OnKeyTriplet), std::placeholders::_1, std::placeholders::_2))) {
		MessageBox(NULL, L"Failed to set keyboard hook!", L"SNK_HS", MB_ICONERROR|MB_OK);
		return 2;
	}
	//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
	SnkIcon->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(SnkIcon->GetIconMenu(), 2), GetHotkeyString(Settings.GetModKey(), Settings.GetBindedVK(), GetHotkeyStringType::FULL).c_str()); 
	//SnkIcon->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
	
	//Main thread's message loop
	//GetMessage returns -1 if error (probably happens only with invalid input parameters) and 0 if WM_QUIT 
	//So continue on any non-zero result skipping errors
	MSG msg;
	BOOL res;
	while ((res=GetMessage(&msg, NULL, 0, 0))) {
		if (res>0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//Manually uninitializng some components to make sure right unintializtion order
	SnkHotkey->Stop();
	
	return msg.wParam;
}

INT_PTR CALLBACK BindingDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hwndDlg, DWLP_USER, 0);
			SetDlgItemText(hwndDlg, IDC_VK_PREFIX, GetHotkeyString(std::get<1>(*(std::tuple<HotkeyEngine*, SuiteSettings::ModKeyType, DWORD>*)lParam), 0, GetHotkeyStringType::MOD_KEY).c_str());
			SetDlgItemText(hwndDlg, IDC_VK_VIEWER, GetHotkeyString(SuiteSettings::CTRL_ALT, std::get<2>(*(std::tuple<HotkeyEngine*, SuiteSettings::ModKeyType, DWORD>*)lParam), GetHotkeyStringType::VK).c_str());
			//If we fail with starting binding keyboard hook - exit immediately with 0 result
			//Callee will determine that it was error because HotkeyEngine::Stop() will return false
			//It's callee responsibility to stop this hook
			if (!std::get<0>(*(std::tuple<HotkeyEngine*, SuiteSettings::ModKeyType, DWORD>*)lParam)->StartNew(std::bind(BindKey, hwndDlg, WM_BINDVK, std::placeholders::_1, std::placeholders::_2)))
				EndDialog(hwndDlg, 0);
			return TRUE;
		case WM_BINDVK:
			SetDlgItemText(hwndDlg, IDC_VK_VIEWER, GetHotkeyString(SuiteSettings::CTRL_ALT, wParam, GetHotkeyStringType::VK).c_str());
			SetWindowLongPtr(hwndDlg, DWLP_USER, wParam);
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			EndDialog(hwndDlg, 0);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_CONFIRM_VK:
						EndDialog(hwndDlg, GetWindowLongPtr(hwndDlg, DWLP_USER));
						return TRUE;
					case IDC_CANCEL_VK:
						EndDialog(hwndDlg, 0);
						return TRUE;
				}
			}
			return FALSE;
		default:
			return FALSE;
	}
}

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam)
{
	//Not checking if hk_engine is NULL in menu handlers - handlers won't be called until message loop is fired which happens after creation of hk_engine
	bool hk_was_running=false;
	std::tuple<HotkeyEngine*, SuiteSettings::ModKeyType, DWORD> bindingdlg_tuple;
	DWORD bind_vk=0;
	switch (LOWORD(wParam)) {
		case IDM_EXIT:
			//We can just use PostQuitMessage() and wait for TskbrNtfAreaIcon destructor to destroy icon at the end of the program
			//But in this case icon will be briefly present after the end of message loop till the end of WinMain, though being unresponsive
			//It will be better to destroy the icon right away and then exit message loop
			//And after that do all other uninitialization without icon being visible for unknown purpose
			sender->CloseAndQuit();
			return true;
		case IDM_STOP_START:
			if (hk_engine->IsRunning()) {
				hk_engine->Stop();
				sender->ChangeIconTooltip(L"SNK_HS: STOPPED");
				sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Start"); 
			} else {
				if (!hk_engine->Start()) break;
				sender->ChangeIconTooltip(L"SNK_HS: RUNNING");
				sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Stop"); 
			}
			return true;
		case IDM_EDIT_SHK:
			MessageBox(NULL, L"ID_EDIT_SHK", L"SNK_HS", MB_OK);
			return true;
		case IDM_EDIT_LHK:
			MessageBox(NULL, L"ID_EDIT_LHK", L"SNK_HS", MB_OK);
			return true;
		case IDM_SET_EN_LHK:
			hk_was_running=hk_engine->Stop();
			if (settings->GetLongPress()) {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_UNCHECKED);
				settings->SetLongPress(false);
				hk_triplet->SetLongPress(false);
			} else {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED);
				settings->SetLongPress(true);
				hk_triplet->SetLongPress(true);
			}
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			settings->SetModKey(SuiteSettings::CTRL_ALT);
			hk_triplet->SetCtrlAlt();
			//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
			sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(SuiteSettings::CTRL_ALT, settings->GetBindedVK(), GetHotkeyStringType::FULL).c_str()); 
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_SHIFT_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			settings->SetModKey(SuiteSettings::SHIFT_ALT);
			hk_triplet->SetShiftAlt();
			//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
			sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(SuiteSettings::SHIFT_ALT, settings->GetBindedVK(), GetHotkeyStringType::FULL).c_str()); 
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_SHIFT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			settings->SetModKey(SuiteSettings::CTRL_SHIFT);
			hk_triplet->SetCtrlShift();
			//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
			sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(SuiteSettings::CTRL_SHIFT, settings->GetBindedVK(), GetHotkeyStringType::FULL).c_str()); 
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CUSTOM:
			//First disable this menu item so binding dialog won't be called second time
			//Other menu item can be clicked - they'll restart binding keyboard hook like it was hotkey hook
			sender->EnableIconMenuItem(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_GRAYED);
			hk_was_running=hk_engine->Stop();
			//Several words on InitCommonControls() and InitCommonControlsEx()
			//"Common" name is somewhat misleading
			//Even though controls like "static", "edit" and "button" are common (like in common sence) they are actually "standard" controls
			//So there is no need to initialize them with InitCommonControls() or InitCommonControlsEx() functions
			//Though ICC_STANDARD_CLASSES can be passed to InitCommonControlsEx, it actually does nothing - "standard" controls are really initialized by the system
			//Also these functions has nothing to do with "Win XP"/"ComCtl32 v6+" style - just supply proper manifest to make "standard" controls use it
			//IDD_BINDINGDLG uses only "standard" controls
			//Return result for IDD_BINDINGDLG is VK to bind or 0 if binding was canceled
			bindingdlg_tuple=std::make_tuple(hk_engine, settings->GetModKey(), settings->GetBindedVK());
			bind_vk=DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_BINDINGDLG), sender->GetIconWindow(), BindingDialogProc, (LPARAM)&bindingdlg_tuple);
			//Inside dialog's DLGPROC we are starting new binding keyboard hook
			//If this fail - HotkeyEngine::Stop() will return false because hook wasn't started
			if (!hk_engine->Stop()) break;
			if (bind_vk) { 
				hk_triplet->SetBindedVK(bind_vk);
				settings->SetBindedVK(bind_vk);
				//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
				sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(settings->GetModKey(), bind_vk, GetHotkeyStringType::FULL).c_str()); 
			}
			if (hk_was_running&&!hk_engine->StartNew(std::bind(&KeyTriplet::OnKeyPress, hk_triplet, std::placeholders::_1, std::placeholders::_2))) break;
			sender->EnableIconMenuItem(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_ENABLED);
			return true;
		default:
			return false;
	}
	
	//We get there after break which happens instead of return in all cases where hk_engine should have restarted but failed
	MessageBox(NULL, L"Failed to restart keyboard hook!", L"SNK_HS", MB_ICONERROR|MB_OK);
	sender->CloseAndQuit(3);
	return true;
}

wchar_t GetOemChar(wchar_t def_char, DWORD oem_vk)
{
	//Code below checks if default char for OEM key is actually present on this key
	//If yes - default char is returned, if no - actual char for this key is returned
	//First it checks default layout (which is appended to the end of the list) and then other layouts starting from the last
	//Layout list is unorganized and doesn't follow system layout priority so default layout doesn't have predefined index
	//That's why we check default layout separately but at the same time as part of general layout list check loop
	//Downside - one  excessive check (because default layout is also present in layout list) but in return we have more uniform code
	DWORD actual_vk;
	int hkl_len=GetKeyboardLayoutList(0, NULL);
	HKL hkl_lst[hkl_len+1];
	if (hkl_len) GetKeyboardLayoutList(hkl_len, hkl_lst);
	hkl_lst[hkl_len]=GetKeyboardLayout(0);
	while ((actual_vk=LOBYTE(VkKeyScanEx(def_char, hkl_lst[hkl_len])))==255&&hkl_len--);
	
	if (actual_vk==oem_vk)
		return def_char;
	else
		return (wchar_t)MapVirtualKey(oem_vk, MAPVK_VK_TO_CHAR);
}

std::wstring GetHotkeyString(SuiteSettings::ModKeyType mod_key, DWORD vk, GetHotkeyStringType type)
{
	std::wstring hk_str=L"";
	
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
				hk_str+=L"CapsLk";
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
				hk_str+=L"Num*";
				break;
			case VK_ADD:
				hk_str+=L"Num+";
				break;
			case VK_SEPARATOR:
				hk_str+=L"Num,";
				break;
			case VK_SUBTRACT:
				hk_str+=L"Num-";
				break;
			case VK_DECIMAL:
				hk_str+=L"Num.";
				break;
			case VK_DIVIDE:
				hk_str+=L"Num/";
				break;
			case VK_NUMLOCK:
				hk_str+=L"NumLk";
				break;
			case VK_SCROLL:
				hk_str+=L"ScrLk";
				break;
			case VK_BROWSER_BACK:
				hk_str+=L"BrowserRew";
				break;
			case VK_BROWSER_FORWARD:
				hk_str+=L"BrowserFwd";
				break;
			case VK_BROWSER_REFRESH:
				hk_str+=L"BrowserRld";
				break;
			case VK_BROWSER_STOP:
				hk_str+=L"BrowserStop";
				break;
			case VK_BROWSER_SEARCH:
				hk_str+=L"BrowserFnd";
				break;
			case VK_BROWSER_FAVORITES:
				hk_str+=L"BrowserFav";
				break;
			case VK_BROWSER_HOME:
				hk_str+=L"BrowserHome";
				break;
			case VK_VOLUME_DOWN:
				hk_str+=L"MediaVolDn";
				break;
			case VK_VOLUME_UP:
				hk_str+=L"MediaVolUp";
				break;
			case VK_MEDIA_NEXT_TRACK:
				hk_str+=L"MediaNext";
				break;
			case VK_MEDIA_PREV_TRACK:
				hk_str+=L"MediaPrev";
				break;
			case VK_MEDIA_STOP:
				hk_str+=L"MediaStop";
				break;
			case VK_MEDIA_PLAY_PAUSE:
				hk_str+=L"MediaPlay";
				break;
			case VK_LAUNCH_MAIL:
				hk_str+=L"MediaMail";
				break;
			case VK_LAUNCH_MEDIA_SELECT:
				hk_str+=L"MediaSelect";
				break;
			case VK_LAUNCH_APP1:
				hk_str+=L"MediaApp1";
				break;
			case VK_LAUNCH_APP2:
				hk_str+=L"MediaApp2";
				break;
			case VK_OEM_1:
				hk_str+=GetOemChar(L':', VK_OEM_1);
				break;
			case VK_OEM_PLUS:
				hk_str+=L"+";
				break;
			case VK_OEM_COMMA:
				hk_str+=GetOemChar(L'<', VK_OEM_COMMA);
				break;
			case VK_OEM_MINUS:
				hk_str+=L"-";
				break;
			case VK_OEM_PERIOD:
				hk_str+=GetOemChar(L'>', VK_OEM_PERIOD);
				break;
			case VK_OEM_2:
				hk_str+=GetOemChar(L'?', VK_OEM_2);
				break;
			case VK_OEM_3:
				hk_str+=GetOemChar(L'~', VK_OEM_3);
				break;
			case VK_OEM_4:
				hk_str+=GetOemChar(L'{', VK_OEM_4);
				break;
			case VK_OEM_5:
				hk_str+=GetOemChar(L'|', VK_OEM_5);
				break;
			case VK_OEM_6:
				hk_str+=GetOemChar(L'}', VK_OEM_6);
				break;
			case VK_OEM_7:
				hk_str+=GetOemChar(L'"', VK_OEM_7);
				break;
			case VK_OEM_8:
				hk_str+=(wchar_t)MapVirtualKey(VK_OEM_8, MAPVK_VK_TO_CHAR);
				break;
			case VK_OEM_102:
				hk_str+=GetOemChar(L'|', VK_OEM_7);
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
				if ((vk>=0x30&&vk<=0x39)||(vk>=0x41&&vk<=0x5A)||(vk>=0xE9&&vk<=0xF5)) {
					hk_str+=(wchar_t)MapVirtualKey(vk, MAPVK_VK_TO_CHAR);
				} else if (vk>=0x60&&vk<=0x69) {
					hk_str+=L"Num";
					hk_str+=std::to_wstring(vk-0x60);
				} else if (vk>=0x70&&vk<=0x87) {
					hk_str+=L"F";
					hk_str+=std::to_wstring(vk-0x6F);
				} else {
					std::wstringstream hex_vk;
					hex_vk<<L"0x"<<std::hex<<std::noshowbase<<std::uppercase<<std::setfill(L'0')<<std::setw(2)<<vk;
					hk_str+=hex_vk.str();
				}
		}
	}
	
	return hk_str;
}
