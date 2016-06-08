#include "TaskbarNotificationAreaIcon.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include "Res.h"
#include <memory>
#include <iostream>
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
		MessageBox(NULL, L"Failed to create icon!", L"SNK_HS", MB_OK);
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
	//OnKeyTriplet can be passed as refernce (wrapped in std::ref) or as pointer
	if (!SnkHotkey->StartNew(std::bind(&KeyTriplet::OnKeyPress, std::ref(OnKeyTriplet), std::placeholders::_1, std::placeholders::_2))) {
		MessageBox(NULL, L"Failed to set keyboard hook!", L"SNK_HS", MB_OK);
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
	
	return 0;	//Instead of returning WM_QUIT wParam, always return 0 
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
		default:
			return false;
	}
	
	//We get there after break which happens instead of return in all cases where hk_engine should have restarted but failed
	MessageBox(NULL, L"Failed to restart keyboard hook!", L"SNK_HS", MB_OK);
	sender->CloseAndQuit();
	return true;
}