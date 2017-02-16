#include "SuiteExternalRelations.h"
#include "SuiteCommon.h"
#include <string>
#include <windows.h>
#include <initguid.h>
#include <mstask.h>
#include <taskschd.h>
#include <comutil.h>
#include <lmcons.h>
#include "taskschd_hs.h"

#ifdef DEBUG
#include <iostream>
#endif

namespace SuiteExtRel {
	int Schedule10(bool current_user);
	int Schedule20(bool &na, bool current_user);
}

int SuiteExtRel::Schedule(bool current_user)
{
	int ret;
	bool na;
	
	ret=Schedule20(na, current_user);
	if (na) ret=Schedule10(current_user);
	if (ret) ErrorMessage(L"Error scheduling SnK HotkeySuite!");
	
	return ret;
}

int SuiteExtRel::Schedule10(bool current_user)
{
	if (!current_user) return ERR_SUITEEXTREL+3;	//In Task Scheduler 1.0 you can't run task for any logged on user by using BUILTIN\Users group as user like in 2.0
	
	int ret=ERR_SUITEEXTREL+1;
	
	CoInitialize(NULL);
	
	wchar_t uname[UNLEN+1];
	DWORD uname_len=UNLEN+1;
	GetUserName(uname, &uname_len);
	std::wstring tname(L"SnK HotkeySuite [");
	tname.append(uname);
	tname.append({L']'});
	
	ITaskScheduler *pITS;
	if (SUCCEEDED(CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pITS))) {
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
	}
	
	CoUninitialize();
	
	return ret;
}

int SuiteExtRel::Schedule20(bool &na, bool current_user)
{
	int ret=ERR_SUITEEXTREL+2;
	na=false;
	
	CoInitialize(NULL);
	
	ITaskService *pService;
	if (SUCCEEDED(CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService))) {
#ifdef DEBUG
		std::wcerr<<L"SCHEDULE: Task Scheduler 2.0 available"<<std::endl;
#endif
		ITaskDefinition *pTask;
		if (SUCCEEDED(pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()))&&
			SUCCEEDED(pService->NewTask(0, &pTask))) {
			ret=0;	
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