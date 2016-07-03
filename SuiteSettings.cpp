#include "SuiteSettings.h"

SuiteSettings::SuiteSettings():
	long_press(false), mod_key(ModKeyType::CTRL_ALT), binded_vk(VK_BACK), binded_sc(0x0E), initial_hkl(GetKeyboardLayout(0))
{}
	
SuiteSettings::~SuiteSettings()
{}
