!define APPNAME "SnK HotkeySuite"
!define UNINST_NAME "unins000.exe"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_INSTDIR "${APPNAME}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "${MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME}"
!define MULTIUSER_INSTALLMODE_FUNCTION patchInstdir
!include MultiUser.nsh
!include MUI2.nsh
!include Sections.nsh
!include LogicLib.nsh
!include FileFunc.nsh
!include WinVer.nsh

!ifdef INST64
	!include x64.nsh
	OutFile "HotkeySuiteSetup64.exe"
	Caption "${APPNAME} Setup (x64)"
	UninstallCaption "${APPNAME} Uninstall (x64)"
!else
	OutFile "HotkeySuiteSetup32.exe"
	Caption "${APPNAME} Setup"
	UninstallCaption "${APPNAME} Uninstall"
!endif
Name "${APPNAME}"
BrandingText " "
Var USER_APPDATA
Var D_INSTDIR
Var StartMenuLocation
;Override RequestExecutionLevel set by MultiUser (MULTIUSER_EXECUTIONLEVEL Highest)
;On Vista and above Admin rights will be required while on pre-Vista highest available security level will be used
;Installer requires admin rights - because of the need to register HotkeySuite with Task Scheduler
RequestExecutionLevel admin
;Though MultiUser sets INSTDIR variable by itself, InstallDir call is required for after-browse default directory name auto-append to work
InstallDir "\${APPNAME}"

!define MUI_ICON "hs.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.TXT"
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE checkIfD
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINST_KEY}" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuLocation"
!insertmacro MUI_PAGE_STARTMENU Page_SMenu $StartMenuLocation
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\HotkeySuite.exe"
!define MUI_FINISHPAGE_RUN_PARAMETERS "/a user"
!define MUI_FINISHPAGE_RUN_TEXT "Run HotkeySuite"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.TXT"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show HotkeySuite readme"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Delete SnK HotkeySuite settings"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_RUN_FUNCTION un.deleteStoredSettings
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

!getdllversion "HotkeySuite.exe" APPVER_
!getdllversion "..\snk\SnK.exe" SNKVER_
!searchparse /file "SuiteVersion.h" '#define HS_CRIGHT_YEARS	"' CRIGHT_YEARS '"'
VIProductVersion "${APPVER_1}.${APPVER_2}.0.0"
VIFileVersion "${APPVER_1}.${APPVER_2}.${SNKVER_1}.${SNKVER_2}"
!ifdef INST64
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APPNAME} Setup (x64)"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPNAME} (x64)"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "HotkeySuiteSetup64"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "HotkeySuiteSetup64.exe"
!else
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APPNAME} Setup"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPNAME}"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "HotkeySuiteSetup32"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "HotkeySuiteSetup32.exe"
!endif
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APPVER_1}.${APPVER_2}.${SNKVER_1}.${SNKVER_2}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Lcferrum"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (c) ${CRIGHT_YEARS} Lcferrum"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${APPVER_1}.${APPVER_2}"

SectionGroup /e "HotkeySuite" Grp_HS
	Section "Executable" Sec_HS
		SectionIn RO
		SetOverwrite on
		SetOutPath $INSTDIR
		File "HotkeySuite.exe"
		File "CHANGELOG.TXT"
		File "LICENSE.TXT"
		File "README.TXT"
	SectionEnd
	Section "Default SnK Script" Sec_DEF_SCRIPT
		SetOverwrite on
		SetOutPath "$USER_APPDATA\${APPNAME}"
		File /oname=on_hotkey.txt "snk_default_script.txt"
	SectionEnd
	Section "Add to Autorun" Sec_AUTORUN
		${if} ${AtLeastWinVista}
			${if} $MultiUser.InstallMode == AllUsers
				ExecWait '"$INSTDIR\HotkeySuite.exe" /S machine /a user'
			${else}
				ExecWait '"$INSTDIR\HotkeySuite.exe" /S user /a user'
			${endif}
		${else}
			${if} $MultiUser.InstallMode == AllUsers
				ExecWait '"$INSTDIR\HotkeySuite.exe" /A machine /a user'
			${else}
				ExecWait '"$INSTDIR\HotkeySuite.exe" /A user /a user'
			${endif}
		${endif}
	SectionEnd
