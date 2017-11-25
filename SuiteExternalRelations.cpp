#include "SuiteExternalRelations.h"
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include <windows.h>
#include <initguid.h>
#include <mstask.h>
#include <taskschd.h>
#include <lmcons.h>
#include "taskschd_hs.h"

#ifdef DEBUG
#include <iostream>
#endif

extern pGetUserNameEx fnGetUserNameEx;

namespace SuiteExtRel {
	int Schedule10(bool &na, const wchar_t* params);
	int Schedule20(bool &na, bool current_user, const wchar_t* params);
	int Unschedule10(bool &na);
	int Unschedule20(bool &na, bool current_user);
	bool EnvQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &key_value, DWORD &key_type);
	std::wstring GetUserNameForTaskScheduler(std::wstring &xname);
	size_t FindInPath(const std::wstring &path, const wchar_t* dir, size_t *ret_len);
}

bool SuiteExtRel::LaunchSnkOpenDialog(std::wstring &fpath)
{
	wchar_t buf[MAX_PATH]=L"";

	OPENFILENAME ofn={
#ifdef _WIN64
		sizeof(OPENFILENAME),							//lStructSize
#else
		OPENFILENAME_SIZE_VERSION_400,					//lStructSize
#endif
		NULL,											//hwndOwner
		NULL,											//hInstance
		L"Windowless SnK\0SnKh.exe\0All Files\0*.*\0",	//lpstrFilter
		NULL,											//lpstrCustomFilter
		0,												//nMaxCustFilter
		1,												//nFilterIndex
		buf,											//lpstrFile
		MAX_PATH,										//nMaxFile
		NULL,											//lpstrFileTitle
		0,												//nMaxFileTitle
		NULL,											//lpstrInitialDir
		L"Select Windowless SnK Executable",			//lpstrTitle
		OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST|
		OFN_HIDEREADONLY,								//Flags
		0,												//nFileOffset
		0,												//nFileExtension
		NULL,											//lpstrDefExt
		0,												//lCustData
		NULL,											//lpfnHook
		NULL,											//lpTemplateName
		NULL,											//pvReserved
		0,												//dwReserved
		0												//FlagsEx
	};
	
	if (GetOpenFileName(&ofn)) {
		fpath=buf;
		return true;
	} else
		return false;
}

int SuiteExtRel::AddToAutorun(bool current_user, wchar_t** argv, int argc)
{
	int ret=ERR_SUITEEXTREL+6;
	
	HKEY reg_key;
	//There is a chance that Run key doesn't exist (e.g. on freshly installed OS)
	//So we are using RegCreateKeyEx here - it will just open the key if it already exists or create one otherwise
	if (RegCreateKeyEx(current_user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, NULL)==ERROR_SUCCESS) {
		std::wstring hs_path=QuoteArgument(GetExecutableFileName().c_str());
		while (argc--) {
			hs_path.push_back(L' ');
			hs_path.append(QuoteArgument(*argv++));
		}
		//RegSetValueEx will create absent value or overwrite present value (even if present value have different type)
#ifdef _WIN64
		if (RegSetValueEx(reg_key, L"SnK HotkeySuite (x64)", 0, REG_SZ, (BYTE*)hs_path.c_str(), (hs_path.length()+1)*sizeof(wchar_t))==ERROR_SUCCESS) ret=0;
#else
		if (RegSetValueEx(reg_key, L"SnK HotkeySuite", 0, REG_SZ, (BYTE*)hs_path.c_str(), (hs_path.length()+1)*sizeof(wchar_t))==ERROR_SUCCESS) ret=0;
#endif
		RegCloseKey(reg_key);
	}
	
	if (ret) ErrorMessage(L"Error adding SnK HotkeySuite to Autorun! Make sure that you have enough rights to access registry.");

	return ret;
}

