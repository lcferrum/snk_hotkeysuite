#ifndef TASKBARNOTIFICATIONAREAICON_H
#define TASKBARNOTIFICATIONAREAICON_H

#include <memory>
#include <functional>
#include <windows.h>

//This is singleton class
//Why not use ordinary class or maybe simple (namespaced) include?
//We are keeping this thing as a class because it represents not a collection of functions but a complete object that provides taskbar icon functionality
//Not using singleton means supporting several instances of one class - and each instance will represent different window
//But because of static WNDPROC callback, static hwnd-to-object map variable should be used to keep track for which object WNDPROC is called
//So we are using singleton just to keep things simple and omit this mapping
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
