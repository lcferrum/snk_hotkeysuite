#ifndef SUITEBINDINGDIALOG_H
#define SUITEBINDINGDIALOG_H

#include "HotkeyEngine.h"
#include <windows.h>

#define DLGBX_FN_INV_PARAM					0
#define DLGBX_FN_FAILED						-1
#define BD_DLGPRC_OK						1
#define BD_DLGPRC_CANCEL					2
#define BD_DLGPRC_ERROR						3

typedef struct {
	HotkeyEngine* hk_engine;
	DWORD original_vk;
	DWORD original_sc;
	DWORD binded_vk;
	DWORD binded_sc;
} BINDING_DLGPRC_PARAM;

INT_PTR CALLBACK BindingDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif //SUITEBINDINGDIALOG_H
