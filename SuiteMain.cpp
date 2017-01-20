#include "SuiteMain.h"
#include "TaskbarNotificationAreaIcon.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteExterns.h"
#include "HotkeyEngine.h"
#include "SuiteCommon.h"
#include "Res.h"
#include <string>
#include <functional>

extern pTaskDialog fnTaskDialog;

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam);

void HotkeyEventHandler(SuiteSettings *settings, bool long_press);

int SuiteMain(HINSTANCE hInstance, SuiteSettings *settings)
{
	TskbrNtfAreaIcon* SnkIcon=NULL;
	HotkeyEngine* SnkHotkey=NULL;
	KeyTriplet OnKeyTriplet;
	
	//It's ok to pass reference to NULL HotkeyEngine to OnWmCommand - see IconMenuProc comments
	//std::bind differs from lamda captures in that you can't pass references by normal means - object will be copied anyway
	//To pass a reference you should wrap referenced object in std::ref
	SnkIcon=TskbrNtfAreaIcon::MakeInstance(hInstance, WM_HSTNAICO, SNK_HS_TITLE L": Running", IDI_HSTNAICO, L"SnK_HotkeySuite_IconClass", IDR_ICONMENU, IDM_STOP_START, 
		std::bind(IconMenuProc, std::ref(SnkHotkey), settings, &OnKeyTriplet, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	if (!SnkIcon->IsValid()) {
		ErrorMessage(L"Failed to create icon!");
		return ERR_SUITEMAIN+1;
	}
	
	//At this point taskbar icon is already visible but unusable - it doesn't respond to any clicks and can't show popup menu
	//So it's ok to customize menu here and initialize everything else
	SnkHotkey=HotkeyEngine::MakeInstance(hInstance);
	//By default IDM_EDIT_LHK menu item is enabled and IDM_SET_EN_LHK is unchecked (see Res.rc)
	if (settings->GetLongPress()) {
		SnkIcon->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED); 
		OnKeyTriplet.SetLongPress(true);
	} else {
		SnkIcon->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
		OnKeyTriplet.SetLongPress(false);
	}
	//By default none of IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT and IDM_SET_SHIFT_ALT is checked (see Res.rc)
	switch (settings->GetModKey()) {
		case ModKeyType::CTRL_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			OnKeyTriplet.SetCtrlAlt();
			break;
		case ModKeyType::SHIFT_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			OnKeyTriplet.SetShiftAlt();
			break;
		case ModKeyType::CTRL_SHIFT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			OnKeyTriplet.SetCtrlShift();
			break;
	}
	SnkIcon->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(ModKeyType::DONT_CARE, settings->GetBindedKey(), HkStrType::VK, L"Rebind ", L"...").c_str());
	//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
	SnkIcon->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(SnkIcon->GetIconMenu(), 2), GetHotkeyString(settings->GetModKey(), settings->GetBindedKey(), HkStrType::FULL).c_str()); 
	OnKeyTriplet.SetBindedKey(settings->GetBindedKey());
	OnKeyTriplet.SetOnShortHotkey(std::bind(&HotkeyEventHandler, settings, false));
	OnKeyTriplet.SetOnLongHotkey(std::bind(&HotkeyEventHandler, settings, true));
	//OnKeyTriplet can be passed as reference (wrapped in std::ref) or as pointer
	if (!SnkHotkey->StartNew(std::bind(&KeyTriplet::KeyPressEventHandler, std::ref(OnKeyTriplet), std::placeholders::_1, std::placeholders::_2))) {
		ErrorMessage(L"Failed to set keyboard hook!");
		return ERR_SUITEMAIN+2;
	}
	
	//Main thread's message loop
	//GetMessage returns -1 if error (probably happens only with invalid input parameters) and 0 if WM_QUIT 
	//So continue on any non-zero result skipping errors
	MSG msg;
	BOOL res;
	while ((res=GetMessage(&msg, NULL, 0, 0))) {
		if (res>0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//Manually uninitializing some components to make sure right unintialization order
	SnkHotkey->Stop();			//This way HotkeyEngine is deinitialized right after TskbrNtfAreaIcon
	settings->SaveSettings();	//Main parts of HotkeySuite are deinitialized and now it's time to save settings
	
	return msg.wParam;
}

void HotkeyEventHandler(SuiteSettings *settings, bool long_press) {
	std::wstring snk_cmdline=L"\""+settings->GetSnkPath()+L"\" +mb /pid="+std::to_wstring(GetCurrentProcessId())+L" -mb /cmd=\"";
	if (long_press)
		snk_cmdline+=settings->GetLhkCfgPath();
	else
		snk_cmdline+=settings->GetShkCfgPath();
	snk_cmdline+=L"\"";
	wchar_t cmdline_buf[snk_cmdline.length()+1];
	wcscpy(cmdline_buf, snk_cmdline.c_str());
	STARTUPINFO si={sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi={};
	if (CreateProcess(NULL, cmdline_buf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
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
			sender->CloseAndQuit();	//Sets WM_QUIT's wParam to 0
			return true;
		case IDM_STOP_START:
			if (hk_engine->IsRunning()) {
				hk_engine->Stop();
				sender->ChangeIconTooltip(SNK_HS_TITLE L": Stopped");
				sender->ChangeIcon(IDI_HSSTOPICO);
				sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Start"); 
			} else {
				if (!hk_engine->Start()) break;
				sender->ChangeIconTooltip(SNK_HS_TITLE L": Running");
				sender->ChangeIcon(IDI_HSTNAICO);
				sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Stop"); 
			}
			return true;
		case IDM_EDIT_SHK:
		case IDM_EDIT_LHK:
			{
				std::wstring (SuiteSettings::*fnGetCfgPath)()=LOWORD(wParam)==IDM_EDIT_SHK?&settings->GetShkCfgPath:&settings->GetLhkCfgPath;
				
				DWORD dwAttrib=GetFileAttributes((settings->*fnGetCfgPath)().c_str());
				if (dwAttrib==INVALID_FILE_ATTRIBUTES||(dwAttrib&FILE_ATTRIBUTE_DIRECTORY)) {
					std::wstring msg_text=L"File \""+(settings->*fnGetCfgPath)()+L"\" doesn't exist.\n\nDo you want it to be created?";
					bool create_file=false;
					if (fnTaskDialog) {
						int btn_clicked;
						fnTaskDialog(NULL, NULL, SNK_HS_TITLE, NULL, msg_text.c_str(), TDCBF_YES_BUTTON|TDCBF_NO_BUTTON, TD_INFORMATION_ICON, &btn_clicked);
						create_file=btn_clicked==IDYES;
					} else {
						create_file=MessageBox(NULL, msg_text.c_str(), SNK_HS_TITLE, MB_ICONASTERISK|MB_YESNO|MB_DEFBUTTON1)==IDYES;
					}
					
					if (create_file&&CreateDirTree((settings->*fnGetCfgPath)())) {
						CloseHandle(CreateFile((settings->*fnGetCfgPath)().c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL));
					} else
						return true;
				}
				
				ShellExecute(NULL, NULL, (settings->*fnGetCfgPath)().c_str(), NULL, NULL, SW_SHOWNORMAL);
				return true;
			}
		case IDM_SET_EN_LHK:
			hk_was_running=hk_engine->Stop();
			if (settings->GetLongPress()) {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_UNCHECKED);
				sender->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
				settings->SetLongPress(false);
				hk_triplet->SetLongPress(false);
			} else {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED);
				sender->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_ENABLED);
				settings->SetLongPress(true);
				hk_triplet->SetLongPress(true);
			}
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			settings->SetModKey(ModKeyType::CTRL_ALT);
			hk_triplet->SetCtrlAlt();
			//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
			sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(ModKeyType::CTRL_ALT, settings->GetBindedKey(), HkStrType::FULL).c_str()); 
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_SHIFT_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			settings->SetModKey(ModKeyType::SHIFT_ALT);
			hk_triplet->SetShiftAlt();
			//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
			sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(ModKeyType::SHIFT_ALT, settings->GetBindedKey(), HkStrType::FULL).c_str()); 
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_SHIFT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			settings->SetModKey(ModKeyType::CTRL_SHIFT);
			hk_triplet->SetCtrlShift();
			//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
			sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(ModKeyType::CTRL_SHIFT, settings->GetBindedKey(), HkStrType::FULL).c_str()); 
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CUSTOM:
			{
				//The fun thing with DialogBoxParam is that it won't return until DLGPROC exits with EndDialog but in the same time it won't block thread execution
				//It creates new message loop within current message loop, while latter remains blocked because current message dispatch hasn't ended (it waits for DialogBoxParam to return)
				//But other windows still respond because this new message loop besides it's own DLGPROC happily handles all other thread's window processes
				//When EndDialog is called within DLGPROC, it exits DialogBoxParam's message loop, finishes message dispatch that called DialogBoxParam in the first place and returns to thread's original message loop
				//Q: What happens when DialogBoxParam called several times without exiting previous dialogs?
				//A: We get several nested message loops and all dialog/window processes will be handled by the last created loop
				//Q: What happens if I use PostQuitMessage with one or several dialogs running?
				//A: All present message loops will sequentially exit (including thread's own, starting from the last created) and DialogBoxParam will return 0 every time it's message loop exits
				//Q: What happens when I open several dialogs and exit the one that wasn't the last created?
				//A: DialogBoxParam won't return and last created message loop will still run until it's own dialog exits
				//A: In the end, randomly closing dialogs, we get proper nested message loop unwinding (i.e. DialogBoxParam will return sequentially starting from the last called)
				//All in all, it's better disable icon completely so binding dialog won't be called second time and no other menu items can be clicked
				sender->Enable(false);
				hk_was_running=hk_engine->Stop();
				BINDING_DLGPRC_PARAM bd_dlgprc_param={hk_engine, settings, {0, 0, false}};
				//Several words on InitCommonControls() and InitCommonControlsEx()
				//"Common" name is somewhat misleading
				//Even though controls like "static", "edit" and "button" are common (like in common sence) they are actually "standard" controls
				//So there is no need to initialize them with InitCommonControls() or InitCommonControlsEx() functions
				//Though ICC_STANDARD_CLASSES can be passed to InitCommonControlsEx, it actually does nothing - "standard" controls are really initialized by the system
				//Also these functions has nothing to do with "Win XP"/"ComCtl32 v6+" style - just supply proper manifest to make "standard" controls use it
				//IDD_BINDINGDLG uses only "standard" controls
				switch (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_BINDINGDLG), sender->GetIconWindow(), BindingDialogProc, (LPARAM)&bd_dlgprc_param)) {
					case BD_DLGPRC_ERROR:
					case DLGBX_FN_INV_PARAM:
					case DLGBX_FN_FAILED:
						break;
					case BD_DLGPRC_OK:
						hk_triplet->SetBindedKey(bd_dlgprc_param.binded_key);
						settings->SetBindedKey(bd_dlgprc_param.binded_key);
						sender->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(ModKeyType::DONT_CARE, bd_dlgprc_param.binded_key, HkStrType::VK, L"Rebind ", L"...").c_str());
						//Warning: POPUP menu item modified by position, so every time menu in Res.rc is changed next line should be modified accordingly
						sender->ModifyIconMenu(2, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), 2), GetHotkeyString(settings->GetModKey(), bd_dlgprc_param.binded_key, HkStrType::FULL).c_str()); 
					case BD_DLGPRC_CANCEL:
						if (hk_was_running&&!hk_engine->StartNew(std::bind(&KeyTriplet::KeyPressEventHandler, hk_triplet, std::placeholders::_1, std::placeholders::_2))) break;
						sender->Enable();
						return true;
				}
				break;
			}
		default:
			return false;
	}
	
	//We get there after break which happens instead of return in all cases where hk_engine should have restarted but failed
	ErrorMessage(L"Failed to restart keyboard hook!");
	sender->CloseAndQuit(ERR_SUITEMAIN+3);	//Sets WM_QUIT's wParam to ERR_SUITEMAIN+3
	return true;
}

