//This is collection of definitions for Task Scheduler 2.0 COM interface not found in MinGW's taskschd.h and libtaskschd.a
//Some interfaces may be missing because they are not used by HotkeySuite

#ifndef TASKSCHD_HS_H
#define TASKSCHD_HS_H

#include <windows.h>
#include <ole2.h>
#include <taskschd.h>

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

#ifndef __IRegistrationInfo_FWD_DEFINED__
#define __IRegistrationInfo_FWD_DEFINED__
typedef struct IRegistrationInfo IRegistrationInfo;
#endif

#ifndef __ITriggerCollection_FWD_DEFINED__
#define __ITriggerCollection_FWD_DEFINED__
typedef struct ITriggerCollection ITriggerCollection;
#endif

#ifndef __ITrigger_FWD_DEFINED__
#define __ITrigger_FWD_DEFINED__
typedef struct ITrigger ITrigger;
#endif

#ifndef __IRepetitionPattern_FWD_DEFINED__
#define __IRepetitionPattern_FWD_DEFINED__
typedef struct IRepetitionPattern IRepetitionPattern;
#endif

#ifndef __ITaskSettings_FWD_DEFINED__
#define __ITaskSettings_FWD_DEFINED__
typedef struct ITaskSettings ITaskSettings;
#endif

#ifndef __IIdleSettings_FWD_DEFINED__
#define __IIdleSettings_FWD_DEFINED__
typedef struct IIdleSettings IIdleSettings;
#endif

#ifndef __INetworkSettings_FWD_DEFINED__
#define __INetworkSettings_FWD_DEFINED__
typedef struct INetworkSettings INetworkSettings;
#endif

#ifndef __IPrincipal_FWD_DEFINED__
#define __IPrincipal_FWD_DEFINED__
typedef struct IPrincipal IPrincipal;
#endif 

#ifndef __IActionCollection_FWD_DEFINED__
#define __IActionCollection_FWD_DEFINED__
typedef struct IActionCollection IActionCollection;
#endif

#ifndef __IAction_FWD_DEFINED__
#define __IAction_FWD_DEFINED__
typedef struct IAction IAction;
#endif

#ifndef __ILogonTrigger_FWD_DEFINED__
#define __ILogonTrigger_FWD_DEFINED__
typedef struct ILogonTrigger ILogonTrigger;
#endif

#ifndef __IExecAction_FWD_DEFINED__
#define __IExecAction_FWD_DEFINED__
typedef struct IExecAction IExecAction;
#endif

extern "C" {
	typedef enum _TASK_ACTION_TYPE {
		TASK_ACTION_EXEC=0,
		TASK_ACTION_COM_HANDLER=5,
		TASK_ACTION_SEND_EMAIL=6,
		TASK_ACTION_SHOW_MESSAGE=7
	} TASK_ACTION_TYPE;

	EXTERN_C const CLSID CLSID_TaskScheduler;
	DEFINE_GUID(CLSID_TaskScheduler, 0x0f87369f, 0xa4e5, 0x4cfc, 0xbd, 0x3e, 0x73, 0xe6, 0x15, 0x45, 0x72, 0xdd);

	DEFINE_GUID(IID_ITaskService, 0x2faba4c7, 0x4da9, 0x4013, 0x96, 0x97, 0x20, 0xcc, 0x3f, 0xd4, 0x0f, 0x85);
#ifndef __ITaskService_INTERFACE_DEFINED__
#define __ITaskService_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskService;
	struct ITaskService: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE GetFolder(BSTR path, ITaskFolder **ppFolder)=0;
		virtual HRESULT STDMETHODCALLTYPE GetRunningTasks(LONG flags, IRunningTaskCollection **ppRunningTasks)=0;
		virtual HRESULT STDMETHODCALLTYPE NewTask(DWORD flags, ITaskDefinition **ppDefinition)=0;
		virtual HRESULT STDMETHODCALLTYPE Connect(VARIANT serverName, VARIANT user, VARIANT domain, VARIANT password)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Connected(VARIANT_BOOL *pConnected)=0;
		virtual HRESULT STDMETHODCALLTYPE get_TargetServer(BSTR *pServer)=0;
		virtual HRESULT STDMETHODCALLTYPE get_ConnectedUser(BSTR *pUser)=0;
		virtual HRESULT STDMETHODCALLTYPE get_ConnectedDomain(BSTR *pDomain)=0;
		virtual HRESULT STDMETHODCALLTYPE get_HighestVersion(DWORD *pVersion)=0;
	};
