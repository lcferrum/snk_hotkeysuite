#include "SuiteMain.h"
#include "TaskbarNotificationAreaIcon.h"
#include "SuiteExternalRelations.h"
#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteAboutDialog.h"
#include "SuiteExterns.h"
#include "HotkeyEngine.h"
#include "SuiteVersion.h"
#include "SuiteCommon.h"
#include "Res.h"
#include <string>
#include <functional>
#include <initguid.h>
#include <wincodec.h>

extern pTaskDialog fnTaskDialog;
extern pWICConvertBitmapSource fnWICConvertBitmapSource;
extern pSHGetStockIconInfo fnSHGetStockIconInfo;

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, bool elev_req, TskbrNtfAreaIcon *sender, WPARAM wParam, LPARAM lParam);

void CloseEventHandler(SuiteSettings *settings, TskbrNtfAreaIcon *sender);
void EndsessionTrueEventHandler(SuiteSettings *settings, TskbrNtfAreaIcon *sender, bool critical);
bool CheckIfElevationRequired();
HBITMAP GetUacShieldBitmap();

#ifdef __clang__
//Obscure clang++ bug - it reports "multiple definition" of std::operator+() when statically linking with libstdc++
//Observed on LLVM 3.6.2 with MinGW 4.7.2
//This is a fix for the bug
extern template std::wstring std::operator+(wchar_t const*, std::wstring const&);	//caused by use of std::operator+(wchar_t const*, std::wstring const&)
#endif

