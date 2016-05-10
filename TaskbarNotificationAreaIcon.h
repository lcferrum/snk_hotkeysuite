#ifndef TASKBARNOTIFICATIONAREAICON_H
#define TASKBARNOTIFICATIONAREAICON_H

#include <memory>
#include <windows.h>

class TskbrNtfAreaIcon {
private:
	static std::unique_ptr<TskbrNtfAreaIcon> instance;
	static UINT WmTaskbarCreated;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	bool valid;
	HINSTANCE app_instance;
	NOTIFYICONDATA icon_ntfdata;
	
	TskbrNtfAreaIcon(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid);
protected:
	virtual void IconMenu(HMENU) {}
public:
	static TskbrNtfAreaIcon* MakeInstance(HINSTANCE hInstance, UINT icon_wm, const wchar_t* icon_tooltip, UINT icon_resid);	
	bool IsValid();
	void ChangeIconTooltip(const wchar_t* icon_tooltip);
	void ChangeIcon(UINT icon_resid);
	
	~TskbrNtfAreaIcon();
	TskbrNtfAreaIcon(const TskbrNtfAreaIcon&)=delete;				//Get rid of default copy constructor
	TskbrNtfAreaIcon& operator=(const TskbrNtfAreaIcon&)=delete;	//Get rid of default copy assignment operator
};

#endif //TASKBARNOTIFICATIONAREAICON_H
