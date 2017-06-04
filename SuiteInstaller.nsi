!define APPNAME "SnK HotkeySuite"
!define UNINST_NAME "unins000.exe"
!define INST_FEATURES "InstalledFeatures"
!define STARTMENU_LOC "StartMenuLocation"
!ifdef INST64
	!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} (x64)"
!else
	!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!endif
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_INSTDIR "${APPNAME}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "${MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME}"
!define MULTIUSER_INSTALLMODE_FUNCTION patchInstdir
;This script has specific hacks for MultiUser and MUI2/StartMenu included with NSIS v3.01 - future versions may break them
!include MultiUser.nsh
!include MUI2.nsh
!include nsDialogs.nsh
!include LogicLib.nsh
!include WinVer.nsh

!getdllversion "HotkeySuite.exe" APPVER_
!searchparse /file "SuiteVersion.h" '#define HS_CRIGHT_YEARS	"' CRIGHT_YEARS '"'
!searchparse /file "SuiteVersion.h" '#define _HS_DEV_BUILD	' HS_DEV_BUILD '	'
!if ${HS_DEV_BUILD} == "0"
	!define /redef HS_DEV_BUILD
!else
	!define /redef HS_DEV_BUILD "-dev"
!endif
!getdllversion "..\snk\SnK.exe" SNKVER_
!searchparse /file "..\snk\Version.h" '#define _SNK_DEV_BUILD		' SNK_DEV_BUILD '	'
!if ${SNK_DEV_BUILD} == "0"
	!define /redef SNK_DEV_BUILD
!else
	!define /redef SNK_DEV_BUILD "-dev"
!endif

!ifdef INST64
	!include x64.nsh
	OutFile "SnKHotkeySuiteSetup64.exe"
	Caption "${APPNAME} v${APPVER_1}.${APPVER_2}${HS_DEV_BUILD} Setup (x64)"
	UninstallCaption "${APPNAME} v${APPVER_1}.${APPVER_2}${HS_DEV_BUILD} Uninstall (x64)"
!else
	OutFile "SnKHotkeySuiteSetup32.exe"
	Caption "${APPNAME} v${APPVER_1}.${APPVER_2}${HS_DEV_BUILD} Setup"
	UninstallCaption "${APPNAME} v${APPVER_1}.${APPVER_2}${HS_DEV_BUILD} Uninstall"
!endif
Name "${APPNAME}"
BrandingText " "
Var USER_APPDATA
Var D_INSTDIR
Var UpgDialog.HWND_UPG
Var UpgDialog.HWND_UN
Var UpgDialog.HWND_CNT
Var UpgDialog.Status
Var UpgDialog.OrigPath
Var UpgDialog.OrigMode
Var InstFeatures
Var StartMenu.Location
Var StartMenu.PrevMode
;Override RequestExecutionLevel set by MultiUser (MULTIUSER_EXECUTIONLEVEL Highest)
;On Vista and above Admin rights will be required while on pre-Vista highest available security level will be used
;Installer requires admin rights - because of the need to register HotkeySuite with Task Scheduler
RequestExecutionLevel admin
;Though MultiUser sets INSTDIR variable by itself, InstallDir call is required for after-browse default directory name auto-append to work
InstallDir "\${APPNAME}"

!define MUI_ICON "SuiteInstaller.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "SuiteInstaller.License.rtf"
Page Custom upgradePage upgradePageLeave
!define MUI_PAGE_CUSTOMFUNCTION_PRE changeNextToInstOnUpgrade
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_PRE skipPageIfUpgrade
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!define MUI_PAGE_CUSTOMFUNCTION_PRE skipPageIfUpgrade
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_PAGE_CUSTOMFUNCTION_PRE customStartMenuPageIni
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINST_KEY}" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${STARTMENU_LOC}"
!insertmacro MUI_PAGE_STARTMENU Page_SMenu $StartMenu.Location
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
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Lcferrum"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (c) ${CRIGHT_YEARS} Lcferrum"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APPVER_1}.${APPVER_2}${HS_DEV_BUILD}+${SNKVER_1}.${SNKVER_2}${SNK_DEV_BUILD}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${APPVER_1}.${APPVER_2}${HS_DEV_BUILD}"

