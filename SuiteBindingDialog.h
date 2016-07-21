#ifndef SUITEBINDINGDIALOG_H
#define SUITEBINDINGDIALOG_H

#include "SuiteSettings.h"
#include "HotkeyEngine.h"
#include <windows.h>

#define DLGBX_FN_INV_PARAM					0
#define DLGBX_FN_FAILED						-1
#define BD_DLGPRC_OK						1
#define BD_DLGPRC_CANCEL					2
#define BD_DLGPRC_ERROR						3

typedef struct {
	HotkeyEngine* hk_engine;
	SuiteSettings* settings;	//Though SuiteSettings is passed to BindingDialogProc it should be used only to query values, not set them
	DWORD binded_vk;			//While only scancodes are compared in hotkey handler because of their layout independence, vks are used to distinguish several layout-independent system keys with common scancodes and to display key name in UI
	DWORD binded_sc;			//Non-zero binded_sc indicates that there was binding attempt (key was pressed)
} BINDING_DLGPRC_PARAM;

INT_PTR CALLBACK BindingDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif //SUITEBINDINGDIALOG_H
