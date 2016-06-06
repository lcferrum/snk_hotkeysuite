#include "TaskbarNotificationAreaIcon.h"
#include "SnkExternalRelations.h"
#include "HotkeyEngine.h"
#include "Res.h"
#include <iostream>
#include <windows.h>

#define SC_LCONTROL 0x01D
#define SC_RCONTROL 0x11D
#define SC_LSHIFT 0x02A
#define SC_RSHIFT 0x136 // Must be extended, at least on WinXP, or there will be problems, e.g. SetModifierLRState().
#define SC_LALT 0x038
#define SC_RALT 0x138
#define SC_LWIN 0x15B
#define SC_RWIN 0x15C

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
	
	//OnCtrlAltBS, OnCtrlShiftTld, OnCtrlShiftEsc and state are accessed from hook thread - don't touch them in main thread if hook thread is running
	DWORD state=0x0;
	HotkeyEngine::KeyPressFn OnCtrlAltBS=[&state](HotkeyEngine* sender, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event){ 
		bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
		DWORD vk=kb_event->vkCode;
		
		if (vk==VK_LCONTROL||vk==VK_RCONTROL||vk==VK_CONTROL) {
			if (key_up)
				state&=~0x1;
			else
				state|=0x1;
		}
		
		if (vk==VK_LMENU||vk==VK_RMENU||vk==VK_MENU) {
			if (key_up)
				state&=~0x2;
			else
				state|=0x2;
		}
		
		if (vk==VK_BACK) {
			if (key_up)
				state&=~0x4;
			else
				state|=0x4;
		}
		
		if (!key_up&&state==0x7) {
			MessageBeep(MB_ICONINFORMATION);
			return true;
		}
		
		return false; 
	};
	HotkeyEngine::KeyPressFn OnCtrlShiftEsc=[&state](HotkeyEngine* sender, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event){ 
		bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
		DWORD vk=kb_event->vkCode;
		
		if (vk==VK_LCONTROL||vk==VK_RCONTROL||vk==VK_CONTROL) {
			if (key_up)
				state&=~0x1;
			else
				state|=0x1;
		}
		
		if (vk==VK_LSHIFT||vk==VK_RSHIFT||vk==VK_SHIFT) {
			if (key_up)
				state&=~0x2;
			else
				state|=0x2;
		}
		
		if (vk==VK_ESCAPE) {
			if (key_up)
				state&=~0x4;
			else
				state|=0x4;
		}
		
		if (!key_up&&state==0x7) {
			MessageBeep(MB_ICONINFORMATION);
			return true;
		}
		
		return false; 
	};
	DWORD tilde_en_US=LOBYTE(VkKeyScanEx(L'~', LoadKeyboardLayout(L"00000409", 0)));
	DWORD tilde_cur=LOBYTE(VkKeyScanEx(L'~', GetKeyboardLayout(0)));
	DWORD tilde_any=255;
	if (DWORD hkl_len=GetKeyboardLayoutList(0, NULL)) {
		HKL hkl_lst[hkl_len];
		GetKeyboardLayoutList(hkl_len, hkl_lst);
		for (int hkl_i=0; hkl_i<hkl_len; hkl_i++)
			if ((tilde_any=LOBYTE(VkKeyScanEx(L'~', hkl_lst[hkl_i])))!=255) break;
	}
	HotkeyEngine::KeyPressFn OnCtrlShiftTld=[&state, tilde_any](HotkeyEngine* sender, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event){ 
		bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
		DWORD vk=kb_event->vkCode;
		
		if (vk==VK_LCONTROL||vk==VK_RCONTROL||vk==VK_CONTROL) {
			if (key_up)
				state&=~0x1;
			else
				state|=0x1;
		}
		
		if (vk==VK_LSHIFT||vk==VK_RSHIFT||vk==VK_SHIFT) {
			if (key_up)
				state&=~0x2;
			else
				state|=0x2;
		}
		
		//if (vk==VK_OEM_3) {		//In reality it's not tilde character but rather "key that on en-US layout OFTEN has tilde on it"
		//if (vk==tilde_en_US) {	//"key that on en-US layout ACTUALLY corresponds to tilde"
		//if (vk==tilde_cur) {		//"key that on CURRENT layout corresponds to tilde"
		if (vk==tilde_any) {		//"key that on SOME of the installed layouts (starting from system) corresponds to tilde"
			if (key_up)
				state&=~0x4;
			else
				state|=0x4;
		}
		
		if (!key_up&&state==0x7) {
			MessageBeep(MB_ICONINFORMATION);
			return true;
		}
		
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
	if (!SnkHotkey->StartNew(std::move(OnCtrlShiftEsc))) {
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