#include "SuiteAboutDialog.h"
#include "SuiteSettings.h"
#include "Res.h"
#include <typeinfo> 

namespace AboutDialog {
	INT_PTR CALLBACK StaticProc(HWND hwndCtl, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

INT_PTR CALLBACK AboutDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SuiteSettings *settings=(SuiteSettings*)GetWindowLongPtr(hwndDlg, DWLP_USER);

	switch (uMsg) {
		case WM_INITDIALOG:
			{
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				settings=(SuiteSettings*)lParam;
				
				SetDlgItemText(hwndDlg, IDC_EXE_LOC, GetExecutableFileName().c_str());
				SetDlgItemText(hwndDlg, IDC_SNK_LOC, settings->GetSnkPath().c_str());
				if (typeid(*settings).hash_code()==typeid(SuiteSettingsReg).hash_code()) {
					if (!settings->GetStoredLocation().find(L"HKEY_CURRENT_USER"))
						SetDlgItemText(hwndDlg, IDC_CFG_LOC, L"Settings are stored in the registry (per user)");
					else
						SetDlgItemText(hwndDlg, IDC_CFG_LOC, L"Settings are stored in the registry (for all users)");
					EnableWindow(GetDlgItem(hwndDlg, IDC_CFG_OPEN), false);
				} else
					SetDlgItemText(hwndDlg, IDC_CFG_LOC, settings->GetStoredLocation().c_str());
				
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) {
					SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
					SendDlgItemMessage(hwndDlg, IDC_ABOUT_ICON, STM_SETICON, (WPARAM)hIcon, 0);
				}
				
				HWND hwndHlinkCtl=GetDlgItem(hwndDlg, IDC_PROJECT_HOME);
				
				SetProp(hwndHlinkCtl, L"PROP_ORIG_STATIC_PROC", (HANDLE)GetWindowLongPtr(hwndHlinkCtl, GWLP_WNDPROC));
				SetWindowLongPtr(hwndHlinkCtl, GWLP_WNDPROC, (LONG_PTR)StaticProc);
				
				//Resizing hyperlink to text size (so mouse hover will look properly)
				//Also, setting default and underlined fonts for hyperlink
				//Created font should be destroyed before destroying dialog so not to leak resources
				HFONT hFont=(HFONT)SendMessage(hwndHlinkCtl, WM_GETFONT, 0, 0);
				SetProp(hwndHlinkCtl, L"PROP_DEF_FONT", (HANDLE)hFont);
				HDC hdcHlinkCtl=GetDC(hwndHlinkCtl);
				HGDIOBJ hGdiObj=SelectObject(hdcHlinkCtl, hFont);
				wchar_t hlink_text_buf[64];	//Ofcourse text can be longer than that, but here we are hardcoding reasonable limit for hyperlink caption text (cross-mentioned in the .rc)
				if (int hlink_text_len=GetWindowText(hwndHlinkCtl, hlink_text_buf, 64)) {
					SIZE szHlinkCtl;
					GetTextExtentPoint32(hdcHlinkCtl, hlink_text_buf, hlink_text_len, &szHlinkCtl);	//Passing string length to GetTextExtentPoint32 instead of "character count" is not by the book so don't use UTF16 surrogate pairs in hyperlink caption (cross-mentioned in the .rc)
					SetWindowPos(hwndHlinkCtl, hwndDlg, 0, 0, szHlinkCtl.cx, szHlinkCtl.cy, SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
				}
				SelectObject(hdcHlinkCtl, hGdiObj);
				ReleaseDC(hwndHlinkCtl, hdcHlinkCtl);
				LOGFONT logfont;
				SetProp(hwndHlinkCtl, L"PROP_ULINE_FONT", NULL);
				if (GetObject(hFont, sizeof(logfont), &logfont)) {
					logfont.lfUnderline=TRUE;
					if (hFont=CreateFontIndirect(&logfont))
						SetProp(hwndHlinkCtl, L"PROP_ULINE_FONT", (HANDLE)hFont);
				}
				
				//We should set focus to default button (because we are returning FALSE from WM_INITDIALOG) but without bypassing dialog manager: https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_CLOSE_ABOUT), TRUE);
				return FALSE;	//Returning false so not set default focus on edit control
			}
		case WM_CTLCOLORSTATIC:
			if (GetDlgItem(hwndDlg, IDC_PROJECT_HOME)==(HWND)lParam) {
				SetTextColor((HDC)wParam, RGB(0, 0, 192));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
			} else
				return FALSE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
			return TRUE;
		case WM_SYSCOMMAND:
			if (wParam!=SC_CONTEXTHELP)
				return FALSE;
			//If user pressed help button - fall down to WM_HELP
		case WM_HELP:
			//Because SC_CONTEXTHELP is trapped, the only way to receive WM_HELP is by pressing F1
			ShellExecute(NULL, L"open", GetExecutableFileName(L"\\README.TXT").c_str(), NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_CLOSE_ABOUT:
						EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
						return TRUE;
					case IDC_EXE_OPEN:
						ShellExecute(NULL, L"open", GetExecutableFileName(L"").c_str(), NULL, NULL, SW_SHOWNORMAL);
						return TRUE;
					case IDC_SNK_OPEN:
						{
							size_t last_backslash;
							if ((last_backslash=settings->GetSnkPath().find_last_of(L'\\'))!=std::wstring::npos)
								ShellExecute(NULL, L"open", settings->GetSnkPath().substr(0, last_backslash).c_str(), NULL, NULL, SW_SHOWNORMAL);
							return TRUE;
						}
					case IDC_CFG_OPEN:
						ShellExecute(NULL, L"open", settings->GetStoredLocation().c_str(), NULL, NULL, SW_SHOWNORMAL);
						return TRUE;
				}
			}
			if (wParam==IDCANCEL) {	//Handler for ESC
				EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
				return TRUE;
			}
			return FALSE;
		default:
			return FALSE;
	}
}

