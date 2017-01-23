#ifndef SUITEABOUTDIALOG_H
#define SUITEABOUTDIALOG_H

#include <windows.h>

#define AD_DLGPRC_WHATEVER	1

namespace AboutDialog {
	INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

#endif //SUITEABOUTDIALOG_H
