#include "SuiteExterns.h"

pSHGetFolderPath fnSHGetFolderPathShell32=NULL;
pSHGetFolderPath fnSHGetFolderPathShfolder=NULL;
pSHGetSpecialFolderPath fnSHGetSpecialFolderPath=NULL;
pTaskDialog fnTaskDialog=NULL;
pGetUserNameEx fnGetUserNameEx=NULL;
pChangeWindowMessageFilter fnChangeWindowMessageFilter=NULL;

std::unique_ptr<SuiteExterns> SuiteExterns::instance;

SuiteExterns::SuiteExterns(): 
	hShell32(NULL), hComctl32(NULL), hSecur32(NULL), hShfolder(NULL), hUser32(NULL)
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
	
	if (hShfolder) {
		fnSHGetFolderPathShfolder=(pSHGetFolderPath)GetProcAddress(hShfolder, "SHGetFolderPathW");
	}

	if (hShell32) {
		fnSHGetFolderPathShell32=(pSHGetFolderPath)GetProcAddress(hShell32, "SHGetFolderPathW");
		if (!(fnSHGetSpecialFolderPath=(pSHGetSpecialFolderPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathW")))
			//Try to export by ordinal
			//WARNING: on Win 9x/Me version of shell32.dll this will export SHGetSpecialFolderPathA (ANSI version) instead
			fnSHGetSpecialFolderPath=(pSHGetSpecialFolderPath)GetProcAddress(hShell32, (char*)175);
	}
	
	if (hComctl32) {
		fnTaskDialog=(pTaskDialog)GetProcAddress(hComctl32, "TaskDialog");
	}
	
	if (hSecur32) {
		fnGetUserNameEx=(pGetUserNameEx)GetProcAddress(hSecur32, "GetUserNameExW");
	}
	
	if (hUser32) {
		fnChangeWindowMessageFilter=(pChangeWindowMessageFilter)GetProcAddress(hUser32, "ChangeWindowMessageFilter");
	}
}

//And here we are testing for NULLs because LoadLibrary can fail in method above
void SuiteExterns::UnloadFunctions() 
{
	if (hUser32) FreeLibrary(hUser32);
	if (hShfolder) FreeLibrary(hShfolder);
	if (hSecur32) FreeLibrary(hSecur32);
	if (hShell32) FreeLibrary(hShell32);
	if (hComctl32) FreeLibrary(hComctl32);
}
