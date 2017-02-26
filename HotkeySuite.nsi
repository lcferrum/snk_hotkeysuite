OutFile "HotkeySuiteSetup.exe"
!define APP_NAME "SnK HotkeySuite"
!define PRODUCT_NAME "${APP_NAME}"
!define VERSION "1.0"
!define PROGEXE "HotkeySuite.exe"
!define COMPANY_NAME "Lcferrum"
!define UNINSTALL_FILENAME "uninstall.exe"
!define MULTIUSER_INSTALLMODE_INSTDIR "${APP_NAME}"
!define MULTIUSER_INSTALLMODE_INSTALL_REGISTRY_KEY "${APP_NAME}" 
!define MULTIUSER_INSTALLMODE_UNINSTALL_REGISTRY_KEY "${APP_NAME}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "UninstallString"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION
!define MULTIUSER_INSTALLMODE_DEFAULT_ALLUSERS
!include NsisMultiUser.nsh
!include MUI2.nsh

!insertmacro MUI_PAGE_WELCOME
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MULTIUSER_UNPAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES 
!insertmacro MUI_LANGUAGE English

Section "SnK HotkeySuite"
  SectionIn RO

  SetOutPath $INSTDIR
  SetOverwrite on
  
  MessageBox MB_OK $INSTDIR
SectionEnd

Function .onInit
  !insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
FunctionEnd
