#ifndef HOTKEYENGINE_H
#define HOTKEYENGINE_H

#include <memory>
#include <functional>
#include <windows.h>

//As with TskbrNtfAreaIcon. this is also singleton class
//Thoughts are the same - things are pretty incapsulated but only one instance is allowed to keep things simple giving customization options
class HotkeyEngine {
public:
	typedef std::function<bool(WPARAM wParam, KBDLLHOOKSTRUCT* kb_event)> KeyPressFn;
private:
	static std::unique_ptr<HotkeyEngine> instance;
	static KeyPressFn OnKeyPress;	//Keyboard event is passed to OnKeyPress - it should return true if event shouldn't be passed further (to other keyboard handlers) and false otherwise
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	
	bool running;
	HANDLE hook_thread_handle;
	DWORD hook_thread_id;
	HINSTANCE app_instance;
	
	HotkeyEngine(HINSTANCE hInstance);
public:
	static HotkeyEngine* MakeInstance(HINSTANCE hInstance);	
	bool IsRunning();
	bool Stop();
	bool Start();
	bool Set(KeyPressFn OnKeyPress);
	bool StartNew(KeyPressFn OnKeyPress);
	
	~HotkeyEngine();
	HotkeyEngine(const HotkeyEngine&)=delete;				//Get rid of default copy constructor
	HotkeyEngine& operator=(const HotkeyEngine&)=delete;	//Get rid of default copy assignment operator
	HotkeyEngine(const HotkeyEngine&&)=delete;				//Get rid of default move constructor
	HotkeyEngine& operator=(const HotkeyEngine&&)=delete;	//Get rid of default move assignment operator
};

#endif //HOTKEYENGINE_H
