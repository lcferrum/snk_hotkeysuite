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
!define MULTIUSER_INSTALLMODE_FUNCTION patchInstdirNT4
!include MultiUser.nsh
!include MUI2.nsh
!include Sections.nsh
!include LogicLib.nsh
!include FileFunc.nsh
!include WinVer.nsh

OutFile "HotkeySuiteSetup.exe"
Name "${APPNAME}"
BrandingText " "
Var USER_APPDATA
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
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "SnK HotkeySuite Setup"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APPVER_1}.${APPVER_2}.${SNKVER_1}.${SNKVER_2}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Lcferrum"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (c) ${CRIGHT_YEARS} Lcferrum"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "HotkeySuiteSetup"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "HotkeySuiteSetup.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPNAME}"
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
	File "..\snk\DetectMatrix.html"
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

	WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "${APPNAME}"
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayVersion" "${APPVER_1}.${APPVER_2}"
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\HotkeySuite.exe,0"
	WriteRegStr SHCTX "${UNINST_KEY}" "Publisher" "Lcferrum"
	WriteRegStr SHCTX "${UNINST_KEY}" "URLInfoAbout" "https://github.com/lcferrum"
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
SectionEnd

Section "Uninstall"
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
	Delete "$INSTDIR\DetectMatrix.html"
	Delete "$INSTDIR\${UNINST_NAME}"
	RMDir "$INSTDIR"
SectionEnd

LangString DESC_Grp_HS ${LANG_ENGLISH} "Install HotkeySuite with additional options."
LangString DESC_Sec_HS ${LANG_ENGLISH} "HotkeySuite main distribution - executable with docs."
LangString DESC_Sec_DEF_SCRIPT ${LANG_ENGLISH} "Default SnK script to run on hotkey press. You can check what this script does by looking at it's source code (on_hotkey.txt) after installation. Script will be installed only for current user."
LangString DESC_Sec_AUTORUN ${LANG_ENGLISH} "Add HotkeySuite to Autorun (pre-Vista) or schedule it using Task Scheduler (Vista and above)."
LangString DESC_Sec_SNK ${LANG_ENGLISH} "Install bundeled SnK distribution (v${SNKVER_1}.${SNKVER_2}). If you don't want to install it - you can download it separately and set SnkPath variable in HotkeySuite.ini accordingly."
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
		IfSilent +2	;If IfSilent=TRUE jumps over MessageBox
		MessageBox MB_OK|MB_ICONEXCLAMATION "Windows NT family OS required!$\nInstallation will be aborted."
		Quit
	${endif}
	StrCpy $USER_APPDATA "$APPDATA"	;Hack to get SetShellVarContext-independent APPDATA
	!insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
	StrCpy $USER_APPDATA "$APPDATA"	;Hack to get SetShellVarContext-independent APPDATA
	!insertmacro MULTIUSER_UNINIT
FunctionEnd

Function patchInstdirNT4
	;There is a bug (or rather oversight) in MultiUser.nsh
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
