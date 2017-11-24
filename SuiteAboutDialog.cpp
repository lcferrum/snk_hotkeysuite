#include "SuiteAboutDialog.h"
#include "Res.h"
#include <typeinfo>

namespace AboutDialog {
	INT_PTR CALLBACK HyperlinkProc(HWND hwndCtl, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

INT_PTR CALLBACK AboutDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ABOUT_DLGPRC_PARAM *ad_dlgprc_param=(ABOUT_DLGPRC_PARAM*)GetWindowLongPtr(hwndDlg, DWLP_USER);

	//If DialogProc returns FALSE then message is passed to dialog box default window procedure
	//DialogProc return result is nor message return value - message return value should be set explicitly with SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, LONG_PTR)
	//By default if DialogProc returns TRUE and doesn't use SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, LONG_PTR) then message return value is 0
	//Exceptions are WM_INITDIALOG and WM_CTLCOLORSTATIC which return value is DialogProc's return result
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
				ad_dlgprc_param=(ABOUT_DLGPRC_PARAM*)lParam;
				
				//Set current window as fallback window for TskbrNtfAreaIcon
				ad_dlgprc_param->icon->SetModalWnd(hwndDlg);
				
				SetDlgItemText(hwndDlg, IDC_EXE_LOC, GetExecutableFileName().c_str());
				SetDlgItemText(hwndDlg, IDC_SNK_LOC, ad_dlgprc_param->settings->GetSnkPath().c_str());
				SetDlgItemText(hwndDlg, IDC_CFG_LOC, ad_dlgprc_param->settings->GetStoredLocation().c_str());
				
				//Using LR_SHARED to not bother with destroying icon when dialog is destroyed
				HICON hIcon=(HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSTNAICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_SHARED);
				if (hIcon) {
					SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
					SendDlgItemMessage(hwndDlg, IDC_ABOUT_ICON, STM_SETICON, (WPARAM)hIcon, 0);
				}
				
				// ------ HYPERLINK INITIALIZATION START -------
				HWND hwndHlinkCtl=GetDlgItem(hwndDlg, IDC_PROJECT_HOME);
				
				//Sublassing Static control
				SetProp(hwndHlinkCtl, L"PROP_ORIG_STATIC_PROC", (HANDLE)GetWindowLongPtr(hwndHlinkCtl, GWLP_WNDPROC));
				SetWindowLongPtr(hwndHlinkCtl, GWLP_WNDPROC, (LONG_PTR)HyperlinkProc);
				
				//Resizing hyperlink to text size (so mouse hover will look properly)
				//Also, setting default and underlined fonts for hyperlink
				//Created font should be destroyed before destroying dialog so not to leak resources (done in hyperlink subclass)
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
					if ((hFont=CreateFontIndirect(&logfont)))
						SetProp(hwndHlinkCtl, L"PROP_ULINE_FONT", (HANDLE)hFont);
				}
				// ------ HYPERLINK INITIALIZATION END -------
				
				//We should set focus to default button (because we are returning FALSE from WM_INITDIALOG) but without bypassing dialog manager: https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_CLOSE_ABOUT), TRUE);
				return FALSE;	//Returning false so not to set default focus on edit control
			}
		case WM_CTLCOLORSTATIC:
			//Hyperlink related code - setting hyperlink text color
			if (GetDlgItem(hwndDlg, IDC_PROJECT_HOME)==(HWND)lParam) {
				//If COLOR_HOTLIGHT is not available - use default blue color
				if (GetSysColorBrush(COLOR_HOTLIGHT))
					SetTextColor((HDC)wParam, GetSysColor(COLOR_HOTLIGHT));
				else
					SetTextColor((HDC)wParam, RGB(0, 0, 255));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
			} else
				return FALSE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
			return TRUE;
		case WM_SYSCOMMAND:
			//Dialog doesn't have menu so trap SC_KEYMENU
			//This will prevent losing focus on pressing Alt key but won't disable keyboard accelerators
			if ((wParam&0xFFF0)==SC_KEYMENU)
				return TRUE;
			else
				return FALSE;
		case WM_HELP:
			//Received on F1
			ShellExecute(NULL, L"open", GetExecutableFileName(L"\\README.TXT").c_str(), NULL, NULL, SW_SHOWNORMAL);
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_EXE_OPEN:
						ShellExecute(NULL, L"open", GetExecutableFileName(L"").c_str(), NULL, NULL, SW_SHOWNORMAL);
						return TRUE;
					case IDC_CFG_OPEN:
					case IDC_SNK_OPEN:
						{
							//SnK path can be relative
							ShellExecute(NULL, L"open", GetDirPath(LOWORD(wParam)==IDC_SNK_OPEN?GetFullPathNameWrapper(ad_dlgprc_param->settings->GetSnkPath()):ad_dlgprc_param->settings->GetStoredLocation()).c_str(), NULL, NULL, SW_SHOWNORMAL);
							
							return TRUE;
						}
					case IDC_CLOSE_ABOUT:
						EndDialog(hwndDlg, AD_DLGPRC_WHATEVER);
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

