#include "TaskbarNotificationAreaIcon.h"
#include "SnkExternalRelations.h"
#include "HotkeyEngine.h"
#include "Res.h"
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
	
	TskbrNtfAreaIcon::WmCommandFn OnWmCommand=[&SnkHotkey](TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam){
		switch (LOWORD(wParam)) {
			case IDM_EXIT:
				//We can just use PostQuitMessage() and wait for TskbrNtfAreaIcon destructor to destroy icon at the end of the program
				//But in this case icon will be briefly present after the end of message loop till the end of WinMain, though being unresponsive
				//It will be better to destroy the icon right away and then exit message loop
				//And after that do all other uninitialization without icon being visible for unknown purpose
				sender->CloseAndQuit();
				return true;
			case IDM_STOP_START:
				if (SnkHotkey) {
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
				}
				return true;
			case IDM_EDIT_SHK:
				MessageBox(NULL, L"ID_EDIT_SHK", L"SNK_HS", MB_OK);
				return true;
			case IDM_EDIT_LHK:
				MessageBox(NULL, L"ID_EDIT_LHK", L"SNK_HS", MB_OK);
				return true;
			default:
				return false;
		}
	};
	
	HotkeyEngine::KeyPressFn OnKeyPress=[](HotkeyEngine* sender, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event){ 
		if (wParam==WM_KEYUP)
			MessageBeep(MB_ICONINFORMATION);
		return false; 
	};
	
	SnkIcon=TskbrNtfAreaIcon::MakeInstance(hInstance, WM_HSTNAICO, L"SNK_HS: RUNNING", IDI_HSTNAICO, L"SnK_HotkeySuite_IconClass", IDR_ICONMENU, IDM_STOP_START, std::move(OnWmCommand));
	if (!SnkIcon->IsValid()) {
		MessageBox(NULL, L"Failed to create icon!", L"SNK_HS", MB_OK);
		return 0;
	}
	
	//At this point taskbar icon is already visible but unusable - it doesn't respond to any clicks and can't show popup menu
	//So it's ok to customize menu here and initialize everything else
	SnkIcon->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
	SnkHotkey=HotkeyEngine::MakeInstance(hInstance);
	if (!SnkHotkey->StartNew(std::move(OnKeyPress))) {
		SnkIcon->ChangeIconTooltip(L"SNK_HS: STOPPED");
		SnkIcon->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Start"); 
	}
	
	//Main thread's message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)>0) {	//GetMessage returns -1 if error (probably happens only with invalid input parameters) and 0 if WM_QUIT so continue only on positive result
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return 0;	//Instead of returning WM_QUIT wParam, always return 0 
}