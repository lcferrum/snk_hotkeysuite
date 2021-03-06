#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteCommon.h"
#include "Res.h"

INT_PTR CALLBACK BindingDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BINDING_DLGPRC_PARAM *bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)GetWindowLongPtr(hwndDlg, DWLP_USER);

	//If DialogProc returns FALSE then message is passed to dialog box default window procedure
	//DialogProc return result is nor message return value - message return value should be set explicitly with SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, LONG_PTR)
	//By default if DialogProc returns TRUE and doesn't use SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, LONG_PTR) then message return value is 0
	//Exception is WM_INITDIALOG which return value is DialogProc's return result
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				//It's assumed that bd_dlgprc_param->binded_key.sc is 0 at the init and hk_engine is non-NULL valid pointer
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)lParam;
				
				//Set current window as fallback window for TskbrNtfAreaIcon
				bd_dlgprc_param->icon->SetModalWnd(hwndDlg);
				
				SetWindowText(hwndDlg, GetHotkeyString(bd_dlgprc_param->settings->GetBindedKey(), L"Rebind ").c_str());
				
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				
				//Created font should be destroyed before destroying dialog so not to leak resources
				LOGFONT logfont;
				if (GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_BD_VIEWER, WM_GETFONT, 0, 0), sizeof(logfont), &logfont)) {
					logfont.lfHeight-=2;
					logfont.lfWeight=FW_BOLD;
					if ((bd_dlgprc_param->bold_font=CreateFontIndirect(&logfont)))
						SendDlgItemMessage(hwndDlg, IDC_BD_VIEWER, WM_SETFONT, (WPARAM)bd_dlgprc_param->bold_font, FALSE);
				}
				
				//If we fail with starting binding keyboard hook - exit immediately with BD_DLGPRC_ERROR result
				if (!bd_dlgprc_param->hk_engine->StartNew((LPARAM)hwndDlg, BindKeyEventHandler))
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
				SetDlgItemText(hwndDlg, IDC_BD_VIEWER, GetHotkeyString(bd_dlgprc_param->binded_key, L"Rebind to ", 
					GetHotkeyWarning(bd_dlgprc_param->settings->GetModKey(), bd_dlgprc_param->binded_key, L"?\nWarning: may not work with ", L" modifier key!", L"?").c_str()).c_str());
			}
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			bd_dlgprc_param->hk_engine->Stop();
			EndDialog(hwndDlg, BD_DLGPRC_CANCEL);
			return TRUE;
		case WM_SYSCOMMAND:
			//Dialog doesn't have menu so trap SC_KEYMENU
			//This will prevent losing focus on pressing Alt key but won't disable keyboard accelerators
			if ((wParam&0xFFF0)==SC_KEYMENU)
				return TRUE;
			else
				return FALSE;
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
