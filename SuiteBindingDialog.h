#ifndef SUITEBINDINGDIALOG_H
#define SUITEBINDINGDIALOG_H

#include "TaskbarNotificationAreaIcon.h"
#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include "SuiteCommon.h"
#include <windows.h>

#define BD_DLGPRC_OK						1
#define BD_DLGPRC_CANCEL					2
#define BD_DLGPRC_ERROR						3

typedef struct {
	TskbrNtfAreaIcon* icon;
	HotkeyEngine* hk_engine;
	const SuiteSettings* settings;	//Though SuiteSettings is passed to BindingDialogProc it should be used only to query values, not set them
	BINDED_KEY binded_key;			//While only scancodes are compared in hotkey handler because of their layout independence, vks are used to display key name in UI; non-zero BINDED_KEY.sc indicates that there was binding attempt (key was pressed)
	HFONT bold_font;				//Stores created bold font so as not to use window properties
} BINDING_DLGPRC_PARAM;

namespace BindingDialog {
	INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

#endif //SUITEBINDINGDIALOG_H
