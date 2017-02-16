#include "SuiteExternalRelations.h"
#include "SuiteCommon.h"
#include <windows.h>
#include <initguid.h>
#include <mstask.h>
#include <taskschd.h>
#include <comutil.h>
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
	int ret=ERR_SUITEEXTREL+1;
	
	CoInitialize(NULL);
	
	ITaskScheduler *pITS;
	if (SUCCEEDED(CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pITS))) {
		ITask *pITask;
		if (SUCCEEDED(pITS->NewWorkItem(L"SnK HotkeySuite", CLSID_CTask, IID_ITask, (IUnknown**)&pITask))) {
			IPersistFile *pIPersistFile;
			if (SUCCEEDED(pITask->QueryInterface(IID_PPV_ARGS(&pIPersistFile)))) {
				pIPersistFile->Save(NULL, TRUE);
				ret=0;
				pIPersistFile->Release();
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