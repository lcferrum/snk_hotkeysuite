#ifndef RES_H
#define RES_H

#include <windows.h>

#define IDI_HSTNAICO		101
#define IDR_ICONMENU		102
#define IDD_BINDINGDLG		103			
#define IDM_EXIT			40001
#define IDM_STOP_START		40002
#define IDM_EDIT_SHK		40003
#define IDM_EDIT_LHK		40004
#define IDM_SET_EN_LHK		40051
#define IDM_SET_CTRL_ALT	40052
#define IDM_SET_SHIFT_ALT	40053
#define IDM_SET_CTRL_SHIFT	40054
#define IDM_SET_CUSTOM		40055
#define IDC_VK_VIEWER		30000
#define IDC_CONFIRM_VK		30001
#define IDC_CANCEL_VK		30002

#define WM_HSTNAICO			(WM_USER+1)
#define WM_BINDVK			(WM_USER+2)

#endif //RES_H