int SuiteExtRel::RemoveFromAutorun(bool current_user)
{
	int ret=ERR_SUITEEXTREL+5;
	
	HKEY reg_key;
	LONG res=RegOpenKeyEx(current_user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &reg_key);
	if (res==ERROR_SUCCESS) {
#ifdef _WIN64
		res=RegDeleteValue(reg_key, L"SnK HotkeySuite (x64)");
#else
		res=RegDeleteValue(reg_key, L"SnK HotkeySuite");
#endif
		//If value doesn't exists - ignore it, but don't ignore other errors
		if (res==ERROR_SUCCESS||res==ERROR_FILE_NOT_FOUND) ret=0;
		RegCloseKey(reg_key);
	} else if (res==ERROR_FILE_NOT_FOUND) {
		//No Run key is ok too - nothing to delete
		ret=0;
	}
	
	if (ret) ErrorMessage(L"Error removing SnK HotkeySuite from Autorun! Make sure that you have enough rights to access registry.");

	return ret;
}

std::wstring SuiteExtRel::GetUserNameForTaskScheduler(std::wstring &xname)
{
	std::wstring uname;
	
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		DWORD dwSize;
		if(!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize)&&GetLastError()==ERROR_INSUFFICIENT_BUFFER) {
			PTOKEN_USER ptu=(PTOKEN_USER)new BYTE[dwSize];
			if (GetTokenInformation(hToken, TokenUser, (PVOID)ptu, dwSize, &dwSize)) {
				DWORD account_len=0;
				DWORD domain_len=0;
				SID_NAME_USE sid_type;
				if (LookupAccountSid(NULL, ptu->User.Sid, NULL, &account_len, NULL, &domain_len, &sid_type)==FALSE&&account_len&&domain_len) {
					wchar_t account[account_len];
					wchar_t domain[domain_len];
					if (LookupAccountSid(NULL, ptu->User.Sid, account, &account_len, domain, &domain_len, &sid_type)) {
						uname=domain;
						uname.push_back(L'\\');
						uname.append(account);
						xname.append(account);
						xname.push_back(L',');
						xname.append(domain);
					}
				}
			}
			
			delete[] (BYTE*)ptu;
		}
		
		CloseHandle(hToken);
	}
	
	return uname;
}

int SuiteExtRel::Unschedule(bool current_user)
{
	int ret;
	bool na;
	
	ret=Unschedule20(na, current_user);
	if (na) {
		if (!current_user) {
			//In Task Scheduler 1.0 you can't run task for any logged on user by using BUILTIN\Users group as user like in 2.0
			ErrorMessage(L"Task Scheduler 1.0 doesn't support scheduling GUI apps for all users!");
			return ERR_SUITEEXTREL+3;
		} else if ((ret=Unschedule10(na), na)) {
			ErrorMessage(L"Task Scheduler 1.0 not available!");
			return ERR_SUITEEXTREL+3;
		}
	}
	if (ret) ErrorMessage(L"Error unscheduling SnK HotkeySuite! Make sure that you have Admin rights before unscheduling a task.");
	
	return ret;
}

int SuiteExtRel::Unschedule10(bool &na)
{
	int ret=ERR_SUITEEXTREL+4;
	na=false;
	
	CoInitialize(NULL);
	
#ifdef _WIN64
	std::wstring tname(L"SnK HotkeySuite (x64) [");
#else
	std::wstring tname(L"SnK HotkeySuite [");
#endif
	if (!GetUserNameForTaskScheduler(tname).empty()) {
		tname.push_back(L']');
		
		ITaskScheduler *pITS;
		if (SUCCEEDED(CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pITS))) {
#ifdef DEBUG
			std::wcerr<<L"SCHEDULE: Task Scheduler 1.0 available"<<std::endl;
#endif
			//If task doesn't exists - ignore it, but don't ignore other errors
			HRESULT hr=pITS->Delete(tname.c_str());
			if (SUCCEEDED(hr)||hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ret=0;

			pITS->Release();
		} else {
			na=true;
			ret=0;
		}
	}
	
	CoUninitialize();
	
	return ret;
}