SectionGroup /e "HotkeySuite" Grp_HS
	Section "Executable" Sec_HS
		SectionIn RO
		SetOverwrite on
		SetOutPath $INSTDIR
		
		;Check if HotkeySuite was previously installed and try to kill it using it's own SnK
		Call killExistingSnK
		
		File "CHANGELOG.TXT"
		File "LICENSE.TXT"
		File "README.TXT"
		File "HotkeySuite.exe"
	SectionEnd
	Section "Default SnK Script" Sec_DEF_SCRIPT
		SetOverwrite on
		SetOutPath "$USER_APPDATA\${APPNAME}"
		File "on_hotkey.txt"
	SectionEnd
	Section /o "" Sec_DEF_SCRIPT2
		SetOverwrite on
		SetOutPath "$USER_APPDATA\${APPNAME}"
		File "on_hotkey.txt"
	SectionEnd
	Section "Add to Autorun" Sec_AUTORUN
		IntOp $InstFeatures $InstFeatures | 1
		;/S and /A switches will silently update scheduled task and autorun entry if they already exist
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
	IntOp $InstFeatures $InstFeatures | 2
	File "..\snk\SnK.exe"
	File "..\snk\SnKh.exe"
	File /oname=SnK.CHANGELOG.TXT "..\snk\CHANGELOG.TXT"
	File /oname=SnK.README.TXT "..\snk\README.TXT"
	File /oname=SnK.LICENSE.TXT "..\snk\LICENSE.TXT"
	File /oname=SnK.DetectMatrix.html "..\snk\DetectMatrix.html"
SectionEnd

Section /o "Add to PATH" Sec_PATH
	IntOp $InstFeatures $InstFeatures | 4
	;/P switch won't add dublicated entries to PATH variable
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

!ifdef INST64
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "${APPNAME} (x64)"
!else
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "${APPNAME}"
!endif
	WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr SHCTX "${UNINST_KEY}" "DisplayVersion" "${APPVER_1}.${APPVER_2}${HS_DEV_BUILD}"
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
		IntOp $InstFeatures $InstFeatures | 8
		CreateDirectory "$SMPROGRAMS\$StartMenu.Location"
		CreateShortcut "$SMPROGRAMS\$StartMenu.Location\Uninstall.lnk" "$INSTDIR\${UNINST_NAME}" "/$MultiUser.InstallMode"
		CreateShortcut "$SMPROGRAMS\$StartMenu.Location\HotkeySuite.lnk" "$INSTDIR\HotkeySuite.exe" "/a user"
	!insertmacro MUI_STARTMENU_WRITE_END
	
	WriteRegDWORD SHCTX "${UNINST_KEY}" "${INST_FEATURES}" "$InstFeatures"
	
	${if} ${Silent}
		Exec '"$INSTDIR\HotkeySuite.exe" /a user'
	${endif}
SectionEnd

Section "Uninstall"
	;Try to kill HotkeySuite using it's own SnK
	Call un.killInstalledSnK

	!insertmacro MUI_STARTMENU_GETFOLDER Page_SMenu $StartMenu.Location
	Delete "$SMPROGRAMS\$StartMenu.Location\HotkeySuite.lnk"
	Delete "$SMPROGRAMS\$StartMenu.Location\Uninstall.lnk"
	RMDir "$SMPROGRAMS\$StartMenu.Location"
  
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
	Delete "$INSTDIR\SnK.CHANGELOG.TXT"
	Delete "$INSTDIR\SnK.README.TXT"
	Delete "$INSTDIR\SnK.LICENSE.TXT"
	Delete "$INSTDIR\SnK.DetectMatrix.html"
	Delete "$INSTDIR\${UNINST_NAME}"
	RMDir "$INSTDIR"
	
	${if} ${FileExists} "$INSTDIR\HotkeySuite.exe"
		MessageBox MB_OK|MB_ICONINFORMATION 'Unable to delete "$INSTDIR\HotkeySuite.exe"$\nMaybe an instance of the program is still running. Please close it and delete file manually.' /SD IDOK
	${endif}
SectionEnd

!ifdef INST64
	LangString DESC_Grp_HS ${LANG_ENGLISH} "Install 64-bit version of HotkeySuite."
	LangString DESC_Sec_SNK ${LANG_ENGLISH} "Install bundled SnK distribution (v${SNKVER_1}.${SNKVER_2}${SNK_DEV_BUILD} x64). If you don't want to install it, you should download it separately and set SnkPath variable in HotkeySuite.ini accordingly."
!else
	LangString DESC_Grp_HS ${LANG_ENGLISH} "Install 32-bit version of HotkeySuite."
	LangString DESC_Sec_SNK ${LANG_ENGLISH} "Install bundled SnK distribution (v${SNKVER_1}.${SNKVER_2}${SNK_DEV_BUILD}). If you don't want to install it, you should download it separately and set SnkPath variable in HotkeySuite.ini accordingly."
!endif
LangString DESC_Sec_HS ${LANG_ENGLISH} "HotkeySuite main distribution - executable with docs."
LangString DESC_Sec_DEF_SCRIPT ${LANG_ENGLISH} "Default SnK script to run on single hotkey press. You can check what this script does by launching HotkeySuite and editing single press event. Script will be installed only for the current user."
LangString DESC_Sec_DEF_SCRIPT2 ${LANG_ENGLISH} "Default SnK script to run on single hotkey press. Warning: this will overwrite you current single press event script!"
LangString DESC_Sec_AUTORUN ${LANG_ENGLISH} "Add HotkeySuite to Autorun (pre-Vista) or schedule it using Task Scheduler (Vista and above)."
LangString DESC_Sec_PATH ${LANG_ENGLISH} "Add installation directory to PATH variable to make HotkeySuite and bundeled SnK available from command prompt."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Grp_HS} $(DESC_Grp_HS)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_HS} $(DESC_Sec_HS)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_DEF_SCRIPT} $(DESC_Sec_DEF_SCRIPT)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_DEF_SCRIPT2} $(DESC_Sec_DEF_SCRIPT2)
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
	
	;Hack to get SetShellVarContext-independent APPDATA
	;By default (on init) SetShellVarContext is set to "current" so $APPDATA points to %APPDATA%
	StrCpy $USER_APPDATA "$APPDATA"
	
	;If on_hotkey.txt already exists - don't offer to install default SnK script
	${if} ${FileExists} "$USER_APPDATA\${APPNAME}\on_hotkey.txt"
		SectionSetText ${Sec_DEF_SCRIPT} ""
		SectionSetFlags ${Sec_DEF_SCRIPT} 0x0
		SectionSetText ${Sec_DEF_SCRIPT2} "Default SnK Script"
	${endif}
	
	;Don't loose $INSTDIR set with /D
	${if} "$INSTDIR" != "\${APPNAME}"
		StrCpy $D_INSTDIR "$INSTDIR"
	${endif}
	
	;Initialize MultiUser module
	!insertmacro MULTIUSER_INIT

	;Initialize Upgrade page variables
	ReadRegStr $R0 SHCTX "${MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY}" "${MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME}"
	${if} $R0 == ""
	${orif} "$D_INSTDIR" != ""
		StrCpy $UpgDialog.Status "Skip"
	${else}
		StrCpy $UpgDialog.Status ""
		StrCpy $UpgDialog.OrigPath $INSTDIR
		StrCpy $UpgDialog.OrigMode $MultiUser.InstallMode
	${endif}
	StrCpy $InstFeatures 0
	
	;Initialize StartMenu page variables
	StrCpy $StartMenu.Location ""	;If not changed by StartMenu page, forces MUI_STARTMENU_WRITE macros to get StartMenu path from registry
	ReadRegStr $R0 HKLM "${UNINST_KEY}" "${STARTMENU_LOC}"
	ReadRegStr $R1 HKCU "${UNINST_KEY}" "${STARTMENU_LOC}"
	${if} $R0 == ""
	${andif} $R1 == ""
		StrCpy $StartMenu.PrevMode ""	;Prevent MultiIser awareness if StartMenuLocation not set for both installation modes
	${else}
		StrCpy $StartMenu.PrevMode $MultiUser.InstallMode
	${endif}
	
	;On silent install make silent upgrade
	${if} $UpgDialog.Status == ""
	${andif} ${Silent}
		ReadRegStr $InstFeatures SHCTX "${UNINST_KEY}" "${INST_FEATURES}"
		IntOp $R1 $InstFeatures & 1
		IntOp $R2 $InstFeatures & 2
		IntOp $R4 $InstFeatures & 4
		IntOp $R8 $InstFeatures & 8
		${if} $R1 == 1
			SectionSetFlags ${Sec_AUTORUN} ${SF_SELECTED}
		${else}
			SectionSetFlags ${Sec_AUTORUN} 0x0
		${endif}
		${if} $R2 == 2
			SectionSetFlags ${Sec_SNK} ${SF_SELECTED}
		${else}
			SectionSetFlags ${Sec_SNK} 0x0
		${endif}
		${if} $R4 == 4
			SectionSetFlags ${Sec_PATH} ${SF_SELECTED}
		${else}
			SectionSetFlags ${Sec_PATH} 0x0
		${endif}
		${if} $R8 != 8
			StrCpy $StartMenu.Location ">"	;This will prevent MUI_STARTMENU_WRITE macros from creating StartMenu
		${endif}
		;If during installation default script was installed, installer won't offer to reinstall it - section will be already disabled during upgrade
		;If default script wasn't installed, silent upgrade shoudn't install it either
		SectionSetFlags ${Sec_DEF_SCRIPT} 0x0
	${endif}
FunctionEnd

Function un.onInit
	;Hack to get SetShellVarContext-independent APPDATA
	;By default (on init) SetShellVarContext is set to "current" so $APPDATA points to %APPDATA%
	StrCpy $USER_APPDATA "$APPDATA"
	
	;Initialize MultiUser module
	!insertmacro MULTIUSER_UNINIT
FunctionEnd

Function upgradePage
	${if} $UpgDialog.Status == "Skip"
		Abort
	${endif}
	
	nsDialogs::Create 1018
	Pop $R0

	${if} $R0 == error
		Abort
	${endif}
		
	!insertmacro MUI_HEADER_TEXT "Upgrade Installation" "Upgrade existing version of SnK HotkeySuite."
	
	${NSD_CreateRadioButton} 20u 50u 280u 10u "Upgrade"
	Pop $UpgDialog.HWND_UPG
	${NSD_CreateRadioButton} 20u 70u 280u 10u "Uninstall"
	Pop $UpgDialog.HWND_UN
	
	${if} ${FileExists} "$UpgDialog.OrigPath\${UNINST_NAME}"
		${if} $UpgDialog.OrigMode == AllUsers
			${NSD_CreateLabel} 0u 0u 300u 20u "SnK HotkeySuite is already installed on this machine. You can upgrade installed SnK HotkeySuite version. Uninstall it. Or continue with normal installation."
			Pop $R0
		${else}
			${NSD_CreateLabel} 0u 0u 300u 20u "SnK HotkeySuite is already installed for the current user. You can upgrade installed SnK HotkeySuite version. Uninstall it. Or continue with normal installation."
			Pop $R0
		${endif}
	${else}
		EnableWindow $UpgDialog.HWND_UN 0
		${if} $UpgDialog.OrigMode == AllUsers
			${NSD_CreateLabel} 0u 0u 300u 20u "SnK HotkeySuite is already installed on this machine, but appears damaged. It is advised to upgrade installed SnK HotkeySuite. Or you can continue with normal installation."
			Pop $R0
		${else}
			${NSD_CreateLabel} 0u 0u 300u 20u "SnK HotkeySuite is already installed for the current user, but appears damaged. It is advised to upgrade installed SnK HotkeySuite. Or you can continue with normal installation."
			Pop $R0
		${endif}
	${endif}
	
	${NSD_CreateRadioButton} 20u 90u 280u 10u "Continue"
	Pop $UpgDialog.HWND_CNT
	
	${if} $UpgDialog.Status == ""
	${orif} $UpgDialog.Status == "Upgrade"
		SendMessage $UpgDialog.HWND_UPG ${BM_SETCHECK} ${BST_CHECKED} 0
	${elseif} $UpgDialog.Status == "Uninstall"
		SendMessage $UpgDialog.HWND_UN ${BM_SETCHECK} ${BST_CHECKED} 0
	${elseif} $UpgDialog.Status == "Continue"
		SendMessage $UpgDialog.HWND_CNT ${BM_SETCHECK} ${BST_CHECKED} 0
	${endif}
	
	nsDialogs::Show
FunctionEnd

