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
enum class FstCmd:char {DEFAULT, LONG_PRESS, SHORT_PRESS, SHELL};

CmdRes ProcessSettingsOptions(std::unique_ptr<SuiteSettings> &Settings, int cmd_argc, wchar_t** cmd_argv, int cmd_shift);

#define ARG_ALL L"machine"
#define ARG_CUR L"user"

#ifdef OBSOLETE_WWINMAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
	LPWSTR lpCmdLine=GetCommandLineW();
	//We should drop first argument which is executable path
	//And it's more of a convention that most programmers adhere to than strict requirement
	//Because actually you can call CreateProcess explicitly passing application name and omitting it from command line, which is a separate argument (not the case with ShellExecuteEx)
	bool inside_quotes=false;
	while (*lpCmdLine>L' '||(*lpCmdLine&&inside_quotes)) {
		if (*lpCmdLine==L'\"')
			inside_quotes=!inside_quotes;
		lpCmdLine++;
	}
	while (*lpCmdLine&&(*lpCmdLine<=L' '))
		lpCmdLine++;
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

	//Command line options:
	//	/S machine|user [ARG1] [ARG2]   - schedule application with Task Scheduler (run w/ highest privileges), arguments will be passed as command line options
	//	/A machine|user [ARG1] [ARG2]   - add application to Autorun, arguments will be passed as command line options
	//	/U machine|user                 - delete scheduled application from Task Scheduler
	//	/R machine|user                 - remove application from Autorun
	//	/P machine|user                 - add application directory to Path environment variable
	//	/C machine|user                 - remove application directory to Path environment variable
	//	/i [INI_PATH]                   - launch application and store settings in ini file pointed by INI_PATH argument or, if absent, in HotkeySuite.ini located in application directory
	//	/a [machine|user]               - launch application and store settings in ini file located in per-user (user) or all-user (machine) AppData directory (automatically selected if argument is absent)
	//	NO ARGUMENTS GIVEN              - launch application and automatically select ini file location
	//	/s ...                          - immidiately run sungle press event and exit (used with /i and /a options or w/o arguments)
	//	/l ...                          - immidiately run long press event and exit (used with /i and /a options or w/o arguments)
	//	/p ...                          - launch SnK shell and exit (used with /i and /a options or w/o arguments)
	
	CmdRes cmd_res=CmdRes::DEFAULT;
	FstCmd fst_cmd=FstCmd::DEFAULT;
	int ext_res;
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
				if (!wcscmp(cmd_argv[0], L"/s"))
					fst_cmd=FstCmd::SHORT_PRESS;
				else if (!wcscmp(cmd_argv[0], L"/l"))
					fst_cmd=FstCmd::LONG_PRESS;
				else if (!wcscmp(cmd_argv[0], L"/p"))
					fst_cmd=FstCmd::SHELL;
				
				if (fst_cmd!=FstCmd::DEFAULT) {
					if (cmd_argc>1) cmd_res=ProcessSettingsOptions(Settings, cmd_argc, cmd_argv, 1);
				} else {
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
					} else {
						cmd_res=ProcessSettingsOptions(Settings, cmd_argc, cmd_argv, 0);
					}
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
	}
	
	switch (fst_cmd) {
		case FstCmd::SHELL:
			return SuiteExtRel::LaunchCommandPrompt(Settings.get(), true);
		case FstCmd::LONG_PRESS:
		case FstCmd::SHORT_PRESS:
			return SuiteExtRel::FireEvent(fst_cmd==FstCmd::LONG_PRESS, Settings.get());
		default:
			{
				int suite_main_err=SuiteMain(Settings.get());	//Generates it's own error messages and sets (returns) exit code accordingly
				if (suite_main_err==ERR_ELEVATE)
					SuiteExtRel::RestartApplication(lpCmdLine, true);
				else if (suite_main_err==ERR_RESTART)
					SuiteExtRel::RestartApplication(lpCmdLine, false);
				return suite_main_err;
			}
	}
}

CmdRes ProcessSettingsOptions(std::unique_ptr<SuiteSettings> &Settings, int cmd_argc, wchar_t** cmd_argv, int cmd_shift)
{
	CmdRes cmd_res=CmdRes::DEFAULT;

	if (!wcscmp(cmd_argv[cmd_shift], L"/i")) {
		if (cmd_argc>cmd_shift+2) {
			cmd_res=CmdRes::ERR_MANY_ARGS;
		} else if (cmd_argc>cmd_shift+1) {
			Settings.reset(new SuiteSettingsIni(cmd_argv[cmd_shift+1]));
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
	} else if (!wcscmp(cmd_argv[cmd_shift], L"/a")) {
		if (cmd_argc>cmd_shift+2) {
			cmd_res=CmdRes::ERR_MANY_ARGS;
		} else if (cmd_argc>cmd_shift+1) {
			if (!wcsncmp(cmd_argv[cmd_shift+1], ARG_CUR, wcslen(cmd_argv[cmd_shift+1]))) {
				Settings.reset(new SuiteSettingsAppData(true));
				cmd_res=CmdRes::SETTINGS_SET;
#ifdef DEBUG
				std::wcerr<<L"SET SETTINGS_APPDATA (CURRENT): INI_PATH="<<Settings->GetStoredLocation()<<std::endl;
#endif
			} else if (!wcsncmp(cmd_argv[cmd_shift+1], ARG_ALL, wcslen(cmd_argv[cmd_shift+1]))) {
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
	
	return cmd_res;
}