int SuiteExtRel::Unschedule20(bool &na, bool current_user)
{
	int ret=ERR_SUITEEXTREL+4;
	na=false;
	
	CoInitialize(NULL);	//Same as CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), whatever, we don't need COINIT_MULTITHREADED

#ifdef _WIN64
	std::wstring tname(L"SnK HotkeySuite (x64)");
#else
	std::wstring tname(L"SnK HotkeySuite");
#endif
	if (current_user) {
		tname.append({L' ', L'['});
		
		if (GetUserNameForTaskScheduler(tname).empty()) {
			CoUninitialize();
			return ret;
		}
		
		tname.push_back(L']');
	}
	
	ITaskService *pService;
	if (SUCCEEDED(CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService))) {
#ifdef DEBUG
		std::wcerr<<L"SCHEDULE: Task Scheduler 2.0 available"<<std::endl;
#endif
		ITaskDefinition *pTask;
		BSTR root_bstr;
		VARIANT empty_var={VT_EMPTY};
		if (SUCCEEDED(pService->Connect(empty_var, empty_var, empty_var, empty_var))&&
			(root_bstr=SysAllocString(L"\\"))) {
			ITaskFolder *pRootFolder;
			if (SUCCEEDED(pService->GetFolder(root_bstr, &pRootFolder))) {
				if (BSTR tname_bstr=SysAllocString(tname.c_str())) {
					//If task doesn't exists - ignore it, but don't ignore other errors
					HRESULT hr=pRootFolder->DeleteTask(tname_bstr, 0);
					if (SUCCEEDED(hr)||hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ret=0;
					SysFreeString(tname_bstr);
				}
				pRootFolder->Release();
			}
			SysFreeString(root_bstr);
		}
		pService->Release();
	} else {
		na=true;
		ret=0;
	}
	
	CoUninitialize();
	
	return ret;
}

int SuiteExtRel::Schedule(bool current_user, wchar_t** argv, int argc)
{
	int ret;
	bool na;
	
	std::wstring params;
	while (argc--) {
		if (!params.empty()) params.push_back(L' ');
		params.append(QuoteArgument(*argv++));
	}
	
	ret=Schedule20(na, current_user, params.c_str());
	if (na) {
		if (!current_user) {
			//In Task Scheduler 1.0 you can't run task for any logged on user by using BUILTIN\Users group as user like in 2.0
			ErrorMessage(L"Task Scheduler 1.0 doesn't support scheduling GUI apps for all users!");
			return ERR_SUITEEXTREL+1;
		} else if ((ret=Schedule10(na, params.c_str()), na)) {
			ErrorMessage(L"Task Scheduler 1.0 not available!");
			return ERR_SUITEEXTREL+1;
		}
	}
	if (ret) ErrorMessage(L"Error scheduling SnK HotkeySuite! Make sure that you have Admin rights before scheduling a task.");
	
	return ret;
}

int SuiteExtRel::Schedule10(bool &na, const wchar_t* params)
{
	int ret=ERR_SUITEEXTREL+2;
	na=false;
	
	CoInitialize(NULL);
	
#ifdef _WIN64
	std::wstring tname(L"SnK HotkeySuite (x64) [");
#else
	std::wstring tname(L"SnK HotkeySuite [");
#endif	
	std::wstring uname(GetUserNameForTaskScheduler(tname));
	if (!uname.empty()) {
		tname.push_back(L']');
		ITaskScheduler *pITS;
		if (SUCCEEDED(CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pITS))) {
#ifdef DEBUG
			std::wcerr<<L"SCHEDULE: Task Scheduler 1.0 available"<<std::endl;
#endif
			ITask *pITask;
			//ITask->Delete w/ ITask->NewWorkItem: if task exists - we are silently recreating it
			pITS->Delete(tname.c_str());
			if (SUCCEEDED(pITS->NewWorkItem(tname.c_str(), CLSID_CTask, IID_ITask, (IUnknown**)&pITask))) {
				ITaskTrigger *pITaskTrigger;
				WORD piNewTrigger;
				if (SUCCEEDED(pITask->SetApplicationName(GetExecutableFileName().c_str()))&&
					SUCCEEDED(pITask->SetWorkingDirectory(GetExecutableFileName(L"").c_str()))&&
					SUCCEEDED(pITask->SetParameters(params))&&
					SUCCEEDED(pITask->SetMaxRunTime(INFINITE))&&
					SUCCEEDED(pITask->SetFlags(TASK_FLAG_RUN_ONLY_IF_LOGGED_ON))&&		//TASK_FLAG_RUN_ONLY_IF_LOGGED_ON required if pwszPassword in SetAccountInformation is NULL
					SUCCEEDED(pITask->SetAccountInformation(uname.c_str(), NULL))&&		//Required
					SUCCEEDED(pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger))) {
					TASK_TRIGGER trigger={};
					trigger.wBeginDay=1;		//Required
					trigger.wBeginMonth=1;		//Required
					trigger.wBeginYear=1970;	//Required
					trigger.cbTriggerSize=sizeof(TASK_TRIGGER);
					trigger.TriggerType=TASK_EVENT_TRIGGER_AT_LOGON;
					
					if (SUCCEEDED(pITaskTrigger->SetTrigger(&trigger))) {
						IPersistFile *pIPersistFile;
						if (SUCCEEDED(pITask->QueryInterface(IID_PPV_ARGS(&pIPersistFile)))) {
							if (SUCCEEDED(pIPersistFile->Save(NULL, TRUE))) ret=0;
							pIPersistFile->Release();
						}
					}
					pITaskTrigger->Release();
				}
				pITask->Release();
			}
			pITS->Release();
		} else {
			na=true;
			ret=0;
		}
	}
	
	CoUninitialize();
	
	return ret;
}

