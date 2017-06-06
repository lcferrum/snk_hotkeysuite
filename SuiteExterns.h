#ifndef SUITEEXTERNS_H
#define SUITEEXTERNS_H

#include <memory>
#include <windows.h>
#define SECURITY_WIN32
#include <security.h>
#include <commctrl.h>

class SuiteExterns {
private:
	static std::unique_ptr<SuiteExterns> instance;
	
	HMODULE hShell32;
	HMODULE hComctl32;
	HMODULE hSecur32;
	HMODULE hShfolder;
	HMODULE hUser32;
	
	void LoadFunctions();
	void UnloadFunctions();
	
	SuiteExterns();
public:
	~SuiteExterns();
	SuiteExterns(const SuiteExterns&)=delete;				//Get rid of default copy constructor
	SuiteExterns& operator=(const SuiteExterns&)=delete;	//Get rid of default copy assignment operator
	SuiteExterns(const SuiteExterns&&)=delete;				//Get rid of default move constructor
	SuiteExterns& operator=(const SuiteExterns&&)=delete;	//Get rid of default move assignment operator
	
	static bool MakeInstance();	
};

typedef HRESULT (WINAPI *pSHGetFolderPath)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
typedef BOOL (WINAPI *pSHGetSpecialFolderPath)(HWND hwndOwner, LPWSTR lpszPath, int csidl, BOOL fCreate);
typedef HRESULT (WINAPI *pTaskDialog)(HWND hWndParent, HINSTANCE hInstance, PCWSTR pszWindowTitle, PCWSTR pszMainInstruction, PCWSTR pszContent, TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons, PCWSTR pszIcon, int *pnButton);
typedef BOOLEAN (WINAPI *pGetUserNameEx)(EXTENDED_NAME_FORMAT NameFormat, LPTSTR lpNameBuffer, PULONG lpnSize);
typedef int (WINAPI *pSHCreateDirectoryEx)(HWND hwnd, LPCWSTR pszPath, const SECURITY_ATTRIBUTES *psa);
typedef int (WINAPI *pSHCreateDirectory)(HWND hwnd, LPCWSTR pszPath);
typedef BOOL (WINAPI *pChangeWindowMessageFilter)(UINT  message, DWORD dwFlag);

#endif //SUITEEXTERNS_H
