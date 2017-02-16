//This is collection of definitions for Task Scheduler 2.0 COM interface not found in MinGW's taskschd.h and libtaskschd.a

#ifndef TASKSCHD_HS_H
#define TASKSCHD_HS_H

#include <windows.h>
#include <ole2.h>
#include <taskschd.h>

//UUID                  {148BD527-A2AB-11CE-B11F-00AA00530503}
//                       148BD527  - A2AB  - 11CE  - B1    1F  - 00    AA    00    53    05    03
//DEFINE_GUID(IID/CLSID, 0x148BD527, 0xA2AB, 0x11CE, 0xB1, 0x1F, 0x00, 0xAA, 0x00, 0x53, 0x05, 0x03);

#ifndef __ITaskService_FWD_DEFINED__
#define __ITaskService_FWD_DEFINED__
typedef struct ITaskService ITaskService;
#endif

#ifndef __ITaskFolder_FWD_DEFINED__
#define __ITaskFolder_FWD_DEFINED__
typedef struct ITaskFolder ITaskFolder;
#endif

#ifndef __ITaskDefinition_FWD_DEFINED__
#define __ITaskDefinition_FWD_DEFINED__
typedef struct ITaskDefinition ITaskDefinition;
#endif

#ifndef __IRunningTask_FWD_DEFINED__
#define __IRunningTask_FWD_DEFINED__
typedef struct IRunningTask IRunningTask;
#endif

#ifndef __IRunningTaskCollection_FWD_DEFINED__
#define __IRunningTaskCollection_FWD_DEFINED__
typedef struct IRunningTaskCollection IRunningTaskCollection;
#endif

#ifndef __IRegisteredTask_FWD_DEFINED__
#define __IRegisteredTask_FWD_DEFINED__
typedef struct IRegisteredTask IRegisteredTask;
#endif

#ifndef __ITaskFolderCollection_FWD_DEFINED__
#define __ITaskFolderCollection_FWD_DEFINED__
typedef struct ITaskFolderCollection ITaskFolderCollection;
#endif

#ifndef __IRegisteredTaskCollection_FWD_DEFINED__
#define __IRegisteredTaskCollection_FWD_DEFINED__
typedef struct IRegisteredTaskCollection IRegisteredTaskCollection;
#endif


