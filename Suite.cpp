#include "SuiteMain.h"
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"
#include <memory>
#include <cstdlib>
#include <windows.h>

#ifdef DEBUG
#include <iostream>
#endif

enum class CmdRes:char {DEFAULT, SETTINGS_SET, ERR_MANY_ARGS, ERR_FEW_ARGS, ERR_UNKNOWN, ERR_NOT_IMPLEMENTED};

#ifdef OBSOLETE_WWINMAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
	LPWSTR lpCmdLine=GetCommandLineW();
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#endif
	SuiteExterns::MakeInstance();
	std::unique_ptr<SuiteSettings> Settings;

	CmdRes cmd_res=CmdRes::DEFAULT;
	if (wcslen(lpCmdLine)) {
		wchar_t** cmd_argv;
		int cmd_argc;	
		if ((cmd_argv=CommandLineToArgvW(lpCmdLine, &cmd_argc))) {
#ifdef DEBUG
			int tst_argc=cmd_argc;
			while (tst_argc--)
				std::wcerr<<L"ARG["<<tst_argc<<L"] = \""<<cmd_argv[tst_argc]<<L"\""<<std::endl;
#endif		
			if (cmd_argc) {
				if (cmd_argc>2) {
					cmd_res=CmdRes::ERR_MANY_ARGS;
				} else if (!wcscmp(cmd_argv[0], L"/schedule")) {
					if (cmd_argc==1)
						cmd_res=CmdRes::ERR_NOT_IMPLEMENTED;
					else
						cmd_res=CmdRes::ERR_MANY_ARGS;
				} else if (!wcscmp(cmd_argv[0], L"/autostart")) {
					if (cmd_argc==1)
						cmd_res=CmdRes::ERR_NOT_IMPLEMENTED;
					else
						cmd_res=CmdRes::ERR_MANY_ARGS;
				} else if (!wcscmp(cmd_argv[0], L"/remove")) {
					if (cmd_argc==1)
						cmd_res=CmdRes::ERR_NOT_IMPLEMENTED;
					else
						cmd_res=CmdRes::ERR_MANY_ARGS;
				} else if (!wcscmp(cmd_argv[0], L"/reg")) {
					if (cmd_argc==1) {
						Settings.reset(new SuiteSettingsReg());
						cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
						std::wcerr<<L"SET SETTINGS_REG: REG_KEY="<<Settings->GetStoredLocation()<<std::endl;
#endif
					} else
						cmd_res=CmdRes::ERR_MANY_ARGS;
				} else if (!wcscmp(cmd_argv[0], L"/ini")) {
					if (cmd_argc==1)
						Settings.reset(new SuiteSettingsIni());
					else
						Settings.reset(new SuiteSettingsIni(cmd_argv[1]));
#ifdef DEBUG
					std::wcerr<<L"SET SETTINGS_INI: INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
					cmd_res=CmdRes::SETTINGS_SET;
				} else if (!wcscmp(cmd_argv[0], L"/appdata")) {
					if (cmd_argc==1) {
						Settings.reset(new SuiteSettingsAppData());
						cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
						std::wcerr<<L"SET SETTINGS_APPDATA: INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
					} else
						cmd_res=CmdRes::ERR_MANY_ARGS;
				} else {
					cmd_res=CmdRes::ERR_UNKNOWN;
				}
			}

			LocalFree(cmd_argv);
		} else {
			ErrorMessage(L"Failed to parse command line!");
			return ERR_SUITE+2;
		}
	}
	
	if (cmd_res!=CmdRes::DEFAULT&&cmd_res!=CmdRes::SETTINGS_SET) {
		if (cmd_res==CmdRes::ERR_UNKNOWN) ErrorMessage(L"Unknown command line argument!");
		if (cmd_res==CmdRes::ERR_MANY_ARGS) ErrorMessage(L"Too many command line arguments!");
		if (cmd_res==CmdRes::ERR_FEW_ARGS) ErrorMessage(L"Not enough command line arguments!");
		if (cmd_res==CmdRes::ERR_NOT_IMPLEMENTED) ErrorMessage(L"Function not implemented!");
		return ERR_SUITE+2;
	}

	if (cmd_res==CmdRes::DEFAULT) {
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
	}

	if (!Settings->SaveSettings()) {
		ErrorMessage(L"Failed to load settings!");
		return ERR_SUITE+1;
	} else
		return SuiteMain(hInstance, Settings.get());	//Generates it's own error messages and sets (returns) exit code accordingly
}