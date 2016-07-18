#include "SuiteMain.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"
#include <memory>
#include <windows.h>

#ifdef OBSOLETE_WWINMAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
	LPWSTR lpCmdLine=GetCommandLineW();
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#endif
	//TODO: process lpCmdLine
	
	std::unique_ptr<SuiteSettings> Settings;
	
	Settings.reset(new SuiteSettingsAppData());
	if (!Settings->IsStored()) {
		Settings.reset(new SuiteSettingsReg());
		if (!Settings->IsStored()) {
			Settings.reset(new SuiteSettingsIni());
		}
	}
	
	if (!Settings->SaveSettings()) {
		MessageBox(NULL, L"Failed to read settings!", L"SNK_HS", MB_ICONERROR|MB_OK);
		return 0;
	} else
		return SuiteMain(hInstance, Settings.get());
}