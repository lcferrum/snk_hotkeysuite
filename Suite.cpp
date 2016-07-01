#include "TaskbarNotificationAreaIcon.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include "SuiteCommon.h"
#include "Res.h"
#include <memory>
#include <tuple>
#include <string>
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
				//The fun thing with DialogBoxParam is that it won't return until DLGPROC exits with EndDialog but in the same time it won't block thread execution
				//It creates new message loop within current message loop, while latter remains blocked because current message dispatch hasn't ended (it waits for DialogBoxParam to return)
				//But other windows still respond because this new message loop besides it's own DLGPROC happily handles all other thread's window processes
				//When EndDialog is called within DLGPROC, it exits DialogBoxParam's message loop, finishes message dispatch that called DialogBoxParam in the first place and returns to thread's original message loop
				//Q: What happens when DialogBoxParam called several times without exiting previous dialogs?
				//A: We get several nested message loops and all dialog/window processes will be handled by the last created loop
				//Q: What happens if I use PostQuitMessage with one or several dialogs running?
				//A: All present message loops will sequentially exit (including thread's own, starting from the last created) and DialogBoxParam will return 0 every time it's message loop exits
				//Q: What happens when I open several dialogs and exit the one that wasn't the last created?
				//A: DialogBoxParam won't return and last created message loop will still run until it's own dialog exits
				//A: In the end, randomly closing dialogs, we get proper nested message loop unwinding (i.e. DialogBoxParam will return sequentially starting from the last called)
				//All in all, it's better disable icon completely so binding dialog won't be called second time and no other menu items can be clicked
				sender->Enable(false);
				hk_was_running=hk_engine->Stop();
				BINDING_DLGPRC_PARAM bd_dlgprc_param={hk_engine, settings->GetBindedVK(), 0};
				//Several words on InitCommonControls() and InitCommonControlsEx()
				//"Common" name is somewhat misleading
				//Even though controls like "static", "edit" and "button" are common (like in common sence) they are actually "standard" controls
				//So there is no need to initialize them with InitCommonControls() or InitCommonControlsEx() functions
				//Though ICC_STANDARD_CLASSES can be passed to InitCommonControlsEx, it actually does nothing - "standard" controls are really initialized by the system
				//Also these functions has nothing to do with "Win XP"/"ComCtl32 v6+" style - just supply proper manifest to make "standard" controls use it
				//IDD_BINDINGDLG uses only "standard" controls
				switch (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_BINDINGDLG), sender->GetIconWindow(), BindingDialogProc, (LPARAM)&bd_dlgprc_param)) {
					//DialogBoxParam will return 0 on Cancel, -1 on fail and 1 on Confirm
					case -1:
						break;
					case 1:
						hk_triplet->SetBindedVK(bd_dlgprc_param.binded_vk);
						settings->SetBindedVK(bd_dlgprc_param.binded_vk);
						sender->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(SuiteSettings::DONT_CARE, bd_dlgprc_param.binded_vk, GetHotkeyStringType::VK, L"Rebind ", L"...").c_str());
						//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
						sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(settings->GetModKey(), bd_dlgprc_param.binded_vk, GetHotkeyStringType::FULL).c_str()); 
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
