#ifndef SUITEEXTRAS_H
#define SUITEEXTRAS_H

#include <memory>
#include <windows.h>
#include <commctrl.h>

class SuiteExtras {
private:
	static std::unique_ptr<SuiteExtras> instance;
	
	HMODULE hShell32;
	HMODULE hComctl32;
	
	void LoadFunctions();
	void UnloadFunctions();
	
	SuiteExtras();
public:
	~SuiteExtras();
	SuiteExtras(const SuiteExtras&)=delete;				//Get rid of default copy constructor
	SuiteExtras& operator=(const SuiteExtras&)=delete;	//Get rid of default copy assignment operator
	
	static bool MakeInstance();	
};

typedef HRESULT (WINAPI *pSHGetFolderPath)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
typedef BOOL (WINAPI *pSHGetSpecialFolderPath)(HWND hwndOwner, LPWSTR lpszPath, int csidl, BOOL fCreate);
typedef HRESULT (WINAPI *pTaskDialog)(HWND hWndParent, HINSTANCE hInstance, PCWSTR pszWindowTitle, PCWSTR pszMainInstruction, PCWSTR pszContent, TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons, PCWSTR pszIcon, int *pnButton);

#endif //SUITEEXTRAS_H
