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
public:
	typedef std::function<bool(TskbrNtfAreaIcon* sender, WPARAM wParam, LPARAM lParam)> WmCommandFn;
	typedef std::function<void(TskbrNtfAreaIcon* sender)> WmCloseFn;
	typedef std::function<void(TskbrNtfAreaIcon* sender, bool critical)> WmEndsessionTrueFn;
private:
	static std::unique_ptr<TskbrNtfAreaIcon> instance;
	static UINT WmTaskbarCreated;
	static WmCommandFn OnWmCommand;	//WM_COMMAND msg is passed to OnWmCommand - it should return true if msg is fully processed or false if msg should be processed by DefWindowProc
	static WmCloseFn OnWmClose;	//WM_CLOSE msg (omitting params) is passed to OnWmClose
	static WmEndsessionTrueFn OnWmEndsessionTrue;	//WM_ENDSESSION w/ wParam=TRUE msg (omitting params, but w/ critical flag) is passed to OnWmEndsessionTrue
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	bool valid;
	bool enabled;
	HINSTANCE app_instance;
	HMENU icon_menu;
	UINT default_menuid;
	ATOM icon_atom;
	NOTIFYICONDATA icon_ntfdata;
	
	TskbrNtfAreaIcon(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid, const wchar_t* icon_class, UINT icon_menuid, UINT default_menuid);
	void ShellNotifyIconModifyOrAdd();
public:
	static TskbrNtfAreaIcon* MakeInstance(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid, const wchar_t* icon_class, UINT icon_menuid, UINT default_menuid, WmCommandFn OnWmCommand, WmCloseFn OnWmClose, WmEndsessionTrueFn OnWmEndsessionTrue);	
	bool IsValid() { return valid; }
	void ChangeIconTooltip(const wchar_t* icon_tooltip);
	void RefreshIcon();
	void ChangeIcon(UINT icon_resid);
	void Close();
	void CloseAndQuit(int exit_code=0);
	void Enable() { enabled=true; }
	void Disable() { enabled=false; }
	bool IsEnabled() { return enabled; }
	HMENU GetIconMenu();
	HWND GetIconWindow();
	BOOL ModifyIconMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem);
	BOOL EnableIconMenuItem(UINT uIDEnableItem, UINT  uEnable);
	DWORD CheckIconMenuItem(UINT uIDCheckItem, UINT uCheck);
	BOOL CheckIconMenuRadioItem(UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags);
	
	~TskbrNtfAreaIcon();
	TskbrNtfAreaIcon(const TskbrNtfAreaIcon&)=delete;				//Get rid of default copy constructor
	TskbrNtfAreaIcon& operator=(const TskbrNtfAreaIcon&)=delete;	//Get rid of default copy assignment operator
	TskbrNtfAreaIcon(const TskbrNtfAreaIcon&&)=delete;				//Get rid of default move constructor
	TskbrNtfAreaIcon& operator=(const TskbrNtfAreaIcon&&)=delete;	//Get rid of default move assignment operator
};

#endif //TASKBARNOTIFICATIONAREAICON_H