int SuiteExtRel::Schedule20(bool &na, bool current_user, const wchar_t* params)
{
	int ret=ERR_SUITEEXTREL+2;
	na=false;
	
	CoInitialize(NULL);	//Same as CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), whatever, we don't need COINIT_MULTITHREADED
	
#ifdef _WIN64
	std::wstring tname(L"SnK HotkeySuite (x64)");
#else
	std::wstring tname(L"SnK HotkeySuite");
#endif
	std::wstring uname;	
	if (current_user) {
		tname.append({L' ', L'['});
		uname=GetUserNameForTaskScheduler(tname);

		if (uname.empty()) {
			CoUninitialize();
			return ret;
		}

		tname.push_back(L']');
	} else {
		uname=L"Users";
	}
	
	ITaskService *pService;
	if (SUCCEEDED(CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService))) {
#ifdef DEBUG
		std::wcerr<<L"SCHEDULE: Task Scheduler 2.0 available"<<std::endl;
#endif
		ITaskDefinition *pTask;
		VARIANT empty_var={VT_EMPTY};
		if (SUCCEEDED(pService->Connect(empty_var, empty_var, empty_var, empty_var))&&
			SUCCEEDED(pService->NewTask(0, &pTask))) {
			IPrincipal *pPrincipal;
			if (SUCCEEDED(pTask->get_Principal(&pPrincipal))) {
				if (BSTR uname_bstr=SysAllocString(uname.c_str())) {
					ITaskSettings *pTaskSettings;
					if (SUCCEEDED(pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST))&&
						(current_user?true:SUCCEEDED(pPrincipal->put_GroupId(uname_bstr)))&&
						SUCCEEDED(pTask->get_Settings(&pTaskSettings))) {
						if (BSTR infinite_bstr=SysAllocString(L"PT0S")) {
							ITriggerCollection *pTriggerCollection;
							if (SUCCEEDED(pTaskSettings->put_ExecutionTimeLimit(infinite_bstr))&&
								SUCCEEDED(pTaskSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE))&&
								SUCCEEDED(pTaskSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE))&&
								SUCCEEDED(pTask->get_Triggers(&pTriggerCollection))) {
								ITrigger *pTrigger;
								if (SUCCEEDED(pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger))) {
									auto ChangeLogonCreds=[pTrigger, uname_bstr](){
										bool ret=false;
										ILogonTrigger *pLogonTrigger;       
										if (SUCCEEDED(pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger))) {
											if (SUCCEEDED(pLogonTrigger->put_UserId(uname_bstr))) ret=true;
											pLogonTrigger->Release();
										}
										return ret;
									};
									IActionCollection *pActionCollection;
									if ((current_user?ChangeLogonCreds():true)&&
										SUCCEEDED(pTask->get_Actions(&pActionCollection))) {
										IAction *pAction;
										if (SUCCEEDED(pActionCollection->Create(TASK_ACTION_EXEC, &pAction))) {
											IExecAction *pExecAction;
											if (SUCCEEDED(pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction))) {
												if (BSTR hspath_bstr=SysAllocString(GetExecutableFileName().c_str())) {
													if (BSTR hsdir_bstr=SysAllocString(GetExecutableFileName(L"").c_str())) {
														if (BSTR params_bstr=SysAllocString(params)) {
															BSTR root_bstr;
															if (SUCCEEDED(pExecAction->put_Path(hspath_bstr))&&
																SUCCEEDED(pExecAction->put_WorkingDirectory(hsdir_bstr))&&
																SUCCEEDED(pExecAction->put_Arguments(params_bstr))&&
																(root_bstr=SysAllocString(L"\\"))) {
																ITaskFolder *pRootFolder;
																if (SUCCEEDED(pService->GetFolder(root_bstr, &pRootFolder))) {
																	if (BSTR tname_bstr=SysAllocString(tname.c_str())) {
																		IRegisteredTask *pRegisteredTask;
																		//ITaskService->NewTask w/ TASK_CREATE_OR_UPDATE: if task exists - we are silently recreating it
																		if (SUCCEEDED(pRootFolder->RegisterTaskDefinition(tname_bstr, pTask, TASK_CREATE_OR_UPDATE, empty_var, empty_var, TASK_LOGON_INTERACTIVE_TOKEN, empty_var, &pRegisteredTask))) {
																			ret=0;
																			pRegisteredTask->Release();
																		}
																		SysFreeString(tname_bstr);
																	}
																	pRootFolder->Release();
																}
																SysFreeString(root_bstr);
															}
															SysFreeString(params_bstr);
														}
														SysFreeString(hsdir_bstr);
													}
													SysFreeString(hspath_bstr);
												}
												pExecAction->Release();
											}
											pAction->Release();
										}
										pActionCollection->Release();
									}
									pTrigger->Release();
								}
								pTriggerCollection->Release();
							}
							SysFreeString(infinite_bstr);
						}
						pTaskSettings->Release();
					}
					SysFreeString(uname_bstr);
				}
				pPrincipal->Release();
			}
			pTask->Release();
		}
		pService->Release();
	} else {
		na=true;
		ret=0;
	}
	
	CoUninitialize();
	
	return ret;
}

