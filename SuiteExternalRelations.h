#ifndef SUITEEXTERNALRELATIONS_H
#define SUITEEXTERNALRELATIONS_H

#include <windows.h>

namespace SuiteExtRel {
	int Schedule(bool current_user, wchar_t** argv, int argc);
	int Unschedule(bool current_user);
	int AddToAutorun(bool current_user, wchar_t** argv, int argc);
	int RemoveFromAutorun(bool current_user);
}

#endif //SUITEEXTERNALRELATIONS_H