#endif
	
	DEFINE_GUID(IID_ITaskFolder, 0x8cfac062, 0xa080, 0x4c15, 0x9a, 0x88, 0xaa, 0x7c, 0x2a, 0xf8, 0x0d, 0xfc);
#ifndef __ITaskFolder_INTERFACE_DEFINED__
#define __ITaskFolder_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskFolder;
	struct ITaskFolder: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Name(BSTR *pName)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Path(BSTR *pPath)=0;
		virtual HRESULT STDMETHODCALLTYPE GetFolder(BSTR path, ITaskFolder **ppFolder)=0;
		virtual HRESULT STDMETHODCALLTYPE GetFolders(LONG flags, ITaskFolderCollection **ppFolders)=0;
		virtual HRESULT STDMETHODCALLTYPE CreateFolder(BSTR subFolderName, VARIANT sddl, ITaskFolder **ppFolder)=0;
		virtual HRESULT STDMETHODCALLTYPE DeleteFolder(BSTR subFolderName, LONG flags)=0;
		virtual HRESULT STDMETHODCALLTYPE GetTask(BSTR path, IRegisteredTask **ppTask)=0;
		virtual HRESULT STDMETHODCALLTYPE GetTasks(LONG flags, IRegisteredTaskCollection **ppTasks)=0;
		virtual HRESULT STDMETHODCALLTYPE DeleteTask(BSTR name, LONG flags)=0;
		virtual HRESULT STDMETHODCALLTYPE RegisterTask(BSTR path, BSTR xmlText, LONG flags, VARIANT userId, VARIANT password, TASK_LOGON_TYPE logonType, VARIANT sddl, IRegisteredTask **ppTask)=0;
		virtual HRESULT STDMETHODCALLTYPE RegisterTaskDefinition(BSTR path, ITaskDefinition *pDefinition, LONG flags, VARIANT userId, VARIANT password, TASK_LOGON_TYPE logonType, VARIANT sddl, IRegisteredTask **ppTask)=0;
		virtual HRESULT STDMETHODCALLTYPE GetSecurityDescriptor(LONG securityInformation, BSTR *pSddl)=0;
		virtual HRESULT STDMETHODCALLTYPE SetSecurityDescriptor(BSTR sddl, LONG flags)=0;
	};
#endif

	DEFINE_GUID(IID_ITaskDefinition, 0xf5bc8fc5, 0x536d, 0x4f77, 0xb8, 0x52, 0xfb, 0xc1, 0x35, 0x6f, 0xde, 0xb6);
#ifndef __ITaskDefinition_INTERFACE_DEFINED__
#define __ITaskDefinition_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskDefinition;
	struct ITaskDefinition: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_RegistrationInfo(IRegistrationInfo **ppRegistrationInfo)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RegistrationInfo(IRegistrationInfo *pRegistrationInfo)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Triggers(ITriggerCollection **ppTriggers)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Triggers(ITriggerCollection *pTriggers)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Settings(ITaskSettings **ppSettings)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Settings(ITaskSettings *pSettings)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Data(BSTR *pData)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Data(BSTR data)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Principal(IPrincipal **ppPrincipal)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Principal(IPrincipal *pPrincipal)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Actions(IActionCollection **ppActions)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Actions(IActionCollection *pActions)=0;
		virtual HRESULT STDMETHODCALLTYPE get_XmlText(BSTR *pXml)=0;
		virtual HRESULT STDMETHODCALLTYPE put_XmlText(BSTR xml)=0;
	};
#endif

	DEFINE_GUID(IID_IRunningTaskCollection, 0x6a67614b, 0x6828, 0x4fec, 0xaa, 0x54, 0x6d, 0x52, 0xe8, 0xf1, 0xf2, 0xdb);