SectionGroupEnd

Section "SnK" Sec_SNK
	SetOverwrite on
	SetOutPath $INSTDIR
	File "..\snk\SnK.exe"
	File "..\snk\SnKh.exe"
	File /oname=SNK.CHANGELOG.TXT "..\snk\CHANGELOG.TXT"
	File /oname=SNK.README.TXT "..\snk\README.TXT"
	File /oname=SNK.LICENSE.TXT "..\snk\LICENSE.TXT"
	File /oname=SNK.DetectMatrix.html "..\snk\DetectMatrix.html"
SectionEnd

Section /o "Add to PATH" Sec_PATH
	${if} $MultiUser.InstallMode == AllUsers
		ExecWait '"$INSTDIR\HotkeySuite.exe" /P machine'
	${else}
		ExecWait '"$INSTDIR\HotkeySuite.exe" /P user'
	${endif}
SectionEnd

Section "-Postinstall"
	SetOverwrite on
	SetOutPath $INSTDIR
	
	WriteUninstaller "$INSTDIR\${UNINST_NAME}"

	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "${APPNAME}"
	WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayVersion" "${APPVER_1}.${APPVER_2}"
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\HotkeySuite.exe,0"
	WriteRegStr SHCTX "${UNINST_KEY}" "Publisher" "Lcferrum"
	WriteRegStr SHCTX "${UNINST_KEY}" "URLInfoAbout" "https://github.com/lcferrum/snk_hotkeysuite"
	WriteRegStr SHCTX "${UNINST_KEY}" "UninstallString" '"$INSTDIR\${UNINST_NAME}" /$MultiUser.InstallMode'
	WriteRegStr SHCTX "${UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\${UNINST_NAME}" /$MultiUser.InstallMode /S'
	WriteRegDWORD SHCTX "${UNINST_KEY}" "NoModify" "1"
	WriteRegDWORD SHCTX "${UNINST_KEY}" "NoRepair" "1"
	WriteRegDWORD SHCTX "${UNINST_KEY}" "VersionMajor" "${APPVER_1}"
	WriteRegDWORD SHCTX "${UNINST_KEY}" "VersionMinor" "${APPVER_2}"
	
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD SHCTX "${UNINST_KEY}" "EstimatedSize" "$0"
	
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Page_SMenu
		CreateDirectory "$SMPROGRAMS\$StartMenuLocation"
		CreateShortcut "$SMPROGRAMS\$StartMenuLocation\Uninstall.lnk" "$INSTDIR\${UNINST_NAME}" "/$MultiUser.InstallMode"
		CreateShortcut "$SMPROGRAMS\$StartMenuLocation\HotkeySuite.lnk" "$INSTDIR\HotkeySuite.exe" "/a user"
	!insertmacro MUI_STARTMENU_WRITE_END
	
	${if} ${Silent}
		Exec '"$INSTDIR\HotkeySuite.exe" /a user'
	${endif}
SectionEnd

