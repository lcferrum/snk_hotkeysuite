#include "SuiteMain.h"
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"
#include <memory>
#include <windows.h>
#include <typeinfo> 

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
	SuiteExterns::MakeInstance();
	
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
				std::wcerr<<L"DEFAULT SETTINGS_INI: INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
			} else {
#ifdef DEBUG
				std::wcerr<<L"STORED SETTINGS_REG: REG_KEY="<<Settings->GetStoredLocation()<<std::endl;
#endif
			}
		} else {
#ifdef DEBUG
			std::wcerr<<L"STORED SETTINGS_APPDATA: INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
		}
	} else {
#ifdef DEBUG
		std::wcerr<<L"STORED SETTINGS_INI: INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
	}
	
	SuiteSettings *polytest=Settings.get();
	std::wcerr<<L"SuiteSettings=SuiteSettings: "<<(typeid(*polytest)==typeid(SuiteSettings)?L"TRUE":L"FALSE")<<std::endl;
	std::wcerr<<L"SuiteSettings=SuiteSettingsReg: "<<(typeid(*polytest)==typeid(SuiteSettingsReg)?L"TRUE":L"FALSE")<<std::endl;
	std::wcerr<<L"SuiteSettings=SuiteSettingsIni: "<<(typeid(*polytest)==typeid(SuiteSettingsIni)?L"TRUE":L"FALSE")<<std::endl;
	std::wcerr<<L"SuiteSettings=SuiteSettingsSection: "<<(typeid(*polytest)==typeid(SuiteSettingsSection)?L"TRUE":L"FALSE")<<std::endl;
	std::wcerr<<L"SuiteSettings=SuiteSettingsAppData: "<<(typeid(*polytest)==typeid(SuiteSettingsAppData)?L"TRUE":L"FALSE")<<std::endl;
	
	if (!Settings->SaveSettings()) {
		ErrorMessage(L"Failed to load settings!");
		return ERR_SUITE+1;
	} else
		return SuiteMain(hInstance, Settings.get());	//Generates it's own error messages and sets (returns) exit code accordingly
}