#include "TaskbarNotificationAreaIcon.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include "Res.h"
#include <memory>
#include <iostream>
#include <windows.h>

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
	KeyTriplet* OnKeyTriplet;	//Can't use stack variable between threads - creating it later on a heap with new
	
	TskbrNtfAreaIcon::WmCommandFn OnWmCommand=[&SnkHotkey, &Settings, &OnKeyTriplet](TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam){
		//Int this lambda we are not checking if SnkHotkey or OnKeyTriplet is NULL in menu handlers
		//It's because handlers won't be called until message loop is fired which happens after creation of SnkHotkey and OnKeyTriplet
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
				if (SnkHotkey->IsRunning()) {
					SnkHotkey->Stop();
					sender->ChangeIconTooltip(L"SNK_HS: STOPPED");
					sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Start"); 
				} else {
					if (SnkHotkey->Start()) {
						sender->ChangeIconTooltip(L"SNK_HS: RUNNING");
						sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Stop"); 
					}
				}
				return true;
			case IDM_EDIT_SHK:
				MessageBox(NULL, L"ID_EDIT_SHK", L"SNK_HS", MB_OK);
				return true;
			case IDM_EDIT_LHK:
				MessageBox(NULL, L"ID_EDIT_LHK", L"SNK_HS", MB_OK);
				return true;
			case IDM_SET_EN_LHK:
				hk_was_running=SnkHotkey->Stop();
				if (Settings.GetLongPress()) {
					sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_UNCHECKED);
					Settings.SetLongPress(false);
					OnKeyTriplet->SetLongPress(false);
				} else {
					sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED);
					Settings.SetLongPress(true);
					OnKeyTriplet->SetLongPress(true);
				}
				if (hk_was_running) SnkHotkey->Start();
				return true;
			case IDM_SET_CTRL_ALT:
				hk_was_running=SnkHotkey->Stop();
				sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
				Settings.SetModKey(SuiteSettings::CTRL_ALT);
				OnKeyTriplet->SetCtrlAlt();
				if (hk_was_running) SnkHotkey->Start();
				return true;
			case IDM_SET_SHIFT_ALT:
				hk_was_running=SnkHotkey->Stop();
				sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
				Settings.SetModKey(SuiteSettings::SHIFT_ALT);
				OnKeyTriplet->SetShiftAlt();
				if (hk_was_running) SnkHotkey->Start();
				return true;
			case IDM_SET_CTRL_SHIFT:
				hk_was_running=SnkHotkey->Stop();
				sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
				Settings.SetModKey(SuiteSettings::CTRL_SHIFT);
				OnKeyTriplet->SetCtrlShift();
				if (hk_was_running) SnkHotkey->Start();
				return true;
			default:
				return false;
		}
	};
	
	SnkIcon=TskbrNtfAreaIcon::MakeInstance(hInstance, WM_HSTNAICO, L"SNK_HS: RUNNING", IDI_HSTNAICO, L"SnK_HotkeySuite_IconClass", IDR_ICONMENU, IDM_STOP_START, std::move(OnWmCommand));
	if (!SnkIcon->IsValid()) {
		MessageBox(NULL, L"Failed to create icon!", L"SNK_HS", MB_OK);
		return 1;
	}
	
	//At this point taskbar icon is already visible but unusable - it doesn't respond to any clicks and can't show popup menu
	//So it's ok to customize menu here and initialize everything else
	SnkHotkey=HotkeyEngine::MakeInstance(hInstance);
	OnKeyTriplet=new KeyTriplet();
	if (Settings.GetLongPress()) {
		SnkIcon->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED); 
		OnKeyTriplet->SetLongPress(true);
	}
	switch (Settings.GetModKey()) {
		case SuiteSettings::CTRL_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			OnKeyTriplet->SetCtrlAlt();
			break;
		case SuiteSettings::SHIFT_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			OnKeyTriplet->SetShiftAlt();
			break;
		case SuiteSettings::CTRL_SHIFT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			OnKeyTriplet->SetCtrlShift();
			break;
	}
	if (!SnkHotkey->StartNew(std::bind(&KeyTriplet::OnKeyPress, OnKeyTriplet, std::placeholders::_1, std::placeholders::_2))) {
		MessageBox(NULL, L"Failed to set keyboard hook!", L"SNK_HS", MB_OK);
		delete OnKeyTriplet;
		return 1;
	}
	//SnkIcon->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
	
	//Main thread's message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)>0) {	//GetMessage returns -1 if error (probably happens only with invalid input parameters) and 0 if WM_QUIT so continue only on positive result
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	//Manually uninitializng some components to make sure right unintializtion order
	SnkHotkey->Stop();
	delete OnKeyTriplet;
	
	return 0;	//Instead of returning WM_QUIT wParam, always return 0 
}