!include MUI2.nsh

OutFile "HotkeySuiteSetup.exe"
Name "SnK HotkeySuite"
InstallDir "$LOCALAPPDATA\Modern UI Test"
BrandingText " "
RequestExecutionLevel highest

!define MUI_ICON "hs.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.TXT"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES 
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "SnK HotkeySuite" Section_HS
  SectionIn RO

  SetOutPath $INSTDIR
  SetOverwrite on
  
  MessageBox MB_OK $INSTDIR
SectionEnd

Section "SnK" Section_SNK
  MessageBox MB_OK "LOLOK"
SectionEnd

Section "Default SnK Script" Section_DEF_SCRIPT
  MessageBox MB_OK "LOLOK"
SectionEnd

LangString DESC_Section_HS ${LANG_ENGLISH} "Description of section 1."
LangString DESC_Section_SNK ${LANG_ENGLISH} "Description of section 2."
LangString DESC_Section_DEF_SCRIPT ${LANG_ENGLISH} "Description of section 3."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_HS} $(DESC_Section_HS)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_SNK} $(DESC_Section_SNK)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_DEF_SCRIPT} $(DESC_Section_DEF_SCRIPT)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