int SuiteMain(SuiteSettings *settings)
{
	TskbrNtfAreaIcon* SnkIcon=NULL;
	HotkeyEngine* SnkHotkey=NULL;
	
	std::wstring snk_path;
	if (!settings->IsCustomShk()||!settings->IsCustomLhk()) {
		snk_path=settings->GetSnkPath();
		DWORD dwAttrib=GetFileAttributes(snk_path.c_str());
		if (dwAttrib==INVALID_FILE_ATTRIBUTES||(dwAttrib&FILE_ATTRIBUTE_DIRECTORY)) {
			const wchar_t *wrn_msg=L"Path to SnK is not valid!\n\nPlease choose valid SnK path.";
			if (fnTaskDialog) {
				int btn_clicked;
				fnTaskDialog(NULL, NULL, SNK_HS_TITLE, NULL, wrn_msg, TDCBF_OK_BUTTON, TD_WARNING_ICON, &btn_clicked);
			} else {
				MessageBox(NULL, wrn_msg, SNK_HS_TITLE, MB_ICONWARNING|MB_OK);
			}
			if (SuiteExtRel::LaunchSnkOpenDialog(snk_path))
				settings->SetSnkPath(snk_path);
			else {
				ErrorMessage(L"Path to SnK is not valid!");
				return ERR_SUITEMAIN+1;
			}
		}
		snk_path=QuoteArgument(snk_path.c_str())+L" /sec /bpp +p /cmd=";
	}
	std::wstring snk_cmdline_s;
	std::wstring snk_cmdline_l;
	if (settings->IsCustomShk())
		snk_cmdline_s=settings->GetCustomShk();
	else
		snk_cmdline_s=snk_path+QuoteArgument(settings->GetShkCfgPath().c_str());		
	if (settings->IsCustomLhk())
		snk_cmdline_l=settings->GetCustomLhk();
	else
		snk_cmdline_l=snk_path+QuoteArgument(settings->GetLhkCfgPath().c_str());
	
	//CreateProcessW requires lpCommandLine to be non-const string because it may edit it
	//MSDN doesn't provide details on why it may happen, but actually it will occur when lpApplicationName is not provided
	//In this case CreateProcessW will try to tokenize lpCommandLine with NULLs to try to find lpApplicationName in it
	//Interesting thing is that it will revert all changes to lpCommandLine when finished
	//So it just have to be writable and we can assume that contents will stay the same
	//CreateProcessA doesn't have such remark because it's actually a wrapper to CreateProcessW and passes unicode copy of lpCommandLine to CreateProcessW and not the original string
	//wstring::c_str() since C++11 returns pointer to continous NULL-terminated array
	//While it's not advised to edit it (though it won't lead to memory violations as long as string length is considered, which is the case with CreateProcessW) it won't do any harm here
	//Because CreateProcessW preserves it's contents and we won't do string manipulations with this buffer afterwards anyway
	//So it's safe to drop const qualifier
	KeyTriplet OnKeyTriplet(const_cast<wchar_t*>(snk_cmdline_s.c_str()), const_cast<wchar_t*>(snk_cmdline_l.c_str()));
	
	//Check if we elevated rights may be required
	bool elev_req=CheckIfElevationRequired();
	
	//It's ok to pass reference to NULL HotkeyEngine to OnWmCommand - see IconMenuProc comments
	//std::bind differs from lamda captures in that you can't pass references by normal means - object will be copied anyway
	//To pass a reference you should wrap referenced object in std::ref
	SnkIcon=TskbrNtfAreaIcon::MakeInstance(GetModuleHandle(NULL), WM_HSTNAICO, SNK_HS_TITLE L": Running", elev_req?IDI_HSLTNAICO:IDI_HSTNAICO, L"SnK_HotkeySuite_IconClass", IDR_ICONMENU, IDM_STOP_START,
		std::bind(IconMenuProc, std::ref(SnkHotkey), settings, &OnKeyTriplet, elev_req, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(CloseEventHandler, settings, std::placeholders::_1),
		std::bind(EndsessionTrueEventHandler, settings, std::placeholders::_1, std::placeholders::_2));
	if (!SnkIcon->IsValid()) {
		ErrorMessage(L"Failed to create icon!");
		return ERR_SUITEMAIN+2;
	}
	
	//At this point taskbar icon is already visible but unusable - it doesn't respond to any clicks and can't show popup menu
	//So it's ok to customize menu here and initialize everything else
	SnkHotkey=HotkeyEngine::MakeInstance();
	//By default IDM_EDIT_LHK menu item is enabled and IDM_SET_EN_LHK is unchecked (see Res.rc)
	if (settings->GetLongPress()) {
		SnkIcon->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED); 
	} else {
		SnkIcon->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
	}
	//By default none of IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT and IDM_SET_SHIFT_ALT is checked (see Res.rc)
	switch (settings->GetModKey()) {
		case ModKeyType::CTRL_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			break;
		case ModKeyType::SHIFT_ALT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			break;
		case ModKeyType::CTRL_SHIFT:
			SnkIcon->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			break;
	}
	SnkIcon->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(settings->GetBindedKey(), L"Rebind ", L"...").c_str());
	SnkIcon->ModifyIconMenu(POS_SETTINGS, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(SnkIcon->GetIconMenu(), POS_SETTINGS), GetHotkeyString(settings->GetModKey(), settings->GetBindedKey()).c_str());
	HBITMAP uac_bitmap=NULL;
	if (elev_req) {
		if ((uac_bitmap=GetUacShieldBitmap())) {
			MENUINFO mi={sizeof(MENUINFO), MIM_STYLE};
			if (SnkIcon->GetIconMenuInfo(&mi)) {
				mi.dwStyle|=MNS_CHECKORBMP;
				SnkIcon->SetIconMenuInfo(&mi);
			}
			//ModifyMenu doesn't work here - it just replaces text with bitmap
			MENUITEMINFO mii={sizeof(MENUITEMINFO)};
			mii.fMask=MIIM_BITMAP;
			mii.hbmpItem=uac_bitmap;
			SnkIcon->SetIconMenuItemInfo(IDM_ELEVATE, FALSE, &mii);
		}
	} else
		SnkIcon->RemoveIconMenu(IDM_ELEVATE, MF_BYCOMMAND);

	//It is possible to set initial stack commit size for hotkey hook thread
	//By default it is 0 and this means that stack commit size is the same as defined in PE header (that is 4 KB for MinGW/Clang)
	//It was observed (on Win Server 2012 R2 x64 test machine for v1.1) that hotkey hook thread consumes around 2 KB of stack at it's peak usage when not triggered (i.e. no hotkey press actually happens)
	//When hotkey press happens, stack peak usage jumps to around 9 KB (thanks to CreateProcess call)
	//Because most of the time hook thread stays in it's untriggered state, we keep default initial commit size value from PE header (i.e. 4 KB)
	//Commit size will grow automatically if more stack space is needed by the thread
	if (!SnkHotkey->StartNew((LPARAM)&OnKeyTriplet, OnKeyTriplet.CreateEventHandler(settings))) {
		ErrorMessage(L"Failed to set keyboard hook!");
		return ERR_SUITEMAIN+3;
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
	if (uac_bitmap) DeleteObject(uac_bitmap);	//If UAC shield bitmap was created - free it
	
	return msg.wParam;
}

