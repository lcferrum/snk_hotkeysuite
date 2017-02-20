#include "SuiteExternalRelations.h"
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

namespace SuiteExtRel {
	int Schedule10(bool &na);
	int Schedule20(bool &na, bool current_user);
}

int SuiteExtRel::Schedule(bool current_user)
{
	int ret;
	bool na;
	
	ret=Schedule20(na, current_user);
	if (na) {
		if (!current_user) {
			//In Task Scheduler 1.0 you can't run task for any logged on user by using BUILTIN\Users group as user like in 2.0
			ErrorMessage(L"Task Scheduler 1.0 doesn't support scheduling GUI apps for all users!");
			return ERR_SUITEEXTREL+3;
		} else if ((ret=Schedule10(na), na)) {
			ErrorMessage(L"Task Scheduler 1.0 not available!");
			return ERR_SUITEEXTREL+4;
		}
	}
	if (ret) ErrorMessage(L"Error scheduling SnK HotkeySuite!");
	
	return ret;
}

int SuiteExtRel::Schedule10(bool &na)
{
	int ret=ERR_SUITEEXTREL+1;
	na=false;
	
	CoInitialize(NULL);
	
	wchar_t uname[UNLEN+1];
	DWORD uname_len=UNLEN+1;
	GetUserName(uname, &uname_len);
	std::wstring tname(L"SnK HotkeySuite [");
	tname.append(uname);
	tname.append({L']'});
	
	ITaskScheduler *pITS;
	if (SUCCEEDED(CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pITS))) {
#ifdef DEBUG
		std::wcerr<<L"SCHEDULE: Task Scheduler 1.0 available"<<std::endl;
#endif
		ITask *pITask;
		if (SUCCEEDED(pITS->NewWorkItem(tname.c_str(), CLSID_CTask, IID_ITask, (IUnknown**)&pITask))) {
			ITaskTrigger *pITaskTrigger;
			WORD piNewTrigger;
			if (SUCCEEDED(pITask->SetApplicationName(GetExecutableFileName().c_str()))&&
				SUCCEEDED(pITask->SetWorkingDirectory(GetExecutableFileName(L"").c_str()))&&
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

int SuiteExtRel::Schedule20(bool &na, bool current_user)
{
	int ret=ERR_SUITEEXTREL+2;
	na=false;
	
	CoInitialize(NULL);	//Same as CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), whatever, we don't need COINIT_MULTITHREADED
	//CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_PKT_PRIVACY,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,0,NULL);
	
	std::wstring tname(L"SnK HotkeySuite");
	BSTR uname_bstr=NULL;	
	if (current_user) {
		wchar_t uname[UNLEN+1];
		DWORD uname_len=UNLEN+1;
		GetUserName(uname, &uname_len);
		tname.append({L' ', L'['});
		tname.append(uname);
		tname.append({L']'});
	} else {
		uname_bstr=SysAllocString(L"Users");
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
				ITaskSettings *pTaskSettings;
				if (SUCCEEDED(pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST))&&
					(current_user?true:(uname_bstr&&SUCCEEDED(pPrincipal->put_GroupId(uname_bstr))))&&
					SUCCEEDED(pTask->get_Settings(&pTaskSettings))) {
					if (BSTR infinite_bstr=SysAllocString(L"PT0S")) {
						ITriggerCollection *pTriggerCollection;
						if (SUCCEEDED(pTaskSettings->put_ExecutionTimeLimit(infinite_bstr))&&
							SUCCEEDED(pTaskSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE))&&
							SUCCEEDED(pTaskSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE))&&
							SUCCEEDED(pTask->get_Triggers(&pTriggerCollection))) {
							ITrigger *pTrigger;
							if (SUCCEEDED(pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger))) {
								IActionCollection *pActionCollection;
								if (SUCCEEDED(pTask->get_Actions(&pActionCollection))) {
									IAction *pAction;
									if (SUCCEEDED(pActionCollection->Create(TASK_ACTION_EXEC, &pAction))) {
										IExecAction *pExecAction;
										if (SUCCEEDED(pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction))) {
											if (BSTR hspath_bstr=SysAllocString(GetExecutableFileName().c_str())) {
												if (BSTR hsdir_bstr=SysAllocString(GetExecutableFileName(L"").c_str())) {
													BSTR root_bstr;
													if (SUCCEEDED(pExecAction->put_Path(hspath_bstr))&&
														SUCCEEDED(pExecAction->put_WorkingDirectory(hsdir_bstr))&&
														(root_bstr=SysAllocString(L"\\"))) {
														ITaskFolder *pRootFolder;
														if (SUCCEEDED(pService->GetFolder(root_bstr, &pRootFolder))) {
															if (BSTR tname_bstr=SysAllocString(tname.c_str())) {
																IRegisteredTask *pRegisteredTask;
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
				pPrincipal->Release();
			}
			pTask->Release();
		}
		pService->Release();
	} else {
		na=true;
		ret=0;
	}
	
	if (uname_bstr)
		SysFreeString(uname_bstr);
	
	CoUninitialize();
	
	return ret;
}