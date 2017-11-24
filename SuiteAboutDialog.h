#ifndef SUITEABOUTDIALOG_H
#define SUITEABOUTDIALOG_H

#include "TaskbarNotificationAreaIcon.h"
#include "SuiteSettings.h"
#include <windows.h>

#define AD_DLGPRC_WHATEVER	1

typedef struct {
	TskbrNtfAreaIcon* icon;
	const SuiteSettings* settings;	//Though SuiteSettings is passed to BindingDialogProc it should be used only to query values, not set them
} ABOUT_DLGPRC_PARAM;

namespace AboutDialog {
	INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

#endif //SUITEABOUTDIALOG_H