bool IconMenuProc(HotkeyEngine* &hk_engine, SuiteSettings *settings, KeyTriplet *hk_triplet, bool elev_req, TskbrNtfAreaIcon *sender, WPARAM wParam, LPARAM lParam)
{
	//Not checking if hk_engine is NULL in menu handlers - handlers won't be called until message loop is fired which happens after creation of hk_engine
	bool hk_was_running=false;
	switch (LOWORD(wParam)) {
		case IDM_EXIT:
		case IDM_ELEVATE:
			//We can just use PostQuitMessage() and wait for TskbrNtfAreaIcon destructor to destroy icon at the end of the program
			//But in this case icon will be briefly present after the end of message loop till the end of WinMain, though being unresponsive
			//It will be better to destroy the icon right away and then exit message loop
			//And after that do all other uninitialization without icon being visible for unknown purpose
			sender->CloseAndQuit(LOWORD(wParam)==IDM_ELEVATE?ERR_ELEVATE:0);	//Set WM_QUIT's wParam to 0 if normal exit and ERR_ELEVATE if requesting elevated rights
			return true;
		case IDM_STOP_START:
			if (hk_engine->IsRunning()) {
				hk_engine->Stop();
				sender->ChangeIconTooltip(SNK_HS_TITLE L": Stopped");
				sender->ChangeIcon(elev_req?IDI_HSLSTOPICO:IDI_HSSTOPICO);
				sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Start"); 
			} else {
				if (!hk_engine->Start()) break;
				sender->ChangeIconTooltip(SNK_HS_TITLE L": Running");
				sender->ChangeIcon(elev_req?IDI_HSLTNAICO:IDI_HSTNAICO);
				sender->ModifyIconMenu(IDM_STOP_START, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_STOP_START, L"Stop"); 
			}
			return true;
		case IDM_EDIT_SHK:
		case IDM_EDIT_LHK:
			{
				if (LOWORD(wParam)==IDM_EDIT_SHK?settings->IsCustomShk():settings->IsCustomLhk()) {
					bool create_file=false;
					const wchar_t* MSG_TEXT=L"Custom action is set in config.\n\nPlease edit config file instead.";
					if (fnTaskDialog) {
						int btn_clicked;
						fnTaskDialog(NULL, NULL, SNK_HS_TITLE, NULL, MSG_TEXT, TDCBF_OK_BUTTON, TD_INFORMATION_ICON, &btn_clicked);
					} else {
						MessageBox(NULL, MSG_TEXT, SNK_HS_TITLE, MB_ICONASTERISK|MB_OK);
					}
					return true;
				}
				
				std::wstring cfg_path=LOWORD(wParam)==IDM_EDIT_SHK?settings->GetShkCfgPath():settings->GetLhkCfgPath();
				
				DWORD dwAttrib=GetFileAttributes(cfg_path.c_str());
				if (dwAttrib==INVALID_FILE_ATTRIBUTES||(dwAttrib&FILE_ATTRIBUTE_DIRECTORY)) {
					std::wstring msg_text=L"File \""+cfg_path+L"\" doesn't exist.\n\nDo you want it to be created?";
					bool create_file=false;
					if (fnTaskDialog) {
						int btn_clicked;
						fnTaskDialog(NULL, NULL, SNK_HS_TITLE, NULL, msg_text.c_str(), TDCBF_YES_BUTTON|TDCBF_NO_BUTTON, TD_INFORMATION_ICON, &btn_clicked);
						create_file=btn_clicked==IDYES;
					} else {
						create_file=MessageBox(NULL, msg_text.c_str(), SNK_HS_TITLE, MB_ICONASTERISK|MB_YESNO|MB_DEFBUTTON1)==IDYES;
					}
					
					if (create_file&&CreateDirTreeForFile(cfg_path)) {
						CloseHandle(CreateFile(cfg_path.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL));
					} else
						return true;
				}
				
				ShellExecute(NULL, NULL, cfg_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
				return true;
			}
		case IDM_SET_EN_LHK:
			hk_was_running=hk_engine->Stop();
			if (settings->GetLongPress()) {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_UNCHECKED);
				sender->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_GRAYED);
				settings->SetLongPress(false);
			} else {
				sender->CheckIconMenuItem(IDM_SET_EN_LHK, MF_BYCOMMAND|MF_CHECKED);
				sender->EnableIconMenuItem(IDM_EDIT_LHK, MF_BYCOMMAND|MF_ENABLED);
				settings->SetLongPress(true);
			}
			hk_engine->Set((LPARAM)hk_triplet, hk_triplet->CreateEventHandler(settings));
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_ALT, MF_BYCOMMAND);
			settings->SetModKey(ModKeyType::CTRL_ALT);
			sender->ModifyIconMenu(POS_SETTINGS, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), POS_SETTINGS), GetHotkeyString(ModKeyType::CTRL_ALT, settings->GetBindedKey()).c_str()); 
			hk_engine->Set((LPARAM)hk_triplet, hk_triplet->CreateEventHandler(settings));
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_SHIFT_ALT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_SHIFT_ALT, MF_BYCOMMAND);
			settings->SetModKey(ModKeyType::SHIFT_ALT);
			sender->ModifyIconMenu(POS_SETTINGS, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), POS_SETTINGS), GetHotkeyString(ModKeyType::SHIFT_ALT, settings->GetBindedKey()).c_str()); 
			hk_engine->Set((LPARAM)hk_triplet, hk_triplet->CreateEventHandler(settings));
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
		case IDM_SET_CTRL_SHIFT:
			hk_was_running=hk_engine->Stop();
			sender->CheckIconMenuRadioItem(IDM_SET_CTRL_ALT, IDM_SET_CTRL_SHIFT, IDM_SET_CTRL_SHIFT, MF_BYCOMMAND);
			settings->SetModKey(ModKeyType::CTRL_SHIFT);
			sender->ModifyIconMenu(POS_SETTINGS, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), POS_SETTINGS), GetHotkeyString(ModKeyType::CTRL_SHIFT, settings->GetBindedKey()).c_str()); 
			hk_engine->Set((LPARAM)hk_triplet, hk_triplet->CreateEventHandler(settings));
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
				sender->Disable();
				hk_was_running=hk_engine->Stop();
				BINDING_DLGPRC_PARAM bd_dlgprc_param={sender, hk_engine, settings, {0, 0, false}, NULL};
				//Several words on InitCommonControls() and InitCommonControlsEx()
				//"Common" name is somewhat misleading
				//Even though controls like "static", "edit" and "button" are common (like in common sence) they are actually "standard" controls
				//So there is no need to initialize them with InitCommonControls() or InitCommonControlsEx() functions
				//Though ICC_STANDARD_CLASSES can be passed to InitCommonControlsEx, it actually does nothing - "standard" controls are really initialized by the system
				//Also these functions has nothing to do with "Win XP"/"ComCtl32 v6+" style - just supply proper manifest to make "standard" controls use it
				//IDD_BINDINGDLG uses only "standard" controls
				switch (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_BINDINGDLG), sender->GetIconWindow(), BindingDialog::DialogProc, (LPARAM)&bd_dlgprc_param)) {
					case BD_DLGPRC_ERROR:
					case DLGBX_FN_INV_PARAM:
					case DLGBX_FN_FAILED:
						break;
					case BD_DLGPRC_OK:
						settings->SetBindedKey(bd_dlgprc_param.binded_key);
						sender->ModifyIconMenu(IDM_SET_CUSTOM, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED|MF_ENABLED, IDM_SET_CUSTOM, GetHotkeyString(bd_dlgprc_param.binded_key, L"Rebind ", L"...").c_str());
						sender->ModifyIconMenu(POS_SETTINGS, MF_BYPOSITION|MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_POPUP, (UINT_PTR)GetSubMenu(sender->GetIconMenu(), POS_SETTINGS), GetHotkeyString(settings->GetModKey(), bd_dlgprc_param.binded_key).c_str()); 
					case BD_DLGPRC_CANCEL:
						hk_engine->Set((LPARAM)hk_triplet, hk_triplet->CreateEventHandler(settings));
						if (hk_was_running&&!hk_engine->Start()) break;
						sender->SetModalWnd(NULL);
						sender->Enable();
						return true;
				}
				break;
			}
