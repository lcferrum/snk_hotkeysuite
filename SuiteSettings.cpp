#include "SuiteSettings.h"

SuiteSettings::SuiteSettings():
	long_press(false), mod_key(ModKeyType::CTRL_ALT), binded_vk(VK_BACK), binded_sc(0x0E), initial_hkl(GetKeyboardLayout(0)),
	shk_cfg_path(L"on_hotkey.cfg"), lhk_cfg_path(L"on_hotkey_long_press.cfg"), snk_path(L"SnKh.exe")
{}
	
SuiteSettings::~SuiteSettings()
{}

SuiteSettingsReg::SuiteSettingsReg():
	SuiteSettings()
{
	LoadSettingsFromReg();
}

SuiteSettingsReg::~SuiteSettingsReg()
{}

SuiteSettingsIni::SuiteSettingsIni():
	SuiteSettings(), ini_path(L"HotkeySuite.ini")
{
	LoadSettingsFromIni();
}

SuiteSettingsIni::SuiteSettingsIni(const std::wstring &ini_path):
	SuiteSettings(), ini_path(ini_path)
{
	LoadSettingsFromIni();
}

SuiteSettingsIni::~SuiteSettingsIni()
{}

void SuiteSettingsReg::LoadSettingsFromReg()
{}

void SuiteSettingsReg::SaveSettings()
{}

void SuiteSettingsIni::LoadSettingsFromIni()
{}

void SuiteSettingsIni::SaveSettings()
{}
