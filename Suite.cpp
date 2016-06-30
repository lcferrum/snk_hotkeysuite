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
std::wstring GetHotkeyString(SuiteSettings::ModKeyType mod_key, DWORD vk, GetHotkeyStringType type, const wchar_t* prefix=NULL, const wchar_t* postfix=NULL);

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
	} else {
		SnkIcon->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
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
	SnkIcon->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(SuiteSettings::DONT_CARE, Settings.GetBindedVK(), GetHotkeyStringType::VK, L"Rebind ", L"...").c_str());
	//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
	SnkIcon->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(SnkIcon->GetIconMenu(), 2), GetHotkeyString(Settings.GetModKey(), Settings.GetBindedVK(), GetHotkeyStringType::FULL).c_str()); 

	
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
	
	//Manually uninitializing some components to make sure right unintialization order
	SnkHotkey->Stop();
	
	return msg.wParam;
}

#define BindingDialogProc_HK_ENGINE(lParam)	std::get<0>(*(std::tuple<HotkeyEngine*, DWORD, DWORD>*)lParam)
#define BindingDialogProc_OLD_VK(lParam)	std::get<1>(*(std::tuple<HotkeyEngine*, DWORD, DWORD>*)lParam)
#define BindingDialogProc_NEW_VK(lParam)	std::get<2>(*(std::tuple<HotkeyEngine*, DWORD, DWORD>*)lParam)
INT_PTR CALLBACK BindingDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				SetWindowText(hwndDlg, GetHotkeyString(SuiteSettings::DONT_CARE, BindingDialogProc_OLD_VK(lParam), GetHotkeyStringType::VK, L"Rebind ").c_str());
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				//If we fail with starting binding keyboard hook - exit immediately with -1 result which indicates error
				if (!BindingDialogProc_HK_ENGINE(lParam)->StartNew(std::bind(BindKey, hwndDlg, WM_BINDVK, std::placeholders::_1, std::placeholders::_2)))
					EndDialog(hwndDlg, -1);
				return TRUE;
			}
		case WM_BINDVK:
			if (!BindingDialogProc_NEW_VK(GetWindowLongPtr(hwndDlg, DWLP_USER))) {
				BindingDialogProc_NEW_VK(GetWindowLongPtr(hwndDlg, DWLP_USER))=wParam;
				BindingDialogProc_HK_ENGINE(GetWindowLongPtr(hwndDlg, DWLP_USER))->Stop();
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM_VK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_VK), TRUE);
				//We should set focus to default button (it's not set by default because button was disabled) but without bypassing dialog manager: https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_CONFIRM_VK), TRUE);
				SetDlgItemText(hwndDlg, IDC_VK_VIEWER, GetHotkeyString(SuiteSettings::DONT_CARE, wParam, GetHotkeyStringType::VK, L"Rebind to ", L"?").c_str());
			}
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			BindingDialogProc_HK_ENGINE(GetWindowLongPtr(hwndDlg, DWLP_USER))->Stop();
			EndDialog(hwndDlg, 0);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			//They are enabled only when binding hook has already stopped
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_CONFIRM_VK:
						EndDialog(hwndDlg, 1);
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
#undef BindingDialogProc_HK_ENGINE
#undef BindingDialogProc_OLD_VK
#undef BindingDialogProc_NEW_VK

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam)
{
	//Not checking if hk_engine is NULL in menu handlers - handlers won't be called until message loop is fired which happens after creation of hk_engine
	bool hk_was_running=false;
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
				sender->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
				settings->SetLongPress(false);
				hk_triplet->SetLongPress(false);
			} else {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED);
				sender->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_ENABLED);
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
			{
				std::tuple<HotkeyEngine*, DWORD, DWORD> bindingdlg_tuple;
				bindingdlg_tuple=std::make_tuple(hk_engine, settings->GetBindedVK(), 0);
				//The fun thing with DialogBoxParam is that it won't return until DLGPROC exits with EndDialog but in the same time it won't block thread execution
				//It creates new message loop within current message loop, while latter remains blocked because current message dispatch hasn't ended (it waits for DialogBoxParam to return)
				//But other windows still respond because this new message loop besides it's own DLGPROC happily handles all other thread's window processes
				//When EndDialog is called within DLGPROC, it exits DialogBoxParam's message loop, finishes message dispatch that called DialogBoxParam in the first place and returns to thread's original message loop
				//Q: What happens when DialogBoxParam called several times without exiting previous dialogs?
				//A: We get several nested message loops and all dialog/window processes will be handled by last created loop
				//Q: What happens if I use PostQuitMessage with one or several dialogs running?
				//A: All present message loops will exit (including thread's own, starting from the last created) and DialogBoxParam will return 0 every time it's message loop exits
				//Q: What happens when I open several dialogs and exit the one that wasn't the last created?
				//A: DialogBoxParam won't return and last created message loop will still run until it's own dialog exits
				//A: In the end, randomly closing dialogs, we get proper nested message loop unwinding (i.e. DialogBoxParam will return sequentially starting from the last called)
				//All in all, it's better disable icon completely so binding dialog won't be called second time and no other menu items can be clicked
				sender->Enable(false);
				hk_was_running=hk_engine->Stop();
				//Several words on InitCommonControls() and InitCommonControlsEx()
				//"Common" name is somewhat misleading
				//Even though controls like "static", "edit" and "button" are common (like in common sence) they are actually "standard" controls
				//So there is no need to initialize them with InitCommonControls() or InitCommonControlsEx() functions
				//Though ICC_STANDARD_CLASSES can be passed to InitCommonControlsEx, it actually does nothing - "standard" controls are really initialized by the system
				//Also these functions has nothing to do with "Win XP"/"ComCtl32 v6+" style - just supply proper manifest to make "standard" controls use it
				//IDD_BINDINGDLG uses only "standard" controls
				switch (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_BINDINGDLG), sender->GetIconWindow(), BindingDialogProc, (LPARAM)&bindingdlg_tuple)) {
					//DialogBoxParam will return 0 on Cancel, -1 on fail and 1 on Confirm
					case -1:
						break;
					case 1:
						hk_triplet->SetBindedVK(std::get<2>(bindingdlg_tuple));
						settings->SetBindedVK(std::get<2>(bindingdlg_tuple));
						sender->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(SuiteSettings::DONT_CARE, std::get<2>(bindingdlg_tuple), GetHotkeyStringType::VK, L"Rebind ", L"...").c_str());
						//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
						sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(settings->GetModKey(), std::get<2>(bindingdlg_tuple), GetHotkeyStringType::FULL).c_str()); 
					case 0:
						if (hk_was_running&&!hk_engine->StartNew(std::bind(&KeyTriplet::OnKeyPress, hk_triplet, std::placeholders::_1, std::placeholders::_2))) break;
						sender->Enable();
						return true;
				}
				break;
			}
		default:
			return false;
	}
	
	//We get there after break which happens instead of return in all cases where hk_engine should have restarted but failed
	MessageBox(NULL, L"Failed to restart keyboard hook!", L"SNK_HS", MB_ICONERROR|MB_OK);
	sender->CloseAndQuit(3);
	return true;
}

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
