#include "SuiteHotkeyFunctions.h"
#include "SuiteBindingDialog.h"
#include "SuiteSettings.h"
#include "SuiteCommon.h"
#include "Res.h"
#include <functional>

INT_PTR CALLBACK BindingDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BINDING_DLGPRC_PARAM *bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)GetWindowLongPtr(hwndDlg, DWLP_USER);
	
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				//It's assumed that bd_dlgprc_param->binded_vk is 0 at the init and hk_engine is non-NULL valid pointer
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				bd_dlgprc_param=(BINDING_DLGPRC_PARAM*)lParam;
				
				SetWindowText(hwndDlg, GetHotkeyString(SuiteSettings::DONT_CARE, bd_dlgprc_param->original_vk, GetHotkeyStringType::VK, L"Rebind ").c_str());
				
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				
				//Created font should be destroyed before destroying dialog so not to leak resources
				HFONT hFont;
				LOGFONT logfont;
				if (GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_VK_VIEWER, WM_GETFONT, 0, 0), sizeof(logfont), &logfont)) {
					logfont.lfHeight-=2;
					logfont.lfWeight=FW_BOLD;
					if (hFont=CreateFontIndirect(&logfont))
						SendDlgItemMessage(hwndDlg, IDC_VK_VIEWER, WM_SETFONT, (WPARAM)hFont, FALSE);
				}
				
				//If we fail with starting binding keyboard hook - exit immediately with -1 result which indicates error
				if (!bd_dlgprc_param->hk_engine->StartNew(std::bind(BindKey, hwndDlg, WM_BINDVK, std::placeholders::_1, std::placeholders::_2))) {
					DeleteObject(hFont);
					EndDialog(hwndDlg, -1);
				}
				
				return TRUE;
			}
		case WM_BINDVK:
			if (!bd_dlgprc_param->binded_vk) {
				bd_dlgprc_param->binded_vk=wParam;
				bd_dlgprc_param->hk_engine->Stop();
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM_VK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RETRY_VK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_VK), TRUE);
				//We should set focus to default button (it's not set by default because button was disabled) but without bypassing dialog manager: https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_CONFIRM_VK), TRUE);
				SetDlgItemText(hwndDlg, IDC_VK_VIEWER, GetHotkeyString(SuiteSettings::DONT_CARE, wParam, GetHotkeyStringType::VK, L"Rebind to ", L"?").c_str());
			}
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			bd_dlgprc_param->hk_engine->Stop();
			DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_VK_VIEWER, WM_GETFONT, 0, 0));
			EndDialog(hwndDlg, 0);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			//They are enabled only when binding hook has already stopped
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_RETRY_VK:
						EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM_VK), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_RETRY_VK), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_VK), FALSE);
						bd_dlgprc_param->binded_vk=0;
						SetDlgItemText(hwndDlg, IDC_VK_VIEWER, L"Press any key...");
						//If we fail with starting binding keyboard hook - exit immediately with -1 result which indicates error
						if (!bd_dlgprc_param->hk_engine->Start()) {
							DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_VK_VIEWER, WM_GETFONT, 0, 0));
							EndDialog(hwndDlg, -1);
						}
						return TRUE;
					case IDC_CONFIRM_VK:
						DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_VK_VIEWER, WM_GETFONT, 0, 0));
						EndDialog(hwndDlg, 1);
						return TRUE;
					case IDC_CANCEL_VK:
						DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_VK_VIEWER, WM_GETFONT, 0, 0));
						EndDialog(hwndDlg, 0);
						return TRUE;
				}
			}
			return FALSE;
		default:
			return FALSE;
	}
}
