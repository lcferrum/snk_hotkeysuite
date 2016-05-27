#include "TaskbarNotificationAreaIcon.h"
#include "Res.h"
#include <cstdio>
#include <cwchar>
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
	TskbrNtfAreaIcon::OnWmCommand=[](TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam){
		switch (LOWORD(wParam)) {
			case ID_EXIT:
				MessageBox(NULL, L"ID_EXIT", L"SNK_HS", MB_OK);
				PostQuitMessage(0);
				return true;
			case ID_STOP_START:
				MessageBox(NULL, L"ID_STOP_START", L"SNK_HS", MB_OK);
				return true;
			case ID_EDIT_SHK:
				MessageBox(NULL, L"ID_EDIT_SHK", L"SNK_HS", MB_OK);
				return true;
			case ID_EDIT_LHK:
				MessageBox(NULL, L"ID_EDIT_LHK", L"SNK_HS", MB_OK);
				return true;
			default:
				return false;
		}
	};
	
	if (!TskbrNtfAreaIcon::MakeInstance(hInstance, WM_HSTNAICO, L"SNK_HS", IDI_HSTNAICO, L"SnK_HotkeySuite_IconClass", IDR_ICONMENU, ID_EXIT)->IsValid()) {
		MessageBox(NULL, L"Failed to create icon!", L"SNK_HS", MB_OK);
		return 0;
	}
	
	//At this point taskbar icon is already visible but unusable - it doesn't respond to any clicks and can't show popup menu
	//So it's ok to customize menu here
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} 
	
	return msg.wParam;
}