extern "C" {
	DEFINE_GUID(IID_ITaskService, 0x2faba4c7, 0x4da9, 0x4013, 0x96, 0x97, 0x20, 0xcc, 0x3f, 0xd4, 0x0f, 0x85);
#ifndef __ITaskService_INTERFACE_DEFINED__
#define __ITaskService_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskService;
	struct ITaskService: public IDispatch {
	public:
		virtual HRESULT WINAPI GetFolder(BSTR path, ITaskFolder **ppFolder)=0;
		virtual HRESULT WINAPI GetRunningTasks(LONG flags, IRunningTaskCollection **ppRunningTasks)=0;
		virtual HRESULT WINAPI NewTask(DWORD flags, ITaskDefinition **ppDefinition)=0;
		virtual HRESULT WINAPI Connect(VARIANT serverName, VARIANT user, VARIANT domain, VARIANT password)=0;
		virtual HRESULT WINAPI get_Connected(VARIANT_BOOL *pConnected)=0;
		virtual HRESULT WINAPI get_TargetServer(BSTR *pServer)=0;
		virtual HRESULT WINAPI get_ConnectedUser(BSTR *pUser)=0;
		virtual HRESULT WINAPI get_ConnectedDomain(BSTR *pDomain)=0;
		virtual HRESULT WINAPI get_HighestVersion(DWORD *pVersion)=0;
	};
#endif
	EXTERN_C const CLSID CLSID_TaskScheduler;
	DEFINE_GUID(CLSID_TaskScheduler, 0x0f87369f, 0xa4e5, 0x4cfc, 0xbd, 0x3e, 0x73, 0xe6, 0x15, 0x45, 0x72, 0xdd);
	
	DEFINE_GUID(IID_ITaskFolder, 0x8cfac062, 0xa080, 0x4c15, 0x9a, 0x88, 0xaa, 0x7c, 0x2a, 0xf8, 0x0d, 0xfc);
#ifndef __ITaskFolder_INTERFACE_DEFINED__
#define __ITaskFolder_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskFolder;
	struct ITaskFolder: public IDispatch {
	public:
		virtual HRESULT WINAPI get_Name(BSTR *pName)=0;
		virtual HRESULT WINAPI get_Path(BSTR *pPath)=0;
		virtual HRESULT WINAPI GetFolder(BSTR path, ITaskFolder **ppFolder)=0;
		virtual HRESULT WINAPI GetFolders(LONG flags, ITaskFolderCollection **ppFolders)=0;
		virtual HRESULT WINAPI CreateFolder(BSTR subFolderName, VARIANT sddl, ITaskFolder **ppFolder)=0;
		virtual HRESULT WINAPI DeleteFolder(BSTR subFolderName, LONG flags)=0;
		virtual HRESULT WINAPI GetTask(BSTR path, IRegisteredTask **ppTask)=0;
		virtual HRESULT WINAPI GetTasks(LONG flags, IRegisteredTaskCollection **ppTasks)=0;
		virtual HRESULT WINAPI DeleteTask(BSTR name, LONG flags)=0;
		virtual HRESULT WINAPI RegisterTask(BSTR path, BSTR xmlText, LONG flags, VARIANT userId, VARIANT password, TASK_LOGON_TYPE logonType, VARIANT sddl, IRegisteredTask **ppTask)=0;
		virtual HRESULT WINAPI RegisterTaskDefinition(BSTR path, ITaskDefinition *pDefinition, LONG flags, VARIANT userId, VARIANT password, TASK_LOGON_TYPE logonType, VARIANT sddl, IRegisteredTask **ppTask)=0;
		virtual HRESULT WINAPI GetSecurityDescriptor(LONG securityInformation, BSTR *pSddl)=0;
		virtual HRESULT WINAPI SetSecurityDescriptor(BSTR sddl, LONG flags)=0;
	};
#endif

	DEFINE_GUID(IID_ITaskDefinition, 0xf5bc8fc5, 0x536d, 0x4f77, 0xb8, 0x52, 0xfb, 0xc1, 0x35, 0x6f, 0xde, 0xb6);
#ifndef __ITaskDefinition_INTERFACE_DEFINED__
#define __ITaskDefinition_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskDefinition;
	struct ITaskDefinition: public IDispatch {
	public:
		//virtual HRESULT WINAPI get_RegistrationInfo(IRegistrationInfo **ppRegistrationInfo)=0;
		//virtual HRESULT WINAPI put_RegistrationInfo(IRegistrationInfo *pRegistrationInfo)=0;
		//virtual HRESULT WINAPI get_Triggers(ITriggerCollection **ppTriggers)=0;
		//virtual HRESULT WINAPI put_Triggers(ITriggerCollection *pTriggers)=0;
		//virtual HRESULT WINAPI get_Settings(ITaskSettings **ppSettings)=0;
		//virtual HRESULT WINAPI put_Settings(ITaskSettings *pSettings)=0;
		virtual HRESULT WINAPI get_Data(BSTR *pData)=0;
		virtual HRESULT WINAPI put_Data(BSTR data)=0;
		//virtual HRESULT WINAPI get_Principal(IPrincipal **ppPrincipal)=0;
		//virtual HRESULT WINAPI put_Principal(IPrincipal *pPrincipal)=0;
		//virtual HRESULT WINAPI get_Actions(IActionCollection **ppActions)=0;
		//virtual HRESULT WINAPI put_Actions(IActionCollection *pActions)=0;
		virtual HRESULT WINAPI get_XmlText(BSTR *pXml)=0;
		virtual HRESULT WINAPI put_XmlText(BSTR xml)=0;
	};
#endif

	DEFINE_GUID(IID_IRunningTaskCollection, 0x6a67614b, 0x6828, 0x4fec, 0xaa, 0x54, 0x6d, 0x52, 0xe8, 0xf1, 0xf2, 0xdb);
#ifndef __IRunningTaskCollection_INTERFACE_DEFINED__
#define __IRunningTaskCollection_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRunningTaskCollection;
	struct IRunningTaskCollection: public IDispatch {
	public:
		virtual HRESULT WINAPI get_Count(LONG *pCount)=0;
		virtual HRESULT WINAPI get_Item(VARIANT index, IRunningTask **ppRunningTask)=0;
		virtual HRESULT WINAPI get__NewEnum(IUnknown **ppEnum)=0;
	};
#endif

	DEFINE_GUID(IID_IRegisteredTask, 0x9c86f320, 0xdee3, 0x4dd1, 0xb9, 0x72, 0xa3, 0x03, 0xf2, 0x6b, 0x06, 0x1e);
#ifndef __IRegisteredTask_INTERFACE_DEFINED__
#define __IRegisteredTask_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRegisteredTask;
	struct IRegisteredTask: public IDispatch {
	public:
		virtual HRESULT WINAPI get_Name(BSTR *pName)=0;
		virtual HRESULT WINAPI get_Path(BSTR *pPath)=0;
		virtual HRESULT WINAPI get_State(TASK_STATE *pState)=0;
		virtual HRESULT WINAPI get_Enabled(VARIANT_BOOL *pEnabled)=0;
		virtual HRESULT WINAPI put_Enabled(VARIANT_BOOL enabled)=0;
		virtual HRESULT WINAPI Run(VARIANT params, IRunningTask **ppRunningTask)=0;
		virtual HRESULT WINAPI RunEx(VARIANT params, LONG flags, LONG sessionID, BSTR user, IRunningTask **ppRunningTask)=0;
		virtual HRESULT WINAPI GetInstances(LONG flags, IRunningTaskCollection **ppRunningTasks)=0;
		virtual HRESULT WINAPI get_LastRunTime(DATE *pLastRunTime)=0;
		virtual HRESULT WINAPI get_LastTaskResult(LONG *pLastTaskResult)=0;
		virtual HRESULT WINAPI get_NumberOfMissedRuns(LONG *pNumberOfMissedRuns)=0;
		virtual HRESULT WINAPI get_NextRunTime(DATE *pNextRunTime)=0;
		virtual HRESULT WINAPI get_Definition(ITaskDefinition **ppDefinition)=0;
		virtual HRESULT WINAPI get_Xml(BSTR *pXml)=0;
		virtual HRESULT WINAPI GetSecurityDescriptor(LONG securityInformation, BSTR *pSddl)=0;
		virtual HRESULT WINAPI SetSecurityDescriptor(BSTR sddl, LONG flags)=0;
		virtual HRESULT WINAPI Stop(LONG flags)=0;
		virtual HRESULT WINAPI GetRunTimes(const LPSYSTEMTIME pstStart, const LPSYSTEMTIME pstEnd, DWORD *pCount, LPSYSTEMTIME *pRunTimes)=0;
	};
#endif

	DEFINE_GUID(IID_IRunningTask, 0x653758fb, 0x7b9a, 0x4f1e, 0xa4, 0x71, 0xbe, 0xeb, 0x8e, 0x9b, 0x83, 0x4e);
#ifndef __IRunningTask_INTERFACE_DEFINED__
#define __IRunningTask_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRunningTask;
	struct IRunningTask: public IDispatch {
	public:
		virtual HRESULT WINAPI get_Name(BSTR *pName)=0;
		virtual HRESULT WINAPI get_InstanceGuid(BSTR *pGuid)=0;
		virtual HRESULT WINAPI get_Path(BSTR *pPath)=0;
		virtual HRESULT WINAPI get_State(TASK_STATE *pState)=0;
		virtual HRESULT WINAPI get_CurrentAction(BSTR *pName)=0;
		virtual HRESULT WINAPI Stop()=0;
		virtual HRESULT WINAPI Refresh()=0;
		virtual HRESULT WINAPI get_EnginePID(DWORD *pPID)=0;
	};
#endif

	DEFINE_GUID(IID_ITaskFolderCollection, 0x79184a66, 0x8664, 0x423f, 0x97, 0xf1, 0x63, 0x73, 0x56, 0xa5, 0xd8, 0x12);
#ifndef __ITaskFolderCollection_INTERFACE_DEFINED__
#define __ITaskFolderCollection_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskFolderCollection;
	struct ITaskFolderCollection: public IDispatch {
	public:
		virtual HRESULT WINAPI get_Count(LONG *pCount)=0;
		virtual HRESULT WINAPI get_Item(VARIANT index, ITaskFolder **ppFolder)=0;
		virtual HRESULT WINAPI get__NewEnum(IUnknown **ppEnum)=0;
	};
#endif

	DEFINE_GUID(IID_IRegisteredTaskCollection, 0x86627eb4, 0x42a7, 0x41e4, 0xa4, 0xd9, 0xac, 0x33, 0xa7, 0x2f, 0x2d, 0x52);
#ifndef __IRegisteredTaskCollection_INTERFACE_DEFINED__
#define __IRegisteredTaskCollection_INTERFACE_DEFINED__
	struct IRegisteredTaskCollection: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Count(LONG *pCount)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Item(VARIANT index, IRegisteredTask **ppRegisteredTask)=0;
		virtual HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown **ppEnum)=0;
	};
#endif
}

#endif //TASKSCHD_HS_H