INT_PTR CALLBACK AboutDialog::HyperlinkProc(HWND hwndCtl, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//This is rather simple substitution for SysLink control from ComCtl32 v6+
	//We are subclassing Static control to make it look and behave like some kind of hyperlink control
	//This is mainly done with WM_CANCELMODE, WM_CAPTURECHANGED, WM_MOUSEMOVE and WM_LBUTTONDOWN messages to make underline on mouse hover
	//We are using mouse capture to monitor when mouse leaves Static control
	//Link is opened on WM_LBUTTONUP, hyperlink text is set in .rc
	//Also used with this subclass are WM_CTLCOLORSTATIC and WM_INITDIALOG (hyperlink initialization section) parent dialog's message handlers
	//Static control should have SS_NOTIFY style set for this subclass to work
	
	WNDPROC orig_proc=(WNDPROC)GetProp(hwndCtl, L"PROP_ORIG_STATIC_PROC");

	switch (uMsg) {
		case WM_DESTROY:
			SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)orig_proc);
			RemoveProp(hwndCtl, L"PROP_ORIG_STATIC_PROC");

			SendMessage(hwndCtl, WM_SETFONT, (WPARAM)GetProp(hwndCtl, L"PROP_DEF_FONT"), FALSE);
			RemoveProp(hwndCtl, L"PROP_DEF_FONT");

			if (HANDLE hFont=GetProp(hwndCtl, L"PROP_ULINE_FONT")) {
				DeleteObject(hFont);
				RemoveProp(hwndCtl, L"PROP_ULINE_FONT");
			}
			
			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
			if (GetCapture()==hwndCtl) {
				RECT rect;
				//Coordinates are signed shorts, while HIWORD/LOWORD returns them as unsigned
				POINT pt={(short int)LOWORD(lParam), (short int)HIWORD(lParam)};
				
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
		case WM_CANCELMODE:
		case WM_CAPTURECHANGED:
			SendMessage(hwndCtl, WM_SETFONT, (WPARAM)GetProp(hwndCtl, L"PROP_DEF_FONT"), FALSE);
			InvalidateRect(hwndCtl, NULL, FALSE);
			//Returning FALSE here for WM_CANCELMODE so not to trigger it's behaviour which is to call ReleaseCapture - we don't need it here
			//No difference for WM_CAPTURECHANGED - no default behaviour for it
			return FALSE;
		case WM_LBUTTONUP:
			ShellExecute(NULL, L"open", L"https://github.com/lcferrum/snk_hotkeysuite", NULL, NULL, SW_SHOWNORMAL);
			break;
		case WM_SETCURSOR:
#ifdef _WIN64
			SetCursor(LoadCursor(NULL, IDC_HAND));
#else
			//If IDC_HAND is not available - load custom hand cursor
			if (HCURSOR hCursor=LoadCursor(NULL, IDC_HAND))
				SetCursor(hCursor);
			else
				SetCursor(LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HANDCUR)));
#endif
			//Don't need anyone else to change cursor here so returning TRUE
			return TRUE;
	}

	return CallWindowProc(orig_proc, hwndCtl, uMsg, wParam, lParam);
}