#ifndef __IRunningTaskCollection_INTERFACE_DEFINED__
#define __IRunningTaskCollection_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRunningTaskCollection;
	struct IRunningTaskCollection: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Count(LONG *pCount)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Item(VARIANT index, IRunningTask **ppRunningTask)=0;
		virtual HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown **ppEnum)=0;
	};
#endif

	DEFINE_GUID(IID_IRegisteredTask, 0x9c86f320, 0xdee3, 0x4dd1, 0xb9, 0x72, 0xa3, 0x03, 0xf2, 0x6b, 0x06, 0x1e);
#ifndef __IRegisteredTask_INTERFACE_DEFINED__
#define __IRegisteredTask_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRegisteredTask;
	struct IRegisteredTask: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Name(BSTR *pName)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Path(BSTR *pPath)=0;
		virtual HRESULT STDMETHODCALLTYPE get_State(TASK_STATE *pState)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Enabled(VARIANT_BOOL *pEnabled)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Enabled(VARIANT_BOOL enabled)=0;
		virtual HRESULT STDMETHODCALLTYPE Run(VARIANT params, IRunningTask **ppRunningTask)=0;
		virtual HRESULT STDMETHODCALLTYPE RunEx(VARIANT params, LONG flags, LONG sessionID, BSTR user, IRunningTask **ppRunningTask)=0;
		virtual HRESULT STDMETHODCALLTYPE GetInstances(LONG flags, IRunningTaskCollection **ppRunningTasks)=0;
		virtual HRESULT STDMETHODCALLTYPE get_LastRunTime(DATE *pLastRunTime)=0;
		virtual HRESULT STDMETHODCALLTYPE get_LastTaskResult(LONG *pLastTaskResult)=0;
		virtual HRESULT STDMETHODCALLTYPE get_NumberOfMissedRuns(LONG *pNumberOfMissedRuns)=0;
		virtual HRESULT STDMETHODCALLTYPE get_NextRunTime(DATE *pNextRunTime)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Definition(ITaskDefinition **ppDefinition)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Xml(BSTR *pXml)=0;
		virtual HRESULT STDMETHODCALLTYPE GetSecurityDescriptor(LONG securityInformation, BSTR *pSddl)=0;
		virtual HRESULT STDMETHODCALLTYPE SetSecurityDescriptor(BSTR sddl, LONG flags)=0;
		virtual HRESULT STDMETHODCALLTYPE Stop(LONG flags)=0;
		virtual HRESULT STDMETHODCALLTYPE GetRunTimes(const LPSYSTEMTIME pstStart, const LPSYSTEMTIME pstEnd, DWORD *pCount, LPSYSTEMTIME *pRunTimes)=0;
	};
#endif

	DEFINE_GUID(IID_IRunningTask, 0x653758fb, 0x7b9a, 0x4f1e, 0xa4, 0x71, 0xbe, 0xeb, 0x8e, 0x9b, 0x83, 0x4e);
#ifndef __IRunningTask_INTERFACE_DEFINED__
#define __IRunningTask_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRunningTask;
	struct IRunningTask: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Name(BSTR *pName)=0;
		virtual HRESULT STDMETHODCALLTYPE get_InstanceGuid(BSTR *pGuid)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Path(BSTR *pPath)=0;
		virtual HRESULT STDMETHODCALLTYPE get_State(TASK_STATE *pState)=0;
		virtual HRESULT STDMETHODCALLTYPE get_CurrentAction(BSTR *pName)=0;
		virtual HRESULT STDMETHODCALLTYPE Stop()=0;
		virtual HRESULT STDMETHODCALLTYPE Refresh()=0;
		virtual HRESULT STDMETHODCALLTYPE get_EnginePID(DWORD *pPID)=0;
	};
#endif

	DEFINE_GUID(IID_ITaskFolderCollection, 0x79184a66, 0x8664, 0x423f, 0x97, 0xf1, 0x63, 0x73, 0x56, 0xa5, 0xd8, 0x12);