Section "Uninstall"
	;Try to kill HotkeySuite using it's own SnK
	;Can fail if HotkeySuite was never run for current user or non-default INI path is used and SnK was installed at non-default location
	StrCpy $R0 "$INSTDIR\SnKh.exe"
	${ifnot} ${FileExists} "$R0"
		ReadINIStr $R0 "$USER_APPDATA\${APPNAME}\HotkeySuite.ini" "HotkeySuite" "SnkPath"
		${if} "$R0" != ""
			System::Call 'Kernel32::SetEnvironmentVariable(t, t)i ("HS_EXE_PATH", "$INSTDIR").r0'
			System::Call 'Kernel32::SetEnvironmentVariable(t, t)i ("HS_INI_PATH", "$USER_APPDATA\${APPNAME}").r0'
			ExpandEnvStrings $R0 "$R0"
			${ifnot} ${FileExists} "$R0"
				StrCpy $R0 ""
			${endif}
		${endif}
	${endif}
	${if} "$R0" != ""
		ExecWait '"$R0" +alc /pth:full="$INSTDIR\HotkeySuite.exe"'
	${endif}

	!insertmacro MUI_STARTMENU_GETFOLDER Page_SMenu $StartMenuLocation
	Delete "$SMPROGRAMS\$StartMenuLocation\HotkeySuite.lnk"
	Delete "$SMPROGRAMS\$StartMenuLocation\Uninstall.lnk"
	RMDir "$SMPROGRAMS\$StartMenuLocation"
  
	DeleteRegKey SHCTX "${UNINST_KEY}"
	
	${if} ${AtLeastWinVista}
		${if} $MultiUser.InstallMode == AllUsers
			ExecWait '"$INSTDIR\HotkeySuite.exe" /U machine'
		${else}
			ExecWait '"$INSTDIR\HotkeySuite.exe" /U user'
		${endif}
	${else}
		${if} $MultiUser.InstallMode == AllUsers
			ExecWait '"$INSTDIR\HotkeySuite.exe" /R machine'
		${else}
			ExecWait '"$INSTDIR\HotkeySuite.exe" /R user'
		${endif}
	${endif}
	
	${if} $MultiUser.InstallMode == AllUsers
		ExecWait '"$INSTDIR\HotkeySuite.exe" /C machine'
	${else}
		ExecWait '"$INSTDIR\HotkeySuite.exe" /C user'
	${endif}
		
	Delete "$INSTDIR\HotkeySuite.exe"
	Delete "$INSTDIR\CHANGELOG.TXT"
	Delete "$INSTDIR\LICENSE.TXT"
	Delete "$INSTDIR\README.TXT"
	Delete "$INSTDIR\SnK.exe"
	Delete "$INSTDIR\SnKh.exe"
	Delete "$INSTDIR\SNK.CHANGELOG.TXT"
	Delete "$INSTDIR\SNK.README.TXT"
	Delete "$INSTDIR\SNK.LICENSE.TXT"
	Delete "$INSTDIR\SNK.DetectMatrix.html"
	Delete "$INSTDIR\${UNINST_NAME}"
	RMDir "$INSTDIR"
	
	${if} ${FileExists} "$INSTDIR\HotkeySuite.exe"
		MessageBox MB_OK|MB_ICONINFORMATION 'Unable to delete "$INSTDIR\HotkeySuite.exe"$\nMaybe an instance of the program is still running. Please close it and delete file manually.' /SD IDOK
	${endif}
SectionEnd

!ifdef INST64
	LangString DESC_Grp_HS ${LANG_ENGLISH} "Install 64-bit version of HotkeySuite with additional options."
	LangString DESC_Sec_SNK ${LANG_ENGLISH} "Install bundeled SnK distribution (v${SNKVER_1}.${SNKVER_2} x64). If you don't want to install it - you can download it separately and set SnkPath variable in HotkeySuite.ini accordingly."
!else
	LangString DESC_Grp_HS ${LANG_ENGLISH} "Install 32-bit version of HotkeySuite with additional options."
	LangString DESC_Sec_SNK ${LANG_ENGLISH} "Install bundeled SnK distribution (v${SNKVER_1}.${SNKVER_2}). If you don't want to install it - you can download it separately and set SnkPath variable in HotkeySuite.ini accordingly."
!endif
LangString DESC_Sec_HS ${LANG_ENGLISH} "HotkeySuite main distribution - executable with docs."
LangString DESC_Sec_DEF_SCRIPT ${LANG_ENGLISH} "Default SnK script to run on hotkey press. You can check what this script does by looking at it's source code (on_hotkey.txt) after installation. Script will be installed only for current user."
LangString DESC_Sec_AUTORUN ${LANG_ENGLISH} "Add HotkeySuite to Autorun (pre-Vista) or schedule it using Task Scheduler (Vista and above)."
LangString DESC_Sec_PATH ${LANG_ENGLISH} "Add installation directory to PATH variable. So HotkeySuite and bundeled SnK will be available from command prompt."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Grp_HS} $(DESC_Grp_HS)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_HS} $(DESC_Sec_HS)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_DEF_SCRIPT} $(DESC_Sec_DEF_SCRIPT)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_AUTORUN} $(DESC_Sec_AUTORUN)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_SNK} $(DESC_Sec_SNK)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_PATH} $(DESC_Sec_PATH)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
	${ifnot} ${IsNT}
		MessageBox MB_OK|MB_ICONEXCLAMATION "Windows NT family OS required!$\nInstallation will be aborted." /SD IDOK
		Quit
	${endif}
	!ifdef INST64
		${ifnot} ${RunningX64}
			MessageBox MB_OK|MB_ICONEXCLAMATION "64-bit Windows OS required!$\nInstallation will be aborted." /SD IDOK
			Quit
		${endif}
	!endif
	StrCpy $USER_APPDATA "$APPDATA"	;Hack to get SetShellVarContext-independent APPDATA
	${if} "$INSTDIR" != "\${APPNAME}"	;Don't loose $INSTDIR set with /D
		StrCpy $D_INSTDIR "$INSTDIR"
	${endif}
	!insertmacro MULTIUSER_INIT
	${if} "$D_INSTDIR" != ""		;If $INSTDIR was set with /D - reapply it after initializing MultiUser.nsh (for Silent mode)
		StrCpy $INSTDIR "$D_INSTDIR"
	${endif}
FunctionEnd

Function un.onInit
	StrCpy $USER_APPDATA "$APPDATA"	;Hack to get SetShellVarContext-independent APPDATA
	!insertmacro MULTIUSER_UNINIT
FunctionEnd

Function checkIfD
	;If $INSTDIR was set with /D - reapply it after MULTIUSER_PAGE_INSTALLMODE
	;Works only with GUI mode
	${if} "$D_INSTDIR" != ""
		StrCpy $INSTDIR "$D_INSTDIR"
	${endif}
FunctionEnd

Function patchInstdir
	;Though documentation for MultiUser.nsh states that MULTIUSER_USE_PROGRAMFILES64 define is supported, version of the plugin included with NSIS v3.01 doesn't support it
	!ifdef INST64
		${if} ${RunningX64}
		${andif} $MultiUser.InstallMode == AllUsers
		${andif} $MultiUser.InstDir == ""
			StrCpy $INSTDIR "$PROGRAMFILES64\${APPNAME}"
		${endif}
	!endif

	;There is a bug (or rather oversight) in MultiUser.nsh included with NSIS v3.01
	;If it detects that Windows version is less than Win2k it assumes that NSIS is unable to get $LOCALAPPDATA path and replaces it with $PROGRAMFILES
	;This misconception probably stems from MSDN docs that say that CSIDL_LOCAL_APPDATA is available since v5.0 of shell32.dll, which is available only from Win2k onwards
	;What they don't say is that there is a shfolder.dll that emulates CSIDL_LOCAL_APPDATA for older versions of shell32.dll
	;This shfolder.dll is bundled with IE5 (or as part of Platform SDK Redistributable) that can be installed on NT4 and NSIS (at least since v3.01) will use it instead of shell32.dll if it is available
	;And even when shfolder.dll is unavailable on NT4 (and so $LOCALAPPDATA returns nothing), local AppData path can still be queried here using registry
	${ifnot} ${AtLeastWin2000}
	${andif} $MultiUser.InstallMode == CurrentUser
	${andif} $MultiUser.InstDir == ""
		${if} "$LOCALAPPDATA" != ""
			StrCpy $INSTDIR "$LOCALAPPDATA\${APPNAME}"
		${else}
			ReadRegStr $INSTDIR HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Local AppData"
			${if} "$INSTDIR" == ""
				StrCpy $INSTDIR "$PROGRAMFILES\${APPNAME}"
			${else}
				StrCpy $INSTDIR "$INSTDIR\${APPNAME}"
			${endif}
		${endif}
	${endif}
FunctionEnd

Function un.deleteStoredSettings
	RMDir /r "$USER_APPDATA\${APPNAME}"
FunctionEnd
