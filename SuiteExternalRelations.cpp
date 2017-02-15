#include "SuiteExternalRelations.h"
#include <windows.h>
#include <mstask.h>

int SnkExtRel::Schedule(bool current_user)
{
	CoInitialize(NULL);
	
	ITaskScheduler *pITS;
	if (SUCCEEDED(CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pITS)))) {
		ITask *pITask;
		if (SUCCEEDED(pITS->NewWorkItem(L"SnK HotkeySuite", CLSID_CTask, IID_ITask, (IUnknown**)&pITask))) {
			IPersistFile *pIPersistFile;
			if (SUCCEEDED(pITask->QueryInterface(IID_PPV_ARGS(&pIPersistFile)))) {
				pIPersistFile->Save(NULL, TRUE);
				pIPersistFile->Release();
			}
			pITask->Release();
		}
		pITS->Release();
	}
	
	CoUninitialize();
	
	return 0;
}