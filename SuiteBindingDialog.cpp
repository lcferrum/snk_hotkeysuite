#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteCommon.h"
#include "Res.h"
#include <functional>

INT_PTR CALLBACK BindingDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BINDING_DLGPRC_PARAM *bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)GetWindowLongPtr(hwndDlg, DWLP_USER);
	
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				//It's assumed that bd_dlgprc_param->binded_key.sc is 0 at the init and hk_engine is non-NULL valid pointer
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)lParam;
				
				SetWindowText(hwndDlg, GetHotkeyString(ModKeyType::DONT_CARE, bd_dlgprc_param->settings->GetBindedKey(), HkStrType::VK, L"Rebind ").c_str());
				
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				
				//Created font should be destroyed before destroying dialog so not to leak resources
				LOGFONT logfont;
				if (GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_BD_VIEWER, WM_GETFONT, 0, 0), sizeof(logfont), &logfont)) {
					logfont.lfHeight-=2;
					logfont.lfWeight=FW_BOLD;
					if (bd_dlgprc_param->bold_font=CreateFontIndirect(&logfont))
						SendDlgItemMessage(hwndDlg, IDC_BD_VIEWER, WM_SETFONT, (WPARAM)bd_dlgprc_param->bold_font, FALSE);
				}
				
				//If we fail with starting binding keyboard hook - exit immediately with BD_DLGPRC_ERROR result
				if (!bd_dlgprc_param->hk_engine->StartNew(std::bind(BindKeyEventHandler, hwndDlg, WM_BINDSC, std::placeholders::_1, std::placeholders::_2)))
					EndDialog(hwndDlg, BD_DLGPRC_ERROR);
				
				return TRUE;
			}
		case WM_BINDSC:
			if (!bd_dlgprc_param->binded_key.sc) {
				bd_dlgprc_param->binded_key.tuple=(DWORD)wParam;
				bd_dlgprc_param->hk_engine->Stop();
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM_SC), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RETRY_SC), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_SC), TRUE);
				//We should set focus to default button (it's not set by default because button was disabled) but without bypassing dialog manager: https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_CONFIRM_SC), TRUE);
				SetDlgItemText(hwndDlg, IDC_BD_VIEWER, GetHotkeyString(ModKeyType::DONT_CARE, bd_dlgprc_param->binded_key, HkStrType::VK, L"Rebind to ", 
					GetHotkeyWarning(bd_dlgprc_param->settings->GetModKey(), bd_dlgprc_param->binded_key, L"?\nWarning: may not work with ", L" modifier key!", L"?").c_str()).c_str());
			}
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			bd_dlgprc_param->hk_engine->Stop();
			EndDialog(hwndDlg, BD_DLGPRC_CANCEL);
			return TRUE;
		case WM_DESTROY:
			if (bd_dlgprc_param->bold_font)
				DeleteObject(bd_dlgprc_param->bold_font);
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
						bd_dlgprc_param->binded_key.sc=0;
						SetDlgItemText(hwndDlg, IDC_BD_VIEWER, L"Press any key...\n(Ctrl, Alt and Shift keys are ignored)");
						//If we fail with starting binding keyboard hook - exit immediately with BD_DLGPRC_ERROR result
						if (!bd_dlgprc_param->hk_engine->Start())
							EndDialog(hwndDlg, BD_DLGPRC_ERROR);
						return TRUE;
					case IDC_CONFIRM_SC:
						EndDialog(hwndDlg, BD_DLGPRC_OK);
						return TRUE;
					case IDC_CANCEL_SC:
						EndDialog(hwndDlg, BD_DLGPRC_CANCEL);
						return TRUE;
				}
			}
			if (wParam==IDCANCEL) {
				EndDialog(hwndDlg, BD_DLGPRC_CANCEL);
				return TRUE;
			}
			return FALSE;
		default:
			return FALSE;
	}
}