INT_PTR CALLBACK AboutDialog::StaticProc(HWND hwndCtl, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC orig_proc=(WNDPROC)GetProp(hwndCtl, L"PROP_ORIG_STATIC_PROC");

	switch (uMsg) {
		case WM_DESTROY:
			SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)orig_proc);
			RemoveProp(hwndCtl, L"PROP_ORIG_STATIC_PROC");

			SendMessage(hwndCtl, WM_SETFONT, (WPARAM)GetProp(hwndCtl, L"PROP_DEF_FONT"), FALSE);
			RemoveProp(hwndCtl, L"PROP_DEF_FONT");

			DeleteObject((HFONT)GetProp(hwndCtl, L"PROP_ULINE_FONT"));
			RemoveProp(hwndCtl, L"PROP_ULINE_FONT");
			
			break;
		case WM_MOUSEMOVE:
			if (GetCapture()==hwndCtl) {
				RECT rect;
				POINT pt={LOWORD(lParam), HIWORD(lParam)};
				
				GetWindowRect(hwndCtl, &rect);
				ClientToScreen(hwndCtl, &pt);

				if (!PtInRect(&rect, pt)) {
					ReleaseCapture();
				}
			} else {
				if (HANDLE hFont=GetProp(hwndCtl, L"PROP_ULINE_FONT"))
					SendMessage(hwndCtl, WM_SETFONT, (WPARAM)hFont, FALSE);
				InvalidateRect(hwndCtl, NULL, FALSE);
				SetCapture(hwndCtl);
			}
			break;
		case WM_CAPTURECHANGED:
			SendMessage(hwndCtl, WM_SETFONT, (WPARAM)GetProp(hwndCtl, L"PROP_DEF_FONT"), FALSE);
			InvalidateRect(hwndCtl, NULL, FALSE);
			break;
		case WM_LBUTTONUP:
			ShellExecute(NULL, L"open", L"https://github.com/lcferrum", NULL, NULL, SW_SHOWNORMAL);
			break;
		case WM_SETCURSOR:
			//Since IDC_HAND is not available on all operating systems, we will load the arrow cursor if IDC_HAND is not present
			if (HCURSOR hCursor=LoadCursor(NULL, IDC_HAND))
				SetCursor(hCursor);
			else
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			
			return TRUE;
	}

	return CallWindowProc(orig_proc, hwndCtl, uMsg, wParam, lParam);
}