#ifndef __ITaskFolderCollection_INTERFACE_DEFINED__
#define __ITaskFolderCollection_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskFolderCollection;
	struct ITaskFolderCollection: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Count(LONG *pCount)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Item(VARIANT index, ITaskFolder **ppFolder)=0;
		virtual HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown **ppEnum)=0;
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

	DEFINE_GUID(IID_IRegistrationInfo, 0x416D8B73, 0xCB41, 0x4ea1, 0x80, 0x5C, 0x9B, 0xE9, 0xA5, 0xAC, 0x4A, 0x74);
#ifndef __IRegistrationInfo_INTERFACE_DEFINED__
#define __IRegistrationInfo_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRegistrationInfo;
	struct IRegistrationInfo: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Description(BSTR *pDescription)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Description(BSTR description)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Author(BSTR *pAuthor)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Author(BSTR author)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Version(BSTR *pVersion)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Version(BSTR version)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Date(BSTR *pDate)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Date(BSTR date)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Documentation(BSTR *pDocumentation)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Documentation(BSTR documentation)=0;
		virtual HRESULT STDMETHODCALLTYPE get_XmlText(BSTR *pText)=0;
		virtual HRESULT STDMETHODCALLTYPE put_XmlText(BSTR text)=0;
		virtual HRESULT STDMETHODCALLTYPE get_URI(BSTR *pUri)=0;
		virtual HRESULT STDMETHODCALLTYPE put_URI(BSTR uri)=0;
		virtual HRESULT STDMETHODCALLTYPE get_SecurityDescriptor(VARIANT *pSddl)=0;
		virtual HRESULT STDMETHODCALLTYPE put_SecurityDescriptor(VARIANT sddl)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Source(BSTR *pSource)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Source(BSTR source)=0;
	};
#endif

	DEFINE_GUID(IID_ITriggerCollection, 0x85df5081, 0x1b24, 0x4f32, 0x87, 0x8a, 0xd9, 0xd1, 0x4d, 0xf4, 0xcb, 0x77);
#ifndef __ITriggerCollection_INTERFACE_DEFINED__
#define __ITriggerCollection_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITriggerCollection;
	struct ITriggerCollection: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Count(long *pCount)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Item(long index, ITrigger **ppTrigger)=0;
		virtual HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown **ppEnum)=0;
		virtual HRESULT STDMETHODCALLTYPE Create(TASK_TRIGGER_TYPE2 type, ITrigger **ppTrigger)=0;
		virtual HRESULT STDMETHODCALLTYPE Remove(VARIANT index)=0;
		virtual HRESULT STDMETHODCALLTYPE Clear()=0;
	};
#endif

	DEFINE_GUID(IID_ITrigger, 0x09941815, 0xea89, 0x4b5b, 0x89, 0xe0, 0x2a, 0x77, 0x38, 0x01, 0xfa, 0xc3);
#ifndef __ITrigger_INTERFACE_DEFINED__
#define __ITrigger_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITrigger;
	struct ITrigger: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Type(TASK_TRIGGER_TYPE2 *pType)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Id(BSTR *pId)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Id(BSTR id)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Repetition(IRepetitionPattern **ppRepeat)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Repetition(IRepetitionPattern *pRepeat)=0;
		virtual HRESULT STDMETHODCALLTYPE get_ExecutionTimeLimit(BSTR *pTimeLimit)=0;
		virtual HRESULT STDMETHODCALLTYPE put_ExecutionTimeLimit(BSTR timelimit)=0;
		virtual HRESULT STDMETHODCALLTYPE get_StartBoundary(BSTR *pStart)=0;
		virtual HRESULT STDMETHODCALLTYPE put_StartBoundary(BSTR start)=0;
		virtual HRESULT STDMETHODCALLTYPE get_EndBoundary(BSTR *pEnd)=0;
		virtual HRESULT STDMETHODCALLTYPE put_EndBoundary(BSTR end)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Enabled(VARIANT_BOOL *pEnabled)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Enabled(VARIANT_BOOL enabled)=0;
	};
#endif

	DEFINE_GUID(IID_IRepetitionPattern, 0x7FB9ACF1, 0x26BE, 0x400e, 0x85, 0xB5, 0x29, 0x4B, 0x9C, 0x75, 0xDF, 0xD6);
#ifndef __IRepetitionPattern_INTERFACE_DEFINED__
#define __IRepetitionPattern_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IRepetitionPattern;
	struct IRepetitionPattern: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Interval(BSTR *pInterval)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Interval(BSTR interval)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Duration(BSTR *pDuration)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Duration(BSTR duration)=0;
		virtual HRESULT STDMETHODCALLTYPE get_StopAtDurationEnd(VARIANT_BOOL *pStop)=0;
		virtual HRESULT STDMETHODCALLTYPE put_StopAtDurationEnd(VARIANT_BOOL stop)=0;
	};
#endif

	DEFINE_GUID(IID_ITaskSettings, 0x8FD4711D, 0x2D02, 0x4c8c, 0x87, 0xE3, 0xEF, 0xF6, 0x99, 0xDE, 0x12, 0x7E);
#ifndef __ITaskSettings_INTERFACE_DEFINED__
#define __ITaskSettings_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ITaskSettings;
	struct ITaskSettings: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_AllowDemandStart(VARIANT_BOOL *pAllowDemandStart)=0;
		virtual HRESULT STDMETHODCALLTYPE put_AllowDemandStart(VARIANT_BOOL allowDemandStart)=0;
		virtual HRESULT STDMETHODCALLTYPE get_RestartInterval(BSTR *pRestartInterval)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RestartInterval(BSTR restartInterval)=0;
		virtual HRESULT STDMETHODCALLTYPE get_RestartCount(int *pRestartCount)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RestartCount(int restartCount)=0;
		virtual HRESULT STDMETHODCALLTYPE get_MultipleInstances(TASK_INSTANCES_POLICY *pPolicy)=0;
		virtual HRESULT STDMETHODCALLTYPE put_MultipleInstances(TASK_INSTANCES_POLICY policy)=0;
		virtual HRESULT STDMETHODCALLTYPE get_StopIfGoingOnBatteries(VARIANT_BOOL *pStopIfOnBatteries)=0;
		virtual HRESULT STDMETHODCALLTYPE put_StopIfGoingOnBatteries(VARIANT_BOOL stopIfOnBatteries)=0;
		virtual HRESULT STDMETHODCALLTYPE get_DisallowStartIfOnBatteries(VARIANT_BOOL *pDisallowStart)=0;
		virtual HRESULT STDMETHODCALLTYPE put_DisallowStartIfOnBatteries(VARIANT_BOOL disallowStart)=0;
		virtual HRESULT STDMETHODCALLTYPE get_AllowHardTerminate(VARIANT_BOOL *pAllowHardTerminate)=0;
		virtual HRESULT STDMETHODCALLTYPE put_AllowHardTerminate(VARIANT_BOOL allowHardTerminate)=0;
		virtual HRESULT STDMETHODCALLTYPE get_StartWhenAvailable(VARIANT_BOOL *pStartWhenAvailable)=0;
		virtual HRESULT STDMETHODCALLTYPE put_StartWhenAvailable(VARIANT_BOOL startWhenAvailable)=0;
		virtual HRESULT STDMETHODCALLTYPE get_XmlText(BSTR *pText)=0;
		virtual HRESULT STDMETHODCALLTYPE put_XmlText(BSTR text)=0;
		virtual HRESULT STDMETHODCALLTYPE get_RunOnlyIfNetworkAvailable(VARIANT_BOOL *pRunOnlyIfNetworkAvailable)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RunOnlyIfNetworkAvailable(VARIANT_BOOL runOnlyIfNetworkAvailable)=0;
		virtual HRESULT STDMETHODCALLTYPE get_ExecutionTimeLimit(BSTR *pExecutionTimeLimit)=0;
		virtual HRESULT STDMETHODCALLTYPE put_ExecutionTimeLimit(BSTR executionTimeLimit)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Enabled(VARIANT_BOOL *pEnabled)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Enabled(VARIANT_BOOL enabled)=0;
		virtual HRESULT STDMETHODCALLTYPE get_DeleteExpiredTaskAfter(BSTR *pExpirationDelay)=0;
		virtual HRESULT STDMETHODCALLTYPE put_DeleteExpiredTaskAfter(BSTR expirationDelay)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Priority(int *pPriority)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Priority(int priority)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Compatibility(TASK_COMPATIBILITY *pCompatLevel)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Compatibility(TASK_COMPATIBILITY compatLevel)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Hidden(VARIANT_BOOL *pHidden)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Hidden(VARIANT_BOOL hidden)=0;
		virtual HRESULT STDMETHODCALLTYPE get_IdleSettings(IIdleSettings **ppIdleSettings)=0;
		virtual HRESULT STDMETHODCALLTYPE put_IdleSettings(IIdleSettings *pIdleSettings)=0;
		virtual HRESULT STDMETHODCALLTYPE get_RunOnlyIfIdle(VARIANT_BOOL *pRunOnlyIfIdle)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RunOnlyIfIdle(VARIANT_BOOL runOnlyIfIdle)=0;
		virtual HRESULT STDMETHODCALLTYPE get_WakeToRun(VARIANT_BOOL *pWake)=0;
		virtual HRESULT STDMETHODCALLTYPE put_WakeToRun(VARIANT_BOOL wake)=0;
		virtual HRESULT STDMETHODCALLTYPE get_NetworkSettings(INetworkSettings **ppNetworkSettings)=0;
		virtual HRESULT STDMETHODCALLTYPE put_NetworkSettings(INetworkSettings *pNetworkSettings)=0;
	};
#endif

	DEFINE_GUID(IID_IIdleSettings, 0x84594461, 0x0053, 0x4342, 0xA8, 0xFD, 0x08, 0x8F, 0xAB, 0xF1, 0x1F, 0x32);
#ifndef __IIdleSettings_INTERFACE_DEFINED__
#define __IIdleSettings_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IIdleSettings;
	struct IIdleSettings: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_IdleDuration(BSTR *pDelay)=0;
		virtual HRESULT STDMETHODCALLTYPE put_IdleDuration(BSTR delay)=0;
		virtual HRESULT STDMETHODCALLTYPE get_WaitTimeout(BSTR *pTimeout)=0;
		virtual HRESULT STDMETHODCALLTYPE put_WaitTimeout(BSTR timeout)=0;
		virtual HRESULT STDMETHODCALLTYPE get_StopOnIdleEnd(VARIANT_BOOL *pStop)=0;
		virtual HRESULT STDMETHODCALLTYPE put_StopOnIdleEnd(VARIANT_BOOL stop)=0;
		virtual HRESULT STDMETHODCALLTYPE get_RestartOnIdle(VARIANT_BOOL *pRestart)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RestartOnIdle(VARIANT_BOOL restart)=0;
	};
#endif

	DEFINE_GUID(IID_INetworkSettings, 0x9F7DEA84, 0xC30B, 0x4245, 0x80, 0xB6, 0x00, 0xE9, 0xF6, 0x46, 0xF1, 0xB4);
#ifndef __INetworkSettings_INTERFACE_DEFINED__
#define __INetworkSettings_INTERFACE_DEFINED__
	EXTERN_C const IID IID_INetworkSettings;
	struct INetworkSettings: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Name(BSTR *pName)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Name(BSTR name)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Id(BSTR *pId)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Id(BSTR id)=0;
	};
#endif

	DEFINE_GUID(IID_IPrincipal, 0xD98D51E5, 0xC9B4, 0x496a, 0xA9, 0xC1, 0x18, 0x98, 0x02, 0x61, 0xCF, 0x0F);
#ifndef __IPrincipal_INTERFACE_DEFINED__
#define __IPrincipal_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IPrincipal;
	struct IPrincipal: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Id(BSTR *pId)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Id(BSTR Id)=0;
		virtual HRESULT STDMETHODCALLTYPE get_DisplayName(BSTR *pName)=0;
		virtual HRESULT STDMETHODCALLTYPE put_DisplayName(BSTR name)=0;
		virtual HRESULT STDMETHODCALLTYPE get_UserId(BSTR *pUser)=0;
		virtual HRESULT STDMETHODCALLTYPE put_UserId(BSTR user)=0;
		virtual HRESULT STDMETHODCALLTYPE get_LogonType(TASK_LOGON_TYPE *pLogon)=0;
		virtual HRESULT STDMETHODCALLTYPE put_LogonType(TASK_LOGON_TYPE logon)=0;
		virtual HRESULT STDMETHODCALLTYPE get_GroupId(BSTR *pGroup)=0;
		virtual HRESULT STDMETHODCALLTYPE put_GroupId(BSTR group)=0;
		virtual HRESULT STDMETHODCALLTYPE get_RunLevel(TASK_RUNLEVEL_TYPE *pRunLevel)=0;
		virtual HRESULT STDMETHODCALLTYPE put_RunLevel(TASK_RUNLEVEL_TYPE runLevel)=0;
	};
