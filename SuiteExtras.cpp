#include "SuiteExtras.h"

pSHGetFolderPath fnSHGetFolderPath=NULL;
pSHGetSpecialFolderPath fnSHGetSpecialFolderPath=NULL;
pTaskDialog fnTaskDialog=NULL;

std::unique_ptr<SuiteExtras> SuiteExtras::instance;

SuiteExtras::SuiteExtras(): 
	hShell32(NULL), hComctl32(NULL)
{
	LoadFunctions();
}

SuiteExtras::~SuiteExtras() {
	UnloadFunctions();
}

bool SuiteExtras::MakeInstance() 
{
	if (instance)
		return false;
	
	instance.reset(new SuiteExtras());
	return true;
}

//Checking if DLLs are alredy loaded before LoadLibrary is cool but redundant
//This method is private and called (and designed to be called) only once - in constructor before everything else
void SuiteExtras::LoadFunctions() 
{
	hShell32=LoadLibrary(L"shell32.dll");
	hComctl32=LoadLibrary(L"comctl32.dll");

	if (hShell32) {
		fnSHGetFolderPath=(pSHGetFolderPath)GetProcAddress(hShell32, "SHGetFolderPathW");
		fnSHGetSpecialFolderPath=(pSHGetSpecialFolderPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathW");
	}
	
	if (hComctl32) {
		fnTaskDialog=(pTaskDialog)GetProcAddress(hComctl32, "TaskDialog");
	}
}

//And here we are testing for NULLs because LoadLibrary can fail in method above
void SuiteExtras::UnloadFunctions() 
{
	if (hShell32) FreeLibrary(hShell32);
	if (hComctl32) FreeLibrary(hComctl32);
}
