#include "TaskbarNotificationAreaIcon.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include "Res.h"
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <windows.h>

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam);

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
	std::wstringstream hex_vk;
	switch (uMsg) {
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg, IDC_VK_VIEWER, L"0x00");
			SetWindowLongPtr(hwndDlg, DWLP_USER, 0);
			//If we fail with startin binding keyboard hook - exit immediately with 0 result
			//Callee will determine that it was error because HotkeyEngine::Stop() will return false
			//It's callee responsibility to stop this hook
			if (!((HotkeyEngine*)lParam)->StartNew(std::bind(BindKey, hwndDlg, WM_BINDVK, std::placeholders::_1, std::placeholders::_2)))
				EndDialog(hwndDlg, 0);
			return TRUE;
		case WM_BINDVK:
			hex_vk<<L"0x"<<std::hex<<std::noshowbase<<std::uppercase<<std::setfill(L'0')<<std::setw(2)<<wParam<<L" ("<<(wchar_t)MapVirtualKey(wParam, MAPVK_VK_TO_CHAR)<<L")";
			SetDlgItemText(hwndDlg, IDC_VK_VIEWER, hex_vk.str().c_str());
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
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_SHIFT_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			settings->SetModKey(SuiteSettings::SHIFT_ALT);
			hk_triplet->SetShiftAlt();
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_SHIFT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			settings->SetModKey(SuiteSettings::CTRL_SHIFT);
			hk_triplet->SetCtrlShift();
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
			bind_vk=DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_BINDINGDLG), sender->GetIconWindow(), BindingDialogProc, (LPARAM)hk_engine);
			//Inside dialog's DLGPROC we are starting new binding keyboard hook
			//If this fail - HotkeyEngine::Stop() will return false because hook wasn't started
			if (!hk_engine->Stop()) break;
			if (bind_vk) hk_triplet->SetBindedVK(bind_vk);
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