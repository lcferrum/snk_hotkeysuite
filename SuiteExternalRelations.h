#ifndef SUITEEXTERNALRELATIONS_H
#define SUITEEXTERNALRELATIONS_H

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
}

#endif //SUITEEXTERNALRELATIONS_H
