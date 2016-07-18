#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteSettings.h"
#include "SuiteCommon.h"
#include "Res.h"
#include <functional>

BOOL EndDialogWithDeinit(HWND hDlg, INT_PTR nResult, HFONT hFont=NULL)
{
	//Perform all the deinitilization of initialized in WM_INITDIALOG things here
	
	if (hFont)
		DeleteObject(hFont);
	else
		DeleteObject((HFONT)SendDlgItemMessage(hDlg, IDC_BD_VIEWER, WM_GETFONT, 0, 0));
	
	return EndDialog(hDlg, nResult);
}

INT_PTR CALLBACK BindingDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BINDING_DLGPRC_PARAM *bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)GetWindowLongPtr(hwndDlg, DWLP_USER);
	
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				//It's assumed that bd_dlgprc_param->binded_vk is 0 at the init and hk_engine is non-NULL valid pointer
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)lParam;
				
				SetWindowText(hwndDlg, GetHotkeyString(ModKeyType::DONT_CARE, bd_dlgprc_param->original_vk, bd_dlgprc_param->original_sc, HkStrType::VK, L"Rebind ").c_str());
				
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				
				//Created font should be destroyed before destroying dialog so not to leak resources
				HFONT hFont;
				LOGFONT logfont;
				if (GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_BD_VIEWER, WM_GETFONT, 0, 0), sizeof(logfont), &logfont)) {
					logfont.lfHeight-=2;
					logfont.lfWeight=FW_BOLD;
					if (hFont=CreateFontIndirect(&logfont))
						SendDlgItemMessage(hwndDlg, IDC_BD_VIEWER, WM_SETFONT, (WPARAM)hFont, FALSE);
				}
				
				//If we fail with starting binding keyboard hook - exit immediately with -1 result which indicates error
				if (!bd_dlgprc_param->hk_engine->StartNew(std::bind(BindKey, hwndDlg, WM_BINDSC, std::placeholders::_1, std::placeholders::_2)))
					EndDialogWithDeinit(hwndDlg, BD_DLGPRC_ERROR, hFont);
				
				return TRUE;
			}
		case WM_BINDSC:
			if (!bd_dlgprc_param->binded_sc) {
				bd_dlgprc_param->binded_vk=wParam;
				bd_dlgprc_param->binded_sc=lParam;
				bd_dlgprc_param->hk_engine->Stop();
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM_SC), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RETRY_SC), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_SC), TRUE);
				//We should set focus to default button (it's not set by default because button was disabled) but without bypassing dialog manager: https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_CONFIRM_SC), TRUE);
				SetDlgItemText(hwndDlg, IDC_BD_VIEWER, GetHotkeyString(ModKeyType::DONT_CARE, bd_dlgprc_param->binded_vk, bd_dlgprc_param->binded_sc, HkStrType::VK, L"Rebind to ", L"?").c_str());
			}
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			bd_dlgprc_param->hk_engine->Stop();
			EndDialogWithDeinit(hwndDlg, BD_DLGPRC_CANCEL);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			//They are enabled only when binding hook has already stopped
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_RETRY_SC:
						EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM_SC), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_RETRY_SC), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_SC), FALSE);
						bd_dlgprc_param->binded_sc=0;
						SetDlgItemText(hwndDlg, IDC_BD_VIEWER, L"Press any key...");
						//If we fail with starting binding keyboard hook - exit immediately with -1 result which indicates error
						if (!bd_dlgprc_param->hk_engine->Start())
							EndDialogWithDeinit(hwndDlg, BD_DLGPRC_ERROR);
						return TRUE;
					case IDC_CONFIRM_SC:
						EndDialogWithDeinit(hwndDlg, BD_DLGPRC_OK);
						return TRUE;
					case IDC_CANCEL_SC:
						EndDialogWithDeinit(hwndDlg, BD_DLGPRC_CANCEL);
						return TRUE;
				}
			}
			return FALSE;
		default:
			return FALSE;
	}
}
