#include "HotkeyEngine.h"

#define OBJECT_WAIT_TIMEOUT	5000	//5 secs

std::unique_ptr<HotkeyEngine> HotkeyEngine::instance;
HotkeyEngine::KeyPressFn HotkeyEngine::OnKeyPress;

HotkeyEngine* HotkeyEngine::MakeInstance(HINSTANCE hInstance)
{
	instance.reset(new HotkeyEngine(hInstance));
	return instance.get();
}

HotkeyEngine::HotkeyEngine(HINSTANCE hInstance):
	running(false), hook_thread_handle(NULL), hook_thread_id(0), app_instance(hInstance)
{}

HotkeyEngine::~HotkeyEngine() 
{
	Stop();
}

bool HotkeyEngine::IsRunning()
{
	return running;
}

void HotkeyEngine::Stop()
{
	if (!running)
		return;
	
	if (!PostThreadMessage(hook_thread_id, WM_QUIT, 0, 0)||									//Post WM_QUIT to hook thread to exit it
		WaitForSingleObject(hook_thread_handle, OBJECT_WAIT_TIMEOUT)!=WAIT_OBJECT_0)		//Wait for thread to exit
			TerminateThread(hook_thread_handle, 2);											//Or terminate it in case of error or timeout (exit code = 2)
	CloseHandle(hook_thread_handle);
	
	running=false;
}

bool HotkeyEngine::Start()
{
	if (!running) {
		HANDLE success_fail[2];	//Array for two manual-reset unsignalled events
												
		if ((success_fail[0]=CreateEvent(NULL, TRUE, FALSE, NULL))) {		//Success event
			if ((success_fail[1]=CreateEvent(NULL, TRUE, FALSE, NULL))) {	//Fail event
				if ((hook_thread_handle=CreateThread(NULL, 0, ThreadProc, (LPVOID)&success_fail, 0, &hook_thread_id))) {
					switch (WaitForMultipleObjects(2, success_fail, FALSE, OBJECT_WAIT_TIMEOUT)) {
						case WAIT_OBJECT_0:		//Success event signalled
							SetThreadPriority(hook_thread_handle, THREAD_PRIORITY_TIME_CRITICAL);	//Set higher priority for hook thread
							CloseHandle(success_fail[0]);
							CloseHandle(success_fail[1]);
							running=true;
							return true;
						case WAIT_OBJECT_0+1:	//Fail event signalled
							if (WaitForSingleObject(hook_thread_handle, OBJECT_WAIT_TIMEOUT)!=WAIT_OBJECT_0)	//Wait for thread to exit
								TerminateThread(hook_thread_handle, 2);											//Or terminate it in case of error or timeout (exit code = 2)
							break;
						default:				//Error or timeout happened - terminate thread (exit code = 2)
							TerminateThread(hook_thread_handle, 2);
							break;
					} 
					CloseHandle(hook_thread_handle);
				}
				CloseHandle(success_fail[1]);
			}
			CloseHandle(success_fail[0]);
		}
	}
	
	return false;
}

bool HotkeyEngine::StartNew(KeyPressFn OnKeyPress)
{
	if (!running) {
		HotkeyEngine::OnKeyPress=std::move(OnKeyPress);
		return Start();
	} else
		return false;
}

DWORD WINAPI HotkeyEngine::ThreadProc(LPVOID lpParameter)
{
	HANDLE* success_fail=(HANDLE*)lpParameter;	//Success and fail events
	HHOOK kb_hook;
	
	if ((kb_hook=SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance->app_instance, 0))) {
		SetEvent(success_fail[0]);	//Signal success event
	} else {
		SetEvent(success_fail[1]);	//Signal fail event and exit (exit code = 1)
		return 1;
	}
	
	//For hook to work thread should have message loop
	//Though it can be pretty castrated
	//Only GetMessage is needed because hook callback is actually called inside this one function
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)); //GetMessage returns 0 if WM_QUIT, and we are ignoring -1 (error) so not have to check if hook thread has killed itself
	
	//WM_QUIT happened - unhook and exit thread (exit code = 0)
	UnhookWindowsHookEx(kb_hook);
	return 0;
}

LRESULT CALLBACK HotkeyEngine::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//HOOKPROC requires that on less-than-zero nCode CallNextHookEx should be returned immediately
	if (nCode<0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	//Ignore keyboard events if for some reason instance is unitialized (so to prevent accessing NULL object)
	//And let CallNextHookEx handle the keyboard event if OnKeyPress returned FALSE
	if (instance&&OnKeyPress&&OnKeyPress(instance.get(), wParam, (KBDLLHOOKSTRUCT*)lParam))	
		return 1;	//We should return non-zero value if event is processed
	else
		return CallNextHookEx(NULL, nCode, wParam, lParam);
}