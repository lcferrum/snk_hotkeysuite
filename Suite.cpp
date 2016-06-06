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
	
	//Code below gets VK that contains tilde character
	//First it checks default layout (which is appended to the end of the list) and then other layouts starting from the last
	//Layout list is unorganized and doesn't follow system layout priority so default layout doesn't have predefined index
	//That's why we check default layout separately but at the same time as part of general layout list check loop
	//Downside - one  excessive check (because default layout is also present in layout list) but in return we have more uniform code
	//If actual VK not found - common VK_OEM_3 (top-left key) is used instead
	//Some examples:
	//Default ЙЦУКЕН ru-RU layout (no tilde) and additional QWERTY en-US layout (tilde on top-left key): vk_tilde=VK_OEM_3 (top-left key)
	//Default QWERTZ cs-CZ layout (tilde on "1" key) and additional QWERTY cs-CZ layout (tilde on top-left key): vk_tilde=0x31 ("1" key)
	//Only QWERTZ de-DE layout (tilde on "plus" key): vk_tilde=VK_OEM_PLUS ("plus" key)
	//Only QWERTY it-IT layout (no tilde): vk_tilde=VK_OEM_3 (top-left key)
	DWORD vk_tilde;
	int hkl_len=GetKeyboardLayoutList(0, NULL);
	HKL hkl_lst[hkl_len+1];
	if (hkl_len) GetKeyboardLayoutList(hkl_len, hkl_lst);
	hkl_lst[hkl_len]=GetKeyboardLayout(0);
	while ((vk_tilde=LOBYTE(VkKeyScanEx(L'~', hkl_lst[hkl_len])))==255&&hkl_len--||(vk_tilde=VK_OEM_3, 0));
	
	{
		int lol[]={255, 255, 255};
		int lil=2;
		int kek;
		while ((kek=lol[lil])==255&&(lil--||(kek=666, 0)));
		MessageBox(NULL, std::to_wstring(kek).c_str(), L"TST", MB_OK);
	}
	
	HotkeyEngine::KeyPressFn OnCtrlShiftTld=[&state, vk_tilde](HotkeyEngine* sender, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event){ 
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
		
		if (vk==vk_tilde) {
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
	
	int counter=0;
	
	HotkeyEngine::KeyPressFn OnTld=[&counter, vk_tilde](HotkeyEngine* sender, WPARAM wParam, KBDLLHOOKSTRUCT* kb_event){ 
		bool key_up=wParam==WM_KEYUP||wParam==WM_SYSKEYUP;
		DWORD vk=kb_event->vkCode;
		
		if (!key_up&&vk==vk_tilde) counter++;
		
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
	if (!SnkHotkey->StartNew(std::move(OnTld))) {
		SnkIcon->ChangeIconTooltip(L"SNK_HS: STOPPED");
		SnkIcon->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Start"); 
	}
	
	//Main thread's message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)>0) {	//GetMessage returns -1 if error (probably happens only with invalid input parameters) and 0 if WM_QUIT so continue only on positive result
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	MessageBox(NULL, std::to_wstring(counter).c_str(), L"SNK_HS", MB_OK);
	
	return 0;	//Instead of returning WM_QUIT wParam, always return 0 
}