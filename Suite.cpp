#include "SuiteMain.h"
#include "SuiteExtras.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"
#include <memory>
#include <windows.h>

#ifdef DEBUG
#include <iostream>
#endif

#ifdef OBSOLETE_WWINMAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
	LPWSTR lpCmdLine=GetCommandLineW();
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#endif
	SuiteExtras::MakeInstance();
	
	//TODO: process lpCmdLine
	
	std::unique_ptr<SuiteSettings> Settings;
	
	Settings.reset(new SuiteSettingsIni());
	if (!Settings->IsStored()) {
		Settings.reset(new SuiteSettingsAppData());
		if (!Settings->IsStored()) {
			Settings.reset(new SuiteSettingsReg());
			if (!Settings->IsStored()) {
				Settings.reset(new SuiteSettingsIni());
#ifdef DEBUG
				std::wcerr<<L"DEFAULT SETTINGS_INI: INI_PATH="<<((SuiteSettingsIni*)Settings.get())->GetIniPath()<<std::endl;
#endif
			} else {
#ifdef DEBUG
				std::wcerr<<L"STORED SETTINGS_REG: REG_KEY="<<((SuiteSettingsReg*)Settings.get())->GetRegKey()<<std::endl;
#endif
			}
		} else {
#ifdef DEBUG
			std::wcerr<<L"STORED SETTINGS_APPDATA: INI_PATH="<<((SuiteSettingsAppData*)Settings.get())->GetIniPath()<<std::endl;
#endif
		}
	} else {
#ifdef DEBUG
		std::wcerr<<L"STORED SETTINGS_INI: INI_PATH="<<((SuiteSettingsIni*)Settings.get())->GetIniPath()<<std::endl;
#endif
	}
	
	if (!Settings->SaveSettings()) {
		ErrorMessage(L"Failed to load settings!");
		return ERR_SUITE+1;
	} else
		return SuiteMain(hInstance, Settings.get());	//Generates it's own error messages and sets (returns) exit code accordingly
}