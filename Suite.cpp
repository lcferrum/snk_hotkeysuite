#include "SuiteMain.h"
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include "SuiteSettings.h"
#include "SuiteExternalRelations.h"
#include <memory>
#include <cstdlib>
#include <windows.h>

#ifdef DEBUG
#include <iostream>
#endif

enum class CmdRes:char {DEFAULT, SETTINGS_SET, EXTERNAL_CALLED, ERR_MANY_ARGS, ERR_FEW_ARGS, ERR_UNKNOWN, ERR_NOT_IMPLEMENTED};

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
	
	//Even though SuiteSettings supports storing configs in registry and in INI files on per-section basis - we doesn't support it for now
	//Storing setting in INI on per-section basis is not so useful feature
	//While storing settings in the registry seems to be useful, it's also controversial: SnK scripts can't be stored in the registry and should be stored elsewhere
	//Currently SnK scripts are stored with binary for registry settings, so they can't be used out of the box for per-user (in contrast with per-machine) use

	CmdRes cmd_res=CmdRes::DEFAULT;
	int ext_res;
	const wchar_t ARG_ALL[]=L"all";
	const wchar_t ARG_CUR[]=L"current";
	if (wcslen(lpCmdLine)) {
		wchar_t** cmd_argv;
		int cmd_argc;	
#ifdef DEBUG
		std::wcerr<<L"CMDLINE = \""<<lpCmdLine<<L"\""<<std::endl;
#endif
		if ((cmd_argv=CommandLineToArgvW(lpCmdLine, &cmd_argc))) {
#ifdef DEBUG
			int tst_argc=cmd_argc;
			while (tst_argc--)
				std::wcerr<<L"ARG["<<tst_argc<<L"] = \""<<cmd_argv[tst_argc]<<L"\""<<std::endl;
#endif		
			if (cmd_argc) {
				if (!wcscmp(cmd_argv[0], L"/S")) {
					if (cmd_argc>4) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::Schedule(true, cmd_argv+2, cmd_argc-2);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::Schedule(false, cmd_argv+2, cmd_argc-2);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						cmd_res=CmdRes::ERR_FEW_ARGS;
					}
				} else if (!wcscmp(cmd_argv[0], L"/A")) {
					if (cmd_argc>4) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::AddToAutorun(true, cmd_argv+2, cmd_argc-2);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::AddToAutorun(false, cmd_argv+2, cmd_argc-2);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						cmd_res=CmdRes::ERR_FEW_ARGS;
					}
				} else if (!wcscmp(cmd_argv[0], L"/U")) {
					if (cmd_argc>2) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::Unschedule(true);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::Unschedule(false);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						cmd_res=CmdRes::ERR_FEW_ARGS;
					}
				} else if (!wcscmp(cmd_argv[0], L"/R")) {
					if (cmd_argc>2) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::RemoveFromAutorun(true);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::RemoveFromAutorun(false);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						cmd_res=CmdRes::ERR_FEW_ARGS;
					}
				} else if (!wcscmp(cmd_argv[0], L"/P")) {
					if (cmd_argc>2) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::AddToPath(true);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::AddToPath(false);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						cmd_res=CmdRes::ERR_FEW_ARGS;
					}
				} else if (!wcscmp(cmd_argv[0], L"/C")) {
					if (cmd_argc>2) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::RemoveFromPath(true);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							ext_res=SuiteExtRel::RemoveFromPath(false);
							cmd_res=CmdRes::EXTERNAL_CALLED;
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						cmd_res=CmdRes::ERR_FEW_ARGS;
					}
				} else if (!wcscmp(cmd_argv[0], L"/i")) {
					if (cmd_argc>2) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						Settings.reset(new SuiteSettingsIni(cmd_argv[1]));
						cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
						std::wcerr<<L"SET SETTINGS_INI (PATH): INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
					} else {
						Settings.reset(new SuiteSettingsIni());
						cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
						std::wcerr<<L"SET SETTINGS_INI (AUTO): INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
					}
				} else if (!wcscmp(cmd_argv[0], L"/a")) {
					if (cmd_argc>2) {
						cmd_res=CmdRes::ERR_MANY_ARGS;
					} else if (cmd_argc>1) {
						if (!wcsncmp(cmd_argv[1], ARG_CUR, wcslen(cmd_argv[1]))) {
							Settings.reset(new SuiteSettingsAppData(true));
							cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
							std::wcerr<<L"SET SETTINGS_APPDATA (CURRENT): INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
						} else if (!wcsncmp(cmd_argv[1], ARG_ALL, wcslen(cmd_argv[1]))) {
							Settings.reset(new SuiteSettingsAppData(false));
							cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
							std::wcerr<<L"SET SETTINGS_APPDATA (ALL): INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
						} else {
							cmd_res=CmdRes::ERR_UNKNOWN;
						}
					} else {
						Settings.reset(new SuiteSettingsAppData());
						cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
						std::wcerr<<L"SET SETTINGS_APPDATA (AUTO): INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
					}
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
	
	if (cmd_res==CmdRes::EXTERNAL_CALLED)
		return ext_res;
	
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
				Settings.reset(new SuiteSettingsIni());
#ifdef DEBUG
				std::wcerr<<L"DEFAULT SETTINGS_INI: INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
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