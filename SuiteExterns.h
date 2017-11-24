#ifndef SUITEEXTERNS_H
#define SUITEEXTERNS_H

#include <memory>
#include <windows.h>
#define SECURITY_WIN32
#include <security.h>
#include <commctrl.h>
#include <initguid.h>
#include <wincodec.h>

class SuiteExterns {
private:
	static std::unique_ptr<SuiteExterns> instance;
	
	HMODULE hShell32;
	HMODULE hComctl32;
	HMODULE hSecur32;
	HMODULE hShfolder;
	HMODULE hUser32;
	HMODULE hWincodec;
	
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
typedef HRESULT (WINAPI *pSHGetStockIconInfo)(SHSTOCKICONID siid, UINT uFlags, SHSTOCKICONINFO *psii);
typedef BOOL (WINAPI *pChangeWindowMessageFilter)(UINT message, DWORD dwFlag);
typedef HRESULT (WINAPI *pWICConvertBitmapSource)(REFWICPixelFormatGUID dstFormat, IWICBitmapSource *pISrc, IWICBitmapSource **ppIDst);
typedef BOOL (WINAPI *pSetMenuInfo)(HMENU hmenu, LPCMENUINFO lpcmi);
typedef BOOL (WINAPI *pGetMenuInfo)(HMENU hmenu, LPCMENUINFO lpcmi);


#endif //SUITEEXTERNS_H
