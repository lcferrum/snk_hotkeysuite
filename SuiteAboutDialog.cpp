#include "SuiteAboutDialog.h"
#include "Res.h"

namespace AboutDialog {
	BOOL EndDialogWithDeinit(HWND hDlg, INT_PTR nResult, HFONT hFont=NULL);
}

BOOL AboutDialog::EndDialogWithDeinit(HWND hDlg, INT_PTR nResult, HFONT hFont)
{
	//Perform all the deinitilization of initialized in WM_INITDIALOG things here
	
	if (hFont)
		DeleteObject(hFont);
	else
		DeleteObject((HFONT)SendDlgItemMessage(hDlg, IDC_BD_VIEWER, WM_GETFONT, 0, 0));
	
	return EndDialog(hDlg, nResult);
}

INT_PTR CALLBACK AboutDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG:
			return TRUE;
		case WM_CLOSE:
			//Even if dialog doesn't have close (X) button, this message is still received on Alt+F4
			EndDialogWithDeinit(hwndDlg, AD_DLGPRC_WHATEVER);
			return TRUE;
		case WM_COMMAND:
			//Handler for dialog controls
			if (HIWORD(wParam)==BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDC_CLOSE_ABOUT:
						EndDialogWithDeinit(hwndDlg, AD_DLGPRC_WHATEVER);
						return TRUE;
				}
			}
			return FALSE;
		default:
			return FALSE;
	}
}
