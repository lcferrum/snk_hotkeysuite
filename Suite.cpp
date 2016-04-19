#include <cstdio>
#include <cwchar>
#include <iostream>

#ifdef OBSOLETE_WMAIN
typedef struct {
	int newmode;
} _startupinfo;
#undef _CRT_glob
extern int _CRT_glob;
extern "C" void __wgetmainargs(int*, wchar_t***, wchar_t***, int, _startupinfo*);

int main()
{
	wchar_t **enpv, **argv;
	int argc;
	_startupinfo si;
	__wgetmainargs(&argc, &argv, &enpv, _CRT_glob, &si);
#else
extern "C" int wmain(int argc, wchar_t* argv[])
{
#endif
	std::wcout<<L"Let's pretend that it is WinMain!"<<std::endl;
	
	return 0;
}