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
!include WinVer.nsh

OutFile "HotkeySuiteSetup.exe"
Name "${APPNAME}"
BrandingText " "
;Override RequestExecutionLevel set by MultiUser (MULTIUSER_EXECUTIONLEVEL Highest)
;On Vista and above Admin rights will be required while on pre-Vista highest available security level will be used
;Installer requires admin rights - because of the need to register HotkeySuite with Task Scheduler
RequestExecutionLevel admin

!define MUI_ICON "hs.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.TXT"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\HotkeySuite.exe"
!define MUI_FINISHPAGE_RUN_PARAMETERS "/a current"
!define MUI_FINISHPAGE_RUN_TEXT "Run HotkeySuite"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.TXT"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show HotkeySuite readme"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

SectionGroup /e "HotkeySuite" Grp_HS
	Section "Executable" Sec_HS
		SectionIn RO
		SetOverwrite on
		SetOutPath $INSTDIR
		File "HotkeySuite.exe"
		File "CHANGELOG.TXT"
		File "LICENSE.TXT"
		File "README.TXT"
		
		WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
		WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "${APPNAME}"
		WriteRegStr SHCTX "${UNINST_KEY}" "UninstallString" '"$INSTDIR\${UNINST_NAME}" /$MultiUser.InstallMode'
		WriteRegStr SHCTX "${UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\${UNINST_NAME}" /$MultiUser.InstallMode /S'
		WriteRegDWORD SHCTX "${UNINST_KEY}" "EstimatedSize" "5"
		
		WriteUninstaller "$INSTDIR\${UNINST_NAME}"
	SectionEnd
	Section "Default SnK Script" Sec_DEF_SCRIPT
		SetOverwrite on
		SetOutPath "$APPDATA\${APPNAME}"
		File /oname=on_hotkey.txt "snk_default_script.txt"
	SectionEnd
	Section "Add to Autorun" Sec_AUTORUN
		${if} ${AtLeastWinVista}
			${if} $MultiUser.InstallMode == AllUsers
				ExecWait '"$INSTDIR\HotkeySuite.exe" /S all /a current'
			${else}
				ExecWait '"$INSTDIR\HotkeySuite.exe" /S current /a current'
			${endif}
		${else}
			${if} $MultiUser.InstallMode == AllUsers
				ExecWait '"$INSTDIR\HotkeySuite.exe" /A all /a current'
			${else}
				ExecWait '"$INSTDIR\HotkeySuite.exe" /A current /a current'
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
	MessageBox MB_OK "Sec_PATH"
SectionEnd

Section "Uninstall"
	DeleteRegKey SHCTX "${UNINST_KEY}"
SectionEnd

LangString DESC_Grp_HS ${LANG_ENGLISH} "Install HotkeySuite with additional options."
LangString DESC_Sec_HS ${LANG_ENGLISH} "HotkeySuite main distribution - executable with docs."
LangString DESC_Sec_DEF_SCRIPT ${LANG_ENGLISH} "Default SnK script to run on hotkey press. You can check what this script does by looking at it's source code (on_hotkey.txt) after installation."
LangString DESC_Sec_AUTORUN ${LANG_ENGLISH} "Add HotkeySuite to Autorun (pre-Vista) or schedule it using Task Scheduler (Vista and above)."
LangString DESC_Sec_SNK ${LANG_ENGLISH} "Install bundeled SnK distribution. If you don't want to install it - you can download it separately and set SnkPath variable in HotkeySuite.ini accordingly."
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
	!insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
	!insertmacro MULTIUSER_UNINIT
FunctionEnd

Function patchInstdirNT4
	${ifnot} ${AtLeastWin2000}
	${andif} $MultiUser.InstallMode == CurrentUser
	${andif} $MultiUser.InstDir == ""
		${if} "$LOCALAPPDATA" != ""
			StrCpy $INSTDIR "$LOCALAPPDATA\${APPNAME}"
		${else}
			ReadRegStr $INSTDIR HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Local AppData"
			StrCpy $INSTDIR "$INSTDIR\${APPNAME}"
		${endif}
	${endif}
FunctionEnd
