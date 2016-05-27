#ifndef TASKBARNOTIFICATIONAREAICON_H
#define TASKBARNOTIFICATIONAREAICON_H

#include <memory>
#include <functional>
#include <windows.h>

class TskbrNtfAreaIcon {
private:
	static std::unique_ptr<TskbrNtfAreaIcon> instance;
	static UINT WmTaskbarCreated;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	bool valid;
	HINSTANCE app_instance;
	HMENU icon_menu;
	UINT default_menuid;
	NOTIFYICONDATA icon_ntfdata;
	
	TskbrNtfAreaIcon(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid, const wchar_t* icon_class, UINT icon_menuid, UINT default_menuid);
public:
	static TskbrNtfAreaIcon* MakeInstance(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid, const wchar_t* icon_class, UINT icon_menuid, UINT default_menuid);	
	static TskbrNtfAreaIcon* GetInstance();	
	bool IsValid();
	void ChangeIconTooltip(const wchar_t* icon_tooltip);
	void ChangeIcon(UINT icon_resid);
	HMENU GetIconMenu();
	
	static std::function<bool(TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam)> OnWmCommand;
	
	~TskbrNtfAreaIcon();
	TskbrNtfAreaIcon(const TskbrNtfAreaIcon&)=delete;				//Get rid of default copy constructor
	TskbrNtfAreaIcon& operator=(const TskbrNtfAreaIcon&)=delete;	//Get rid of default copy assignment operator
};

#endif //TASKBARNOTIFICATIONAREAICON_H
