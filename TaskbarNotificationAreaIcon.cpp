#include "TaskbarNotificationAreaIcon.h"
#include "Res.h"

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

//Using first version of NOTIFYICONDATA to be compatible with pre-Win2000 OS versions
TskbrNtfAreaIcon::TskbrNtfAreaIcon(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid):
	valid(false), app_instance(hInstance), icon_ntfdata{
		NOTIFYICONDATA_V1_SIZE, 							//cbSize
		NULL, 												//hWnd (will set it later)
		ICON_UID, 											//uID
		NIF_MESSAGE|NIF_ICON|NIF_TIP, 						//uFlags
		icon_wm,											//uCallbackMessage
		LoadIcon(hInstance, MAKEINTRESOURCE(icon_resid))	//hIcon (szTip and uVersion are initialized with NULLs)
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
		MAKEINTRESOURCE(IDR_ICONMENU),		//lpszMenuName
		ICON_CLASS,							//lpszClassName
		0									//hIconSm
	};

    if (!RegisterClassEx(&wcex)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)	//Exit only if registration failed not because of already registered class
		return;
	
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
		switch (message) {
			case WM_CREATE:
				return 0;
			case WM_SETTINGCHANGE:		//WM_SETTINGCHANGE with wParam set to SPI_SETWORKAREA signal that icon should be recreated
				if (wParam==SPI_SETWORKAREA)
					Shell_NotifyIcon(NIM_ADD, &instance->icon_ntfdata);
				return 0;
			case WM_COMMAND:
				switch (LOWORD(wParam)) {
					case ID_EXIT:
						MessageBox(NULL, L"ID_EXIT", L"SNK_HS", MB_OK);
						PostQuitMessage(0);
						return 0;
					case ID_STOP_START:
						MessageBox(NULL, L"ID_STOP_START", L"SNK_HS", MB_OK);
						return 0;
					case ID_EDIT_SHK:
						MessageBox(NULL, L"ID_EDIT_SHK", L"SNK_HS", MB_OK);
						return 0;
					case ID_EDIT_LHK:
						MessageBox(NULL, L"ID_EDIT_LHK", L"SNK_HS", MB_OK);
						return 0;
				}
				break;	//Let DefWindowProc handle the rest of WM_COMMAND variations
			default:
				//Non-const cases goes here
				if (message==instance->icon_ntfdata.uCallbackMessage) {	//Taskbar notification area icon various clicks
					//For the first version of NOTIFYICONDATA lParam (as a whole, not just LOWORD) holds the mouse or keyboard message and wParam holds icon ID
					if (wParam==ICON_UID) {
						switch (lParam) {
							case WM_RBUTTONUP:
								POINT cur_mouse_pt;
								SetMenuDefaultItem(GetSubMenu(GetMenu(hWnd), 0), ID_EXIT, FALSE);
								GetCursorPos(&cur_mouse_pt);
								SetForegroundWindow(hWnd);
								TrackPopupMenu(GetSubMenu(GetMenu(hWnd), 0), TPM_LEFTBUTTON|TPM_LEFTALIGN, cur_mouse_pt.x, cur_mouse_pt.y, 0, hWnd, NULL);
								SendMessage(hWnd, WM_NULL, 0, 0);
								return 0;
							case WM_LBUTTONDBLCLK:
								MessageBox(NULL, L"WM_LBUTTONDBLCLK", L"SNK_HS", MB_OK);
								return 0;
						}
					}
					//Let DefWindowProc handle the rest of uCallbackMessage variations
				} else if (message==WmTaskbarCreated) {	//WM_TASKBARCREATED signal that icon should be recreated
					Shell_NotifyIcon(NIM_ADD, &instance->icon_ntfdata);
					return 0;
				}
				//Let DefWindowProc handle all other messages
		}
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}
