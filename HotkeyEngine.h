#ifndef HOTKEYENGINE_H
#define HOTKEYENGINE_H

#include <memory>
#include <functional>
#include <windows.h>

#ifdef _WIN64
#define HKECALL
#else
#define HKECALL __cdecl
#endif

//As with TskbrNtfAreaIcon this is also singleton class
//Thoughts are the same - things are pretty incapsulated but only one instance is allowed to keep things simple giving customization options
class HotkeyEngine {
public:
	typedef bool (HKECALL *KeyPressFn)(LPARAM event_param, WPARAM llkh_msg, KBDLLHOOKSTRUCT* llkh_struct);
private:
	static std::unique_ptr<HotkeyEngine> instance;
	static LPARAM event_param;	//Custom value to be passed to the OnKeyPress event handler
	static KeyPressFn OnKeyPress;	//Keyboard event is passed to OnKeyPress - it should return true if event shouldn't be passed further (to other keyboard handlers) and false otherwise
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	
	bool running;
	HANDLE hook_thread_handle;
	DWORD hook_thread_id;
	size_t stack_commit;
	
	HotkeyEngine();
public:
	static HotkeyEngine* MakeInstance();	
	bool IsRunning();
	bool Stop();
	bool Start();
	bool Set(LPARAM event_param, KeyPressFn OnKeyPress, size_t stack_commit=0);
	bool StartNew(LPARAM event_param, KeyPressFn OnKeyPress, size_t stack_commit=0);
	
	~HotkeyEngine();
	HotkeyEngine(const HotkeyEngine&)=delete;				//Get rid of default copy constructor
	HotkeyEngine& operator=(const HotkeyEngine&)=delete;	//Get rid of default copy assignment operator
	HotkeyEngine(const HotkeyEngine&&)=delete;				//Get rid of default move constructor
	HotkeyEngine& operator=(const HotkeyEngine&&)=delete;	//Get rid of default move assignment operator
};

#endif //HOTKEYENGINE_H
