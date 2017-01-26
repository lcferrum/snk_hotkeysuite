#include "SuiteAboutDialog.h"
#include "Res.h"

namespace AboutDialog {
	INT_PTR CALLBACK StaticProc(HWND hwndCtl, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

INT_PTR CALLBACK AboutDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) {
					SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
					SendDlgItemMessage(hwndDlg, IDC_ABOUT_ICON, STM_SETICON, (WPARAM)hIcon, 0);
				}
				
				HWND hwndCtl=GetDlgItem(hwndDlg, IDC_PROJECT_HOME);
				
				SetProp(hwndCtl, L"PROP_ORIG_STATIC_PROC", (HANDLE)GetWindowLongPtr(hwndCtl, GWLP_WNDPROC));
				SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)StaticProc);
				
				//Created font should be destroyed before destroying dialog so not to leak resources
				HFONT hFont=(HFONT)SendMessage(hwndCtl, WM_GETFONT, 0, 0);
				LOGFONT logfont;
				SetProp(hwndCtl, L"PROP_DEF_FONT", (HANDLE)hFont);
				SetProp(hwndCtl, L"PROP_ULINE_FONT", NULL);
				if (GetObject(hFont, sizeof(logfont), &logfont)) {
					logfont.lfUnderline=TRUE;
					if (hFont=CreateFontIndirect(&logfont))
						SetProp(hwndCtl, L"PROP_ULINE_FONT", (HANDLE)hFont);
				}
				
				return TRUE;
			}
		case WM_CTLCOLORSTATIC:
			if (GetDlgItem(hwndDlg, IDC_PROJECT_HOME)==(HWND)lParam) {
				SetTextColor((HDC)wParam, RGB(0, 0, 192));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)GetSysColorBrush(COLOR_MENU);
			} else
				return FALSE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_CLOSE_ABOUT:
						EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
						return TRUE;
					case IDC_PROJECT_HOME:
						ShellExecute(NULL, L"open", L"https://github.com/lcferrum", NULL, NULL, SW_SHOWNORMAL);
						return TRUE;
				}
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
			if (GetCapture()!=hwndCtl) {
				if (HANDLE hFont=GetProp(hwndCtl, L"PROP_ULINE_FONT"))
					SendMessage(hwndCtl, WM_SETFONT, (WPARAM)hFont, FALSE);
				InvalidateRect(hwndCtl, NULL, FALSE);
				SetCapture(hwndCtl);
			} else {
				RECT rect;
				POINT pt={LOWORD(lParam), HIWORD(lParam)};
				
				GetWindowRect(hwndCtl, &rect);
				ClientToScreen(hwndCtl, &pt);

				if (!PtInRect(&rect, pt)) {
					SendMessage(hwndCtl, WM_SETFONT, (WPARAM)GetProp(hwndCtl, L"PROP_DEF_FONT"), FALSE);
					InvalidateRect(hwndCtl, NULL, FALSE);
					ReleaseCapture();
				}
			}
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
