#include "TaskbarNotificationAreaIcon.h"

#define ICON_UID	0	//Only one icon allowed

std::unique_ptr<TskbrNtfAreaIcon> TskbrNtfAreaIcon::instance;
UINT TskbrNtfAreaIcon::WmTaskbarCreated=RegisterWindowMessage(L"TaskbarCreated");
std::function<bool(TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam)> TskbrNtfAreaIcon::OnWmCommand;

TskbrNtfAreaIcon* TskbrNtfAreaIcon::MakeInstance(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid, const wchar_t* icon_class, UINT icon_menuid, UINT default_menuid, WmCommandFn OnWmCommand)
{
	instance.reset(nullptr);	//Before assigning new OnWmCommand make sure that previous instance is destroyed
	TskbrNtfAreaIcon::OnWmCommand=std::move(OnWmCommand);
	instance.reset(new TskbrNtfAreaIcon(hInstance, icon_wm, icon_tooltip, icon_resid, icon_class, icon_menuid, default_menuid));
	return instance.get();
}

TskbrNtfAreaIcon::~TskbrNtfAreaIcon() 
{
	Close();	//Instance still exists while this function is called
}

//Using first version of NOTIFYICONDATA to be compatible with pre-Win2000 OS versions
TskbrNtfAreaIcon::TskbrNtfAreaIcon(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid, const wchar_t* icon_class, UINT icon_menuid, UINT default_menuid):
	valid(false), app_instance(hInstance), icon_menu(NULL), default_menuid(default_menuid), icon_atom(0), icon_ntfdata{
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
		MAKEINTRESOURCE(icon_menuid),		//lpszMenuName
		icon_class,							//lpszClassName
		0									//hIconSm
	};

	//Fail on all error including already registered class
	//Registered class could have been registered elsewhere with totally unpredictable WNDCLASSEX
	if (!(icon_atom=RegisterClassEx(&wcex)))
		return;
	
	if (!(icon_ntfdata.hWnd=CreateWindow(MAKEINTATOM(icon_atom), L"", WS_POPUP,					//This thing internally calls WNDPROC with WM_CREATE message without posting it to message queue
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0, hInstance, 0))) {	//But instance is not ready yet when WNDPROC is called because window is created inside constructor
		UnregisterClass(MAKEINTATOM(icon_atom), hInstance);
		return;
	}
		
	wcsncpy(icon_ntfdata.szTip, icon_tooltip, 63);	//First version of NOTIFYICONDATA only allowes szTip no more than 64 characters in length (including NULL-terminator)
	
	if (!Shell_NotifyIcon(NIM_ADD, &icon_ntfdata)) {
		DestroyWindow(icon_ntfdata.hWnd);			//This thing internally calls WNDPROC with WM_DESTROY message without posting it to message queue (again, instance not ready - message will be ignored)
		UnregisterClass(MAKEINTATOM(icon_atom), hInstance);
		return;
	}
	
	icon_menu=GetSubMenu(GetMenu(icon_ntfdata.hWnd), 0);
	SetMenuDefaultItem(icon_menu, default_menuid, FALSE);
	
	valid=true;
}

bool TskbrNtfAreaIcon::IsValid()
{
	return valid;
}


HMENU TskbrNtfAreaIcon::GetIconMenu()
{
	if (valid)
		return icon_menu;
	else
		return NULL;
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

//Calling this function won't exit message loop!
//It just destroyes window, icon and unregisters class
void TskbrNtfAreaIcon::Close()
{
	if (!valid)
		return;
	
	icon_ntfdata.uFlags=0;
	Shell_NotifyIcon(NIM_DELETE, &icon_ntfdata);
	DestroyWindow(icon_ntfdata.hWnd);					//This thing internally calls WNDPROC with WM_DESTROY message without posting it to message queue
	UnregisterClass(MAKEINTATOM(icon_atom), app_instance);
	valid=false;
}

//Calling this function will destroy window, icon, unregister class and then exit message loop
//It is equivalent to calling PostQuitMessage() inside WM_DESTROY message handler
void TskbrNtfAreaIcon::CloseAndQuit()
{
	Close();
	
	//PostQuitMessage() will prevent any MessageBoxes from showing until message loop have exited 
	//Any other code after it will be executed as usual
	PostQuitMessage(0);
}

BOOL TskbrNtfAreaIcon::ModifyIconMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCWSTR lpNewItem)
{
	if (!valid)
		return FALSE;
	
	return ModifyMenu(icon_menu, uPosition, uFlags, uIDNewItem, lpNewItem);
}

BOOL TskbrNtfAreaIcon::EnableIconMenuItem(UINT uIDEnableItem, UINT uEnable)
{
	if (!valid)
		return FALSE;
	
	return EnableMenuItem(icon_menu, uIDEnableItem, uEnable);
}

LRESULT CALLBACK TskbrNtfAreaIcon::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Why checking instance and hWnd?
	//First of all because some message handlers use instance and theoretically it can be not ready yet when handler is called (e.g. WM_CREATE)
	//Second, someone can have a bright idea to reuse icon window class registered here for some other window - we don't want to process alien messages
	if (instance&&instance->icon_ntfdata.hWnd==hWnd) {
		switch (message) {
			case WM_SETTINGCHANGE:		//WM_SETTINGCHANGE with wParam set to SPI_SETWORKAREA signal that icon should be recreated
				if (wParam==SPI_SETWORKAREA)
					Shell_NotifyIcon(NIM_ADD, &instance->icon_ntfdata);
				return 0;
			case WM_COMMAND:
				if (OnWmCommand&&OnWmCommand(instance.get(), wParam, lParam))
					return 0;
				break;	//Let DefWindowProc handle the rest of WM_COMMAND variations if OnWmCommand returned FALSE
			default:
				//Non-const cases goes here
				if (message==instance->icon_ntfdata.uCallbackMessage) {	//Messages that are sent to taskbar icon
					//For the first version of NOTIFYICONDATA lParam (as a whole, not just LOWORD) holds the mouse or keyboard messages and wParam holds icon ID
					if (wParam==ICON_UID) {
						switch (lParam) {
							case WM_RBUTTONUP:
								POINT cur_mouse_pt;
								GetCursorPos(&cur_mouse_pt);
								SetForegroundWindow(hWnd);
								TrackPopupMenu(instance->icon_menu, TPM_LEFTBUTTON|TPM_LEFTALIGN, cur_mouse_pt.x, cur_mouse_pt.y, 0, hWnd, NULL);
								SendMessage(hWnd, WM_NULL, 0, 0);
								return 0;
							case WM_LBUTTONDBLCLK:
								if (OnWmCommand)
									OnWmCommand(instance.get(), instance->default_menuid, 0);
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
