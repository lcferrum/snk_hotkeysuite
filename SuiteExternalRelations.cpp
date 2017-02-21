#include "SuiteExternalRelations.h"
#include "SuiteExterns.h"
#include "SuiteCommon.h"
#include <string>
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
	bool GetUserNameWrapper(std::wstring &sname, std::wstring &fname);
	std::wstring QuoteArgument(const wchar_t* arg);
}

//Based on "Everyone quotes command line arguments the wrong way": 
// Written by Daniel Colascione <dancol@dancol.org>
// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
std::wstring SuiteExtRel::QuoteArgument(const wchar_t* arg)
{
	if (!arg||!wcslen(arg)) {
		return L"\"\"";
	} else if (!wcspbrk(arg, L" \t\n\v\"")) {
		return arg;
	} else {
		std::wstring qarg=L"\"";
		for (;;) {
			size_t backslash_num=0;

			while (*arg!=L'\0'&&*arg==L'\\') {
				arg++;
				backslash_num++;
			}

			if (*arg==L'\0') {
				qarg.append(backslash_num*2, L'\\');
				break;
			} else if (*arg==L'"') {
				qarg.append(backslash_num*2+1, L'\\');
				qarg.push_back(*arg);
			} else {
				qarg.append(backslash_num, L'\\');
				qarg.push_back(*arg);
			}
			arg++;
		}
		qarg.push_back(L'"');
		return qarg;
	}
}

int SuiteExtRel::AddToAutorun(bool current_user, wchar_t** argv, int argc)
{
	int ret=ERR_SUITEEXTREL+6;
	
	HKEY reg_key;
	if (RegOpenKeyEx(current_user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &reg_key)==ERROR_SUCCESS) {
		std::wstring hs_path=QuoteArgument(GetExecutableFileName().c_str());
		while (argc--) {
			hs_path.push_back(L' ');
			hs_path.append(QuoteArgument(*argv++));
		}
		//RegSetValueEx will create absent value or overwrite present value (even if present value have different type)
		if (RegSetValueEx(reg_key, L"SnK HotkeySuite", 0, REG_SZ, (BYTE*)hs_path.c_str(), (hs_path.length()+1)*sizeof(wchar_t))==ERROR_SUCCESS) ret=0;
		RegCloseKey(reg_key);
	}
	
	if (ret) ErrorMessage(L"Error adding SnK HotkeySuite to Autorun! Make sure that you have enough rights to access registry.");

	return ret;
}

int SuiteExtRel::RemoveFromAutorun(bool current_user)
{
	int ret=ERR_SUITEEXTREL+5;
	
	HKEY reg_key;
	if (RegOpenKeyEx(current_user?HKEY_CURRENT_USER:HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &reg_key)==ERROR_SUCCESS) {
		LONG res=RegDeleteValue(reg_key, L"SnK HotkeySuite");
		//If value doesn't exists - ignore it, but don't ignore other errors
		if (res==ERROR_SUCCESS||res==ERROR_FILE_NOT_FOUND) ret=0;
		RegCloseKey(reg_key);
	}
	
	if (ret) ErrorMessage(L"Error removing SnK HotkeySuite from Autorun! Make sure that you have enough rights to access registry.");

	return ret;
}

bool SuiteExtRel::GetUserNameWrapper(std::wstring &sname, std::wstring &fname)
{
	wchar_t uname[UNLEN+1];
	DWORD uname_len=UNLEN+1;
	
	if (!GetUserName(uname, &uname_len))
		return false;
	
	sname=uname;
	if (fnGetUserNameEx) {
		uname_len=UNLEN+1;
		if (fnGetUserNameEx(NameSamCompatible, uname, &uname_len)) {
			fname=uname;
		} else if (GetLastError()==ERROR_MORE_DATA) {
			wchar_t uname_ex[uname_len];
			if (fnGetUserNameEx(NameSamCompatible, uname_ex, &uname_len))
				fname=uname_ex;
			else
				return false;
		} else
			return false;
	} else {
		fname=uname;
	}

	return true;
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
	
	wchar_t uname[UNLEN+1];
	DWORD uname_len=UNLEN+1;
	if (!GetUserName(uname, &uname_len))
		CoUninitialize();
	std::wstring tname(L"SnK HotkeySuite [");
	tname.append(uname);
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
	
	CoUninitialize();
	
	return ret;
}

int SuiteExtRel::Unschedule20(bool &na, bool current_user)
{
	int ret=ERR_SUITEEXTREL+4;
	na=false;
	
	CoInitialize(NULL);	//Same as CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), whatever, we don't need COINIT_MULTITHREADED
	
	std::wstring tname(L"SnK HotkeySuite");
	if (current_user) {
		wchar_t uname[UNLEN+1];
		DWORD uname_len=UNLEN+1;
		if (!GetUserName(uname, &uname_len))
			CoUninitialize();
		tname.append({L' ', L'['});
		tname.append(uname);
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
	
	wchar_t uname[UNLEN+1];
	DWORD uname_len=UNLEN+1;
	if (!GetUserName(uname, &uname_len))
		CoUninitialize();
	std::wstring tname(L"SnK HotkeySuite [");
	tname.append(uname);
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
				SUCCEEDED(pITask->SetAccountInformation(uname, NULL))&&				//Required
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
	
	CoUninitialize();
	
	return ret;
}

int SuiteExtRel::Schedule20(bool &na, bool current_user, const wchar_t* params)
{
	int ret=ERR_SUITEEXTREL+2;
	na=false;
	
	CoInitialize(NULL);	//Same as CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), whatever, we don't need COINIT_MULTITHREADED
	
	std::wstring tname(L"SnK HotkeySuite");
	std::wstring uname;	
	if (current_user) {		
		std::wstring sname;
		if (!GetUserNameWrapper(sname, uname))
			CoUninitialize();
		tname.append({L' ', L'['});
		tname.append(sname);
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