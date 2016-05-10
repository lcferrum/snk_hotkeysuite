#include "TaskbarNotificationAreaIcon.h"

#define ICON_UID	0	//Only one icon allowed
#define ICON_CLASS	L"SnK_HotkeySuite_IconClass"

std::unique_ptr<TskbrNtfAreaIcon> TskbrNtfAreaIcon::instance;
UINT TskbrNtfAreaIcon::WmTaskbarCreated=RegisterWindowMessage(L"TaskbarCreated");

TskbrNtfAreaIcon* TskbrNtfAreaIcon::MakeInstance(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid)
{
	if (instance)
		return NULL;
	
	instance.reset(new TskbrNtfAreaIcon(hInstance, icon_wm, icon_tooltip, icon_resid));
	return instance.get();
}

TskbrNtfAreaIcon::~TskbrNtfAreaIcon() 
{
	if (valid) {
		icon_ntfdata.uFlags=0;
		Shell_NotifyIcon(NIM_DELETE, &icon_ntfdata);
	}
	
	if (icon_ntfdata.hWnd)
		DestroyWindow(icon_ntfdata.hWnd);
}

//Using first version of NOTIFYICONDATA to be copatible with pre-Win2000 OS versions
TskbrNtfAreaIcon::TskbrNtfAreaIcon(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid):
	valid(false), app_instance(hInstance), icon_ntfdata{
		NOTIFYICONDATA_V1_SIZE, 							//cbSize
		NULL, 												//hWnd
		ICON_UID, 											//uID
		NIF_MESSAGE|NIF_ICON|NIF_TIP, 						//uFlags
		icon_wm,											//uCallbackMessage
		LoadIcon(hInstance, MAKEINTRESOURCE(icon_resid))	//hIcon (szTip is initialized with NULLs)
	}
{
	WNDCLASSEX wcex={
		sizeof(WNDCLASSEX),					//cbSize
		CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,	//style
		(WNDPROC)WindowProc,				//lpfnWndProc
		0,									//cbClsExtra
		0,									//cbWndExtra
		hInstance,							//hInstance
		0,									//hIcon
		0,									//hCursor
		0,									//hbrBackground
		0,									//lpszMenuName
		ICON_CLASS,							//lpszClassName
		0									//hIconSm
	};

    RegisterClassEx(&wcex);	//Won't check return parameter because this thing fails if class already registered (and we won't unregister it)
	
	if (!(icon_ntfdata.hWnd=CreateWindow(ICON_CLASS, L"", WS_POPUP, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0, hInstance, 0)))
		return;
		
	wcsncpy(icon_ntfdata.szTip, icon_tooltip, 63);	//First version of NOTIFYICONDATA only allowes szTip no more than 64 characters in length (including NULL-terminator)
	
	if (!Shell_NotifyIcon(NIM_ADD, &icon_ntfdata))
		return;
	
	valid=true;
}

bool TskbrNtfAreaIcon::IsValid()
{
	return valid;
}

void TskbrNtfAreaIcon::ChangeIconTooltip(const wchar_t* icon_tooltip)
{
	if (!valid)
		return;
	
	wcsncpy(icon_ntfdata.szTip, icon_tooltip, 63);	//First version of NOTIFYICONDATA only allowes szTip no more than 64 characters in length (including NULL-terminator)
	Shell_NotifyIcon(NIM_MODIFY, &icon_ntfdata);
}

void TskbrNtfAreaIcon::ChangeIcon(UINT icon_resid)
{
	if (!valid)
		return;
	
	icon_ntfdata.hIcon=LoadIcon(app_instance, MAKEINTRESOURCE(icon_resid));	
	Shell_NotifyIcon(NIM_MODIFY, &icon_ntfdata);
}

LRESULT CALLBACK TskbrNtfAreaIcon::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (instance&&instance->valid&&instance->icon_ntfdata.hWnd==hWnd) {
		//WM_SETTINGCHANGE (with wParam set to SPI_SETWORKAREA) and WM_TASKBARCREATED signal that icon should be recreated
		if (message==WM_SETTINGCHANGE) {
			if (wParam==SPI_SETWORKAREA)
				Shell_NotifyIcon(NIM_ADD, &instance->icon_ntfdata);
			return 0;
		}
		if (message==WmTaskbarCreated) {
			Shell_NotifyIcon(NIM_ADD, &instance->icon_ntfdata);
			return 0;
		}
			
		//Various button clicks on task bar icon
		if (message==instance->icon_ntfdata.uCallbackMessage) {
			if (wParam!=ICON_UID)
				return 0;
			
			switch (LOWORD(lParam)) {
				case WM_RBUTTONUP:
					MessageBox(NULL, L"WM_RBUTTONUP", L"SNK_HS", MB_OK);
					break;
				case WM_LBUTTONUP:
					MessageBox(NULL, L"WM_LBUTTONUP", L"SNK_HS", MB_OK);
					PostQuitMessage(0);
					break;
			}
			
			return 1;
		}
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}