Function upgradePageLeave
	${NSD_GetState} $UpgDialog.HWND_UPG $R0
	${NSD_GetState} $UpgDialog.HWND_UN $R1
	${NSD_GetState} $UpgDialog.HWND_CNT $R2
	
	${if} $R0 == ${BST_CHECKED}
		;If we are returning from Continue - reapply MULTIUSER_INIT to set right registry hive and INSTDIR
		${if} $UpgDialog.Status == "Continue"
			${if} $UpgDialog.OrigMode == AllUsers
				Call MultiUser.InstallMode.AllUsers
			${else}
				Call MultiUser.InstallMode.CurrentUser
			${endif}
		${endif}

		StrCpy $UpgDialog.Status "Upgrade"
		ReadRegStr $InstFeatures SHCTX "${UNINST_KEY}" "${INST_FEATURES}"
		IntOp $R3 ${SF_SELECTED} | ${SF_RO}
		IntOp $R1 $InstFeatures & 1
		IntOp $R2 $InstFeatures & 2
		IntOp $R4 $InstFeatures & 4
		IntOp $R8 $InstFeatures & 8
		${if} $R1 == 1
			SectionSetFlags ${Sec_AUTORUN} $R3
		${endif}
		${if} $R2 == 2
			SectionSetFlags ${Sec_SNK} $R3
		${endif}
		${if} $R4 == 4
			SectionSetFlags ${Sec_PATH} $R3
		${endif}
		${if} $R8 == 8
			;This resets MUI_PAGE_STARTMENU to default (enabled) state with path taken from registry
			;Lock is achieved by skipping page in customStartMenuPageIni function
			StrCpy $StartMenu.Location ""
		${endif}
	${elseif} $R1 == ${BST_CHECKED}
		StrCpy $UpgDialog.Status "Uninstall"
		;We may return here from Continue - do not rely on SHCTX
		${if} $UpgDialog.OrigMode == AllUsers
			ReadRegStr $R2 HKLM "${UNINST_KEY}" "UninstallString"
		${else}
			ReadRegStr $R2 HKCU "${UNINST_KEY}" "UninstallString"
		${endif}
		Exec "$R2"
		Quit
	${elseif} $R2 == ${BST_CHECKED}
		StrCpy $UpgDialog.Status "Continue"
		;Reset features enabled and locked by upgrade
		${if} $InstFeatures != 0
			IntOp $R1 $InstFeatures & 1
			IntOp $R2 $InstFeatures & 2
			IntOp $R4 $InstFeatures & 4
			IntOp $R8 $InstFeatures & 8
			${if} $R1 == 1
				SectionSetFlags ${Sec_AUTORUN} ${SF_SELECTED}
			${endif}
			${if} $R2 == 2
				SectionSetFlags ${Sec_SNK} ${SF_SELECTED}
			${endif}
			${if} $R4 == 4
				SectionSetFlags ${Sec_PATH} 0x0
			${endif}
			${if} $R8 == 8
				StrCpy $StartMenu.Location ""	;This resets MUI_PAGE_STARTMENU to default (enabled) state with path taken from registry
			${endif}
			StrCpy $InstFeatures 0
		${endif}
	${endif}
FunctionEnd

Function skipPageIfUpgrade
	${if} $UpgDialog.Status == "Upgrade"
		Abort
	${endif}
FunctionEnd

Function changeNextToInstOnUpgrade
	IntOp $R0 $InstFeatures & 8
	${if} $R0 == 8
		GetDlgItem $R1 $HWNDPARENT 1
		SendMessage $R1 ${WM_SETTEXT} 0 "STR:$(^InstallBtn)"
	${endif}
FunctionEnd

Function customStartMenuPageIni
	IntOp $R0 $InstFeatures & 8
	${if} $R0 == 8
		Abort
	${else}
		${if} $StartMenu.PrevMode != ""
			;If installation mode switched - set default StartMenu path but keep checkbox state
			${if} $StartMenu.PrevMode != $MultiUser.InstallMode
				ReadRegStr $R0 SHCTX "${UNINST_KEY}" "${STARTMENU_LOC}"
				StrCpy $R1 $StartMenu.Location 1
				${if} $R1 == ">"
					StrCpy $StartMenu.Location "$R1$R0"
				${else}
					StrCpy $StartMenu.Location "$R0"
				${endif}
			${endif}
			StrCpy $StartMenu.PrevMode $MultiUser.InstallMode
		${endif}
	${endif}
FunctionEnd

Function killExistingSnK
	;Can fail if SnK was installed at non-default location and HotkeySuite was never run for current user or non-default INI path is used
	${if} ${FileExists} "$INSTDIR\HotkeySuite.exe"
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
	${endif}
FunctionEnd

Function un.killInstalledSnK
	;Can fail if SnK was installed at non-default location and HotkeySuite was never run for current user or non-default INI path is used
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
FunctionEnd

Function patchInstdir
	;If $INSTDIR was set with /D - reapply it
	${if} "$D_INSTDIR" != ""
		StrCpy $INSTDIR "$D_INSTDIR"
		Return
	${endif}

	;Though documentation for MultiUser.nsh states that MULTIUSER_USE_PROGRAMFILES64 define is supported, version of the plugin included with NSIS v3.01 doesn't support it
	!ifdef INST64
		${if} ${RunningX64}
		${andif} $MultiUser.InstallMode == AllUsers
		${andif} $MultiUser.InstDir == ""
			StrCpy $INSTDIR "$PROGRAMFILES64\${APPNAME}"
			Return
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