bool SuiteExtRel::EnvQueryValue(HKEY reg_key, const wchar_t* key_name, std::wstring &key_value, DWORD &key_type)
{
	DWORD buf_len;
	key_type=REG_EXPAND_SZ;
	key_value=L"";
	
	//If key not found - return empty string
	//If other error - it's a fail
	LONG res=RegQueryValueEx(reg_key, key_name, NULL, NULL, NULL, &buf_len);
	if (res==ERROR_FILE_NOT_FOUND)
		return true;
	else if (res!=ERROR_SUCCESS)
		return false;
	
	//Returned buffer length is in bytes and because we use unicode build actual returned buffer type is wchar_t
	//+1 is for possible missing NULL-terminator
	wchar_t data_buf[(buf_len+1)/sizeof(wchar_t)];
	
	//If key not found - return empty string
	//If for some other reason we get read error or key of unsuitable type - return false
	res=RegQueryValueEx(reg_key, key_name, NULL, &key_type, (LPBYTE)data_buf, &buf_len);
	if (res==ERROR_FILE_NOT_FOUND)
		return true;
	else if (res!=ERROR_SUCCESS||(key_type!=REG_EXPAND_SZ&&key_type!=REG_SZ))
		return false;
	
	//Make sure that buffer is terminated
	if (!buf_len||data_buf[buf_len/sizeof(wchar_t)-1]!=L'\0') data_buf[buf_len/sizeof(wchar_t)]=L'\0';
	
	key_value=data_buf;
	return true;
}

