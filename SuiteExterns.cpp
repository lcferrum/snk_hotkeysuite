#include "SuiteExterns.h"

pSHGetFolderPath fnSHGetFolderPathShell32=NULL;
pSHGetFolderPath fnSHGetFolderPathShfolder=NULL;
pSHGetSpecialFolderPath fnSHGetSpecialFolderPath=NULL;
pTaskDialog fnTaskDialog=NULL;
pGetUserNameEx fnGetUserNameEx=NULL;
pChangeWindowMessageFilter fnChangeWindowMessageFilter=NULL;
pSHCreateDirectoryEx fnSHCreateDirectoryEx=NULL;
pSHCreateDirectory fnSHCreateDirectory=NULL;
pWICConvertBitmapSource fnWICConvertBitmapSource=NULL;
pSHGetStockIconInfo fnSHGetStockIconInfo=NULL;
pSetMenuInfo fnSetMenuInfo=NULL;
pGetMenuInfo fnGetMenuInfo=NULL;
pFlashWindowEx fnFlashWindowEx=NULL;
pCheckTokenMembership fnCheckTokenMembership=NULL;

std::unique_ptr<SuiteExterns> SuiteExterns::instance;

SuiteExterns::SuiteExterns(): 
	hShell32(NULL), hComctl32(NULL), hSecur32(NULL), hShfolder(NULL), hUser32(NULL), hWincodec(NULL), hAdvapi32(NULL)
{
	LoadFunctions();
}

SuiteExterns::~SuiteExterns() {
	UnloadFunctions();
}

bool SuiteExterns::MakeInstance() 
{
	if (instance)
		return false;
	
	instance.reset(new SuiteExterns());
	return true;
}

//Checking if DLLs are alredy loaded before LoadLibrary is cool but redundant
//This method is private and called (and designed to be called) only once - in constructor before everything else
void SuiteExterns::LoadFunctions() 
{
	hShell32=LoadLibrary(L"shell32.dll");
	hComctl32=LoadLibrary(L"comctl32.dll");
	hSecur32=LoadLibrary(L"secur32.dll");
	hShfolder=LoadLibrary(L"shfolder.dll");
	hUser32=LoadLibrary(L"user32.dll");
	hWincodec=LoadLibrary(L"windowscodecs.dll");
	hAdvapi32=LoadLibrary(L"advapi32.dll");
	
	if (hShfolder) {
		fnSHGetFolderPathShfolder=(pSHGetFolderPath)GetProcAddress(hShfolder, "SHGetFolderPathW");
	}

	if (hShell32) {
		fnSHGetFolderPathShell32=(pSHGetFolderPath)GetProcAddress(hShell32, "SHGetFolderPathW");
		if (!(fnSHGetSpecialFolderPath=(pSHGetSpecialFolderPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathW")))
			//Try to export by ordinal
			//WARNING: on Win 9x/Me version of shell32.dll this will export SHGetSpecialFolderPathA (ANSI version) instead
			fnSHGetSpecialFolderPath=(pSHGetSpecialFolderPath)GetProcAddress(hShell32, (char*)175);
		fnSHCreateDirectoryEx=(pSHCreateDirectoryEx)GetProcAddress(hShell32, "SHCreateDirectoryExW");
		//WARNING: on Win 9x/Me version of shell32.dll SHCreateDirectory is ANSI function
		fnSHCreateDirectory=(pSHCreateDirectory)GetProcAddress(hShell32, (char*)165);
		fnSHGetStockIconInfo=(pSHGetStockIconInfo)GetProcAddress(hShell32, "SHGetStockIconInfo");
	}
	
	if (hComctl32) {
		fnTaskDialog=(pTaskDialog)GetProcAddress(hComctl32, "TaskDialog");
	}
	
	if (hSecur32) {
		fnGetUserNameEx=(pGetUserNameEx)GetProcAddress(hSecur32, "GetUserNameExW");
	}
	
	if (hUser32) {
		fnChangeWindowMessageFilter=(pChangeWindowMessageFilter)GetProcAddress(hUser32, "ChangeWindowMessageFilter");
		fnSetMenuInfo=(pSetMenuInfo)GetProcAddress(hUser32, "SetMenuInfo");
		fnGetMenuInfo=(pGetMenuInfo)GetProcAddress(hUser32, "GetMenuInfo");
		fnFlashWindowEx=(pFlashWindowEx)GetProcAddress(hUser32, "FlashWindowEx");
	}
	
	if (hWincodec) {
		fnWICConvertBitmapSource=(pWICConvertBitmapSource)GetProcAddress(hWincodec, "WICConvertBitmapSource");
	}
	
	if (hAdvapi32) {
		fnCheckTokenMembership=(pCheckTokenMembership)GetProcAddress(hAdvapi32, "CheckTokenMembership");
	}
}

//And here we are testing for NULLs because LoadLibrary can fail in method above
void SuiteExterns::UnloadFunctions() 
{
	if (hAdvapi32) FreeLibrary(hAdvapi32);
	if (hWincodec) FreeLibrary(hWincodec);
	if (hUser32) FreeLibrary(hUser32);
	if (hShfolder) FreeLibrary(hShfolder);
	if (hSecur32) FreeLibrary(hSecur32);
	if (hShell32) FreeLibrary(hShell32);
	if (hComctl32) FreeLibrary(hComctl32);
}