#ifdef DEBUG
		case IDM_DEBUG:
			hk_was_running=hk_engine->Stop();
			if (sender->GetIconMenuState(IDM_DEBUG, MF_BYCOMMAND)&MF_CHECKED) {
				sender->CheckIconMenuItem(IDM_DEBUG, MF_BYCOMMAND|MF_UNCHECKED);
				sender->EnableIconMenuItem(POS_SETTINGS, MF_BYPOSITION|MF_ENABLED);
				hk_engine->Set((LPARAM)hk_triplet, hk_triplet->CreateEventHandler(settings));
			} else {
				sender->CheckIconMenuItem(IDM_DEBUG, MF_BYCOMMAND|MF_CHECKED);
				sender->EnableIconMenuItem(POS_SETTINGS, MF_BYPOSITION|MF_GRAYED);
				hk_engine->Set(0, DebugEventHandler);
			}
			if (hk_was_running&&!hk_engine->Start()) break;
			return true;
#endif
		case IDM_CMDPRMPT:
			SuiteExtRel::LaunchCommandPrompt(GetDirPath(GetFullPathNameWrapper(settings->GetSnkPath())).c_str());
			return true;
		case IDM_ABOUT:
			{
				//Blah blah blah... see comments on IDM_SET_CUSTOM
				sender->Disable();
				hk_was_running=hk_engine->Stop();
				ABOUT_DLGPRC_PARAM ad_dlgprc_param={sender, settings};
				switch (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ABOUTDLG), sender->GetIconWindow(), AboutDialog::DialogProc, (LPARAM)&ad_dlgprc_param)) {
					case DLGBX_FN_INV_PARAM:
					case DLGBX_FN_FAILED:
						break;
					case AD_DLGPRC_WHATEVER:
						if (hk_was_running&&!hk_engine->Start()) break;
						sender->SetModalWnd(NULL);
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

void CloseEventHandler(SuiteSettings *settings, TskbrNtfAreaIcon *sender)
{
	//Settings will be saved after message loop exits
	sender->CloseAndQuit();	//Sets WM_QUIT's wParam to 0
}

void EndsessionTrueEventHandler(SuiteSettings *settings, TskbrNtfAreaIcon *sender, bool critical)
{
	//Session is about to end - there is a chance that process will be terminated right after this event handler exits, abandoning all the code after mesage loop
	//So saving settings now but not exiting app - it will be terminated anyway
	settings->SaveSettings();
}

bool CheckIfElevationRequired()
{
	HANDLE hOwnToken;
	bool required=false;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hOwnToken)) {
		TOKEN_ELEVATION elevation; 
		DWORD ret_len; 
		if (GetTokenInformation(hOwnToken, TokenElevation, &elevation, sizeof(elevation), &ret_len)) {
			required=!elevation.TokenIsElevated;
		}
		CloseHandle(hOwnToken);
	}
	return required;
}