size_t SuiteExtRel::FindInPath(const std::wstring &path, const wchar_t* dir, size_t *ret_len)
{
	//Function correctly matches directories in environment Path variable including preceding (if last dir in Path) and trailing (if not last dir) semicolons
	
	size_t dir_len=wcslen(dir);
	if (dir[dir_len-1]==L'\\') dir_len--;	//Omit trailing backslash
	
	size_t dir_pos=0;
	while ((dir_pos=path.find(dir, dir_pos, dir_len))!=std::wstring::npos) {
		std::wstring tail=path.substr(dir_pos+dir_len, 2);
		
		if (dir_pos&&path[dir_pos-1]!=L';')	{	//Not a vaild directory (not at the biginning and doesn't have preceding semicolon)
			dir_pos++;
			continue;
		} else if (tail.empty()) {				//Directory found at the end of Path or spans whole variable
			if (dir_pos) {						//If at the end - add preceding semicolon
				dir_pos--;
				dir_len++;
			}
		} else if (tail==L"\\")	{				//Directory w/ backslash found at the end of Path or spans whole variable
			if (dir_pos) {						//If at the end - add preceding semicolon
				dir_pos--;
				dir_len+=2;
			} else
				dir_len++;						//Just add trailing backslash
		} else if (tail[0]==L';') {				//Directory found somewhere in the Path
			dir_len++;							//Add trailing semicolon
		} else if (tail==L"\\;") {				//Directory w/ backslash found somewhere in the Path
			dir_len+=2;							//Add trailing semicolon w/ backslash
		} else {
			dir_pos++;
			continue;
		}
		
		if (ret_len) *ret_len=dir_len;
		return dir_pos;
	}
	
	return std::wstring::npos;
}

int SuiteExtRel::AddToPath(bool current_user)
{
	int ret=ERR_SUITEEXTREL+8;
	
	HKEY reg_key;
	//There is a no way that Environment key doesn't exist but, whatever, do like in AddToAutorun
	//RegCreateKeyEx will just open the key if it already exists or create one otherwise
	if (RegCreateKeyEx(current_user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, current_user?L"Environment":L"System\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, NULL)==ERROR_SUCCESS) {
		DWORD key_type;
		std::wstring prev_path;
		
		if (EnvQueryValue(reg_key, L"Path", prev_path, key_type)) {
			std::wstring hs_dir=GetExecutableFileName(L"");
			if (FindInPath(prev_path, hs_dir.c_str(), NULL)!=std::wstring::npos) {
				//Value already added to the path
				ret=0;
			} else {
				if (prev_path.length()) {
					hs_dir.append({L';'});
					hs_dir.append(prev_path);
				}
				
				if (RegSetValueEx(reg_key, L"Path", 0, key_type, (BYTE*)hs_dir.c_str(), (hs_dir.length()+1)*sizeof(wchar_t))==ERROR_SUCCESS) {
					SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, (WPARAM)NULL, (LPARAM)L"Environment", SMTO_NORMAL, 100, NULL);
					ret=0;
				}
			}
		}
		
		RegCloseKey(reg_key);
	}
	
	if (ret) ErrorMessage(L"Error adding SnK HotkeySuite directory to PATH! Make sure that you have enough rights to access registry.");

	return ret;
}

int SuiteExtRel::RemoveFromPath(bool current_user)
{
	int ret=ERR_SUITEEXTREL+7;
	
	HKEY reg_key;
	LONG res=RegOpenKeyEx(current_user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, current_user?L"Environment":L"System\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_SET_VALUE|KEY_QUERY_VALUE, &reg_key);
	if (res==ERROR_SUCCESS) {
		DWORD key_type;
		std::wstring prev_path;
		
		if (EnvQueryValue(reg_key, L"Path", prev_path, key_type)) {
			std::wstring hs_dir=GetExecutableFileName(L"");
			size_t len, pos;
			
			while ((pos=FindInPath(prev_path, hs_dir.c_str(), &len))!=std::wstring::npos)
				prev_path.erase(pos, len);
			
			if (prev_path.empty())	//If variable is empty now - just delete it
				res=RegDeleteValue(reg_key, L"Path");
			else
				res=RegSetValueEx(reg_key, L"Path", 0, key_type, (BYTE*)prev_path.c_str(), (prev_path.length()+1)*sizeof(wchar_t));
			
			//If value doesn't exist - ignore it, but don't ignore other errors
			if (res==ERROR_SUCCESS||res==ERROR_FILE_NOT_FOUND) {
				SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, (WPARAM)NULL, (LPARAM)L"Environment", SMTO_NORMAL, 100, NULL);
				ret=0;
			}
		}
		
		RegCloseKey(reg_key);
	} else if (res==ERROR_FILE_NOT_FOUND) {
		//No Environment key is ok too - nothing to delete
		ret=0;
	}
	
	if (ret) ErrorMessage(L"Error clearing SnK HotkeySuite directory from PATH! Make sure that you have enough rights to access registry.");

	return ret;
}

