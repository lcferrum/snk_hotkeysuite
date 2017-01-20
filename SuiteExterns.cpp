#include "SuiteExterns.h"

pSHGetFolderPath fnSHGetFolderPath=NULL;
pSHGetSpecialFolderPath fnSHGetSpecialFolderPath=NULL;
pTaskDialog fnTaskDialog=NULL;

std::unique_ptr<SuiteExterns> SuiteExterns::instance;

SuiteExterns::SuiteExterns(): 
	hShell32(NULL), hComctl32(NULL)
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

	if (hShell32) {
		fnSHGetFolderPath=(pSHGetFolderPath)GetProcAddress(hShell32, "SHGetFolderPathW");
		fnSHGetSpecialFolderPath=(pSHGetSpecialFolderPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathW");
	}
	
	if (hComctl32) {
		fnTaskDialog=(pTaskDialog)GetProcAddress(hComctl32, "TaskDialog");
	}
}

//And here we are testing for NULLs because LoadLibrary can fail in method above
void SuiteExterns::UnloadFunctions() 
{
	if (hShell32) FreeLibrary(hShell32);
	if (hComctl32) FreeLibrary(hComctl32);
}