HBITMAP GetUacShieldBitmap()
{
	HBITMAP uac_bitmap=NULL;
	
	if (!fnWICConvertBitmapSource||!fnSHGetStockIconInfo)
		return NULL;
	
	CoInitialize(NULL);

	IWICImagingFactory *pIWICIF;
	if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICIF)))) {
		IWICBitmap *pIWICB;
		IWICBitmapSource *pIWICBS;
		SHSTOCKICONINFO sii={sizeof(sii)};
		//LoadIcon(IDI_SHIELD) doesn't work here because it loads large version of UAC shield icon, same as SHGetStockIconInfo(SHGSI_LARGEICON)
		if (SUCCEEDED(fnSHGetStockIconInfo(SIID_SHIELD, SHGSI_ICON|SHGSI_SMALLICON, &sii))&&SUCCEEDED(pIWICIF->CreateBitmapFromHICON(sii.hIcon, &pIWICB))) {
			if (SUCCEEDED(fnWICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, pIWICB, &pIWICBS))) {
				UINT cx, cy;
				if (SUCCEEDED(pIWICBS->GetSize(&cx, &cy))) {
					if (HDC hdc=GetDC(NULL)) {
						void *dib_buf;	//Memory area pointed to by this variable will be freed when HBITMAP, returned by CreateDIBSection(hSection=NULL), will be freed with DeleteObject
						BITMAPINFO bmi={{
							sizeof(BITMAPINFOHEADER),	//BITMAPINFO.BITMAPINFOHEADER.biSize
							(int)cx,					//BITMAPINFO.BITMAPINFOHEADER.biWidth
							-(int)cy,					//BITMAPINFO.BITMAPINFOHEADER.biHeight
							1,							//BITMAPINFO.BITMAPINFOHEADER.biPlanes
							32,							//BITMAPINFO.BITMAPINFOHEADER.biBitCount
							BI_RGB						//BITMAPINFO.BITMAPINFOHEADER.biCompression (rest of BITMAPINFO.BITMAPINFOHEADER members are initialized with NULLs)
						}};								//BITMAPINFO.RGBQUAD members are initialized with NULLs (this is required when biBitCount is 32 and biCompression is BI_RGB)
						if ((uac_bitmap=CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &dib_buf, NULL, 0))) {
							UINT stride=cx*sizeof(DWORD);
							UINT buf_sz=cy*stride;
							if (FAILED(pIWICBS->CopyPixels(NULL, stride, buf_sz, (BYTE*)dib_buf))) {
								DeleteObject(uac_bitmap);
								uac_bitmap=NULL;
							}
						}
						ReleaseDC(NULL, hdc);
					}
				}
				pIWICBS->Release();
			}
			pIWICB->Release();
		}
		pIWICIF->Release();
	}

	CoUninitialize();
	
	return uac_bitmap;
}
