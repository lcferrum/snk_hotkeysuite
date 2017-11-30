#ifndef SUITEEXTERNALRELATIONS_H
#define SUITEEXTERNALRELATIONS_H

#include "SuiteSettings.h"
#include <string>
#include <windows.h>

namespace SuiteExtRel {
	int AddToPath(bool current_user);
	int RemoveFromPath(bool current_user);
	int Schedule(bool current_user, wchar_t** argv, int argc);
	int Unschedule(bool current_user);
	int AddToAutorun(bool current_user, wchar_t** argv, int argc);
	int RemoveFromAutorun(bool current_user);
	bool LaunchSnkOpenDialog(std::wstring &fpath);
	void RestartApplication(const wchar_t* cmdline, bool elevate);
	int LaunchCommandPrompt(const SuiteSettings *settings, bool check_snk=false);
	int FireEvent(bool long_press, const SuiteSettings *settings);
}

#endif //SUITEEXTERNALRELATIONS_H
