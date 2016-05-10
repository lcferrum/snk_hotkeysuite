#include "TaskbarNotificationAreaIcon.h"
#include "Res.h"
#include <cstdio>
#include <cwchar>
#include <iostream>
#include <windows.h>

#ifdef OBSOLETE_WINMAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
	LPWSTR lpCmdLine=GetCommandLineW();
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#endif
	MessageBox(NULL, L"This is really WinMain!", L"SNK_HS", MB_OK);
	
	TskbrNtfAreaIcon::MakeInstance(hInstance, WM_HSTRAYICO, L"SNK_HS", ID_HSTRAYICO);
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)>0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} 
	
	return msg.wParam;
}