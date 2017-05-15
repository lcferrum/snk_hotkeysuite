Search and Kill HotkeySuite

1. License
----------
Copyright (c) 2016-2017 Lcferrum

This program comes with no warranty.
You must use this program at your own risk.
Licensed under BSD license - see LICENSE.TXT file for details.

2. About
--------
HotkeySuite is a resident program that will launch SnK when certain key
combination is pressed. When launched, program will reside in tray - various
launch options, such as key combination and actual SnK command line, can be
changed through it's tray icon menu.

What is SnK? Search and Kill is a console tool that automatically kills Windows
processes using search criteria supplied via command line. You can learn more
about SnK on it's GitHub project page: https://github.com/lcferrum/snk

3. Where to get
---------------
You can compile SnK HotkeySuite by yourself (refer to COMPILE.TXT that comes 
with the sources) or download binary distribution (in the form of installer or
ZIP archive) from Sourceforge:

    https://sourceforge.net/projects/sandk/files/SnK_HotkeySuite/
    
Main project homepage is at GitHub:

    https://github.com/lcferrum/snk_hotkeysuite

4. Installation
---------------
It is recommended to install and update HotkeySuite using installer with
default install options. When using ZIP archive you can create portable
installation. SnK is bundled with HotkeySuite installer, but not included with
ZIP archive - if it's not already installed on the system, you should download
it separately if ZIP archive was used for installation.

If installer was used, HotkeySuite can be uninstalled using Uninstall shortcut
in Start menu HotkeySuite folder (if it was created during install) or from
Control Panel (in both cases, this will also uninstall bundled SnK). If
HotkeySuite was installed from ZIP archive - no actual uninstallation is
needed, just delete whatever was copied from archive with HotkeySuite settings
file (.ini) and SnK scripts (.txt) stored in the same folder.

5. Usage
--------
If HotkeySuite was installed with installer using default options - it should
run automatically at user logon. If you have downloaded HotkeySuite in the form
of ZIP archive or choose not to run it automatically during install, launch it
manually using executable or link in Start menu (if it was created during
install). 

If SnK is not found by HotkeySuite, it won't run and will ask to select valid 
windowless SnK executable. This may happen if you choose not to install bundled
SnK distribution during install or downloaded HotkeySuite in the form of ZIP
archive. In this case, select windowless SnK version present on the system or
download SnK separately and place it's binaries in the same folder where
HotkeySuite was installed.

Settings are stored in INI file. If HotkeySuite was installed with installer,
INI is located in user or common AppData folder (depending on installation
type). Running HotkeySuite copied from ZIP archive creates portable
installation - INI file will be stored in the same folder as HotkeySuite
binary. Latter works only if HotkeySuite was not previously installed with
installer, otherwise INI created by installer will be used instead. By default
SnK scripts are stored in the same folder as INI file, but you can place them
elsewhere and edit INI accordingly. 

Two environment variables are provided by HotkeySuite that you can use in INI
path settings: %HS_EXE_PATH% and %HS_INI_PATH% - they provide absolute paths to
executable and INI folders accordingly (w/o trailing backslash). Besides these
two variables, you can use any other environment variable that is available at
execution time.

When HotkeySuite is running - it's icon is present in the system tray. Right
click on it to bring up menu. Double click to quickly stop/start HotkeySuite.
Stopping HotkeySuite doesn't exit application, but disables key combination
used to launch SnK.

In HotkeySuite menu you can choose to start/stop application, exit it or change
various settings: enable/disable long press detection, change key combination
used to launch SnK and edit SnK scripts used by launched SnK.

5. Usage example
----------------                              
TBD