#endif

	DEFINE_GUID(IID_IActionCollection, 0x02820E19, 0x7B98, 0x4ed2, 0xB2, 0xE8, 0xFD, 0xCC, 0xCE, 0xFF, 0x61, 0x9B);
#ifndef __IActionCollection_INTERFACE_DEFINED__
#define __IActionCollection_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IActionCollection;
	struct IActionCollection: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Count(long *pCount)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Item(long index, IAction **ppAction)=0;
		virtual HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown **ppEnum)=0;
		virtual HRESULT STDMETHODCALLTYPE get_XmlText(BSTR *pText)=0;
		virtual HRESULT STDMETHODCALLTYPE put_XmlText(BSTR text)=0;
		virtual HRESULT STDMETHODCALLTYPE Create(TASK_ACTION_TYPE type, IAction **ppAction)=0;
		virtual HRESULT STDMETHODCALLTYPE Remove(VARIANT index)=0;
		virtual HRESULT STDMETHODCALLTYPE Clear()=0;
		virtual HRESULT STDMETHODCALLTYPE get_Context(BSTR *pContext)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Context(BSTR context)=0;
	};
#endif

	DEFINE_GUID(IID_IAction, 0xBAE54997, 0x48B1, 0x4cbe, 0x99, 0x65, 0xD6, 0xBE, 0x26, 0x3E, 0xBE, 0xA4);
#ifndef __IAction_INTERFACE_DEFINED__
#define __IAction_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IAction;
	struct IAction: public IDispatch {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Id(BSTR *pId)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Id(BSTR Id)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Type(TASK_ACTION_TYPE *pType)=0;
	};
#endif

	DEFINE_GUID(IID_ILogonTrigger, 0x72DADE38, 0xFAE4, 0x4b3e, 0xBA, 0xF4, 0x5D, 0x00, 0x9A, 0xF0, 0x2B, 0x1C);
#ifndef __ILogonTrigger_INTERFACE_DEFINED__
#define __ILogonTrigger_INTERFACE_DEFINED__
	EXTERN_C const IID IID_ILogonTrigger;
	struct ILogonTrigger: public ITrigger {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Delay(BSTR *pDelay)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Delay(BSTR delay)=0;
		virtual HRESULT STDMETHODCALLTYPE get_UserId(BSTR *pUser)=0;
		virtual HRESULT STDMETHODCALLTYPE put_UserId(BSTR user)=0;
	};
#endif

	DEFINE_GUID(IID_IExecAction, 0x4c3d624d, 0xfd6b, 0x49a3, 0xb9, 0xb7, 0x09, 0xcb, 0x3c, 0xd3, 0xf0, 0x47);
#ifndef __IExecAction_INTERFACE_DEFINED__
#define __IExecAction_INTERFACE_DEFINED__
	EXTERN_C const IID IID_IExecAction;
	struct IExecAction: public IAction {
	public:
		virtual HRESULT STDMETHODCALLTYPE get_Path(BSTR *pPath)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Path(BSTR path)=0;
		virtual HRESULT STDMETHODCALLTYPE get_Arguments(BSTR *pArgument)=0;
		virtual HRESULT STDMETHODCALLTYPE put_Arguments(BSTR argument)=0;
		virtual HRESULT STDMETHODCALLTYPE get_WorkingDirectory(BSTR *pWorkingDirectory)=0;
		virtual HRESULT STDMETHODCALLTYPE put_WorkingDirectory(BSTR workingDirectory)=0;
	};
#endif

}

#endif //TASKSCHD_HS_H