void SuiteExtRel::RestartApplication(const wchar_t* cmdline, bool elevate)
{
	//RestartApplication expects cmdline to point to actual string (empty string is ok) w/o executable path
	//Just command line arguments as passed to WinMain

	std::wstring path=GetExecutableFileName();
	SHELLEXECUTEINFO sei={sizeof(sei)}; 
	sei.lpVerb=elevate?L"runas":L"open"; 
	sei.lpFile=path.c_str(); 
	sei.lpParameters=cmdline; 
	sei.hwnd=NULL; 
	sei.nShow=SW_NORMAL;
	
	if (!ShellExecuteEx(&sei)) {
		if (elevate) {
			if (GetLastError()==ERROR_CANCELLED) { 
				sei.lpVerb=L"open";
				if (ShellExecuteEx(&sei)) return;
			} else { 
				ErrorMessage(L"Failed to restart application as administrator!");
				return;
			}
		}
		ErrorMessage(L"Failed to restart application!");
	}
}

void SuiteExtRel::LaunchCommandPrompt(const wchar_t* dir)
{
	if (DWORD env_len=GetEnvironmentVariable(L"ComSpec", NULL, 0)) {
		wchar_t env_buf[env_len];
		if (GetEnvironmentVariable(L"ComSpec", env_buf, env_len)) {
			std::wstring cd_cmd=L"/s /k title SnK Shell && set Path=";
			cd_cmd.append(QuoteArgument(dir));
			cd_cmd.append(L";%Path% && pushd ");
			cd_cmd.append(QuoteArgument(dir));
			SHELLEXECUTEINFO sei={sizeof(sei)}; 
			sei.lpVerb=L"open"; 
			sei.lpFile=env_buf; 
			sei.lpParameters=cd_cmd.c_str(); 
			sei.hwnd=NULL; 
			sei.nShow=SW_NORMAL;
			ShellExecuteEx(&sei);
		}
	}
}

int SuiteExtRel::FireEvent(bool long_press, SuiteSettings *settings)
{
	std::wstring snk_path=settings->GetSnkPath();
	DWORD dwAttrib=GetFileAttributes(snk_path.c_str());
	if (dwAttrib==INVALID_FILE_ATTRIBUTES||(dwAttrib&FILE_ATTRIBUTE_DIRECTORY)) {
		ErrorMessage(L"Path to SnK is not valid!");
		return ERR_SUITEEXTREL+9;
	}
	
	std::wstring snk_cmdline=QuoteArgument(snk_path.c_str());
	snk_cmdline.append(L" /sec /bpp +p /cmd=");
	if (long_press)
		snk_cmdline.append(QuoteArgument(settings->GetLhkCfgPath().c_str()));
	else
		snk_cmdline.append(QuoteArgument(settings->GetShkCfgPath().c_str()));
	
	PROCESS_INFORMATION pi={};
	STARTUPINFO si={sizeof(STARTUPINFO), NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, STARTF_USESHOWWINDOW, SW_SHOWNORMAL};
	if (CreateProcess(NULL, const_cast<wchar_t*>(snk_cmdline.c_str()), NULL, NULL, FALSE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return 0;
	}
	
	ErrorMessage(L"Failed to launch SnK!");
	return ERR_SUITEEXTREL+10;	
}
