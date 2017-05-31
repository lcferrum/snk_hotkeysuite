Search and Kill HotkeySuite

1. License
----------
Copyright (c) 2016-2017 Lcferrum

This program comes with no warranty.
You must use this program at your own risk.
Licensed under BSD license - see LICENSE.TXT file for details.

2. About
--------
SnK HotkeySuite is all-in-one solution for hung and unresponsive Windows apps
which hinder further user interaction with OS, complicate Task Manager use or
even force user to restart computer. SnK HotkeySuite, like Task Manager, is
activated with key combination. But in contrast to it, when fired, will not
require any more user input and will automatically search and kill troublesome
apps.

SnK HotkeySuite actually consists of two programs: HotkeySuite itself, a
resident program that detects key combination, and SnK (Search and Kill),
console tool that automatically kills Windows processes using search criteria
supplied via command line. When started, HotkeySuite will reside in tray and
launch SnK when certain key combination is pressed. Various launch options,
such as actual key combination and SnK command line, can be changed through
it's tray icon menu. You can learn more about SnK on it's GitHub project page:
https://github.com/lcferrum/snk

Check out 'Usage example' chapter in this README to find out typical SnK
HotkeySuite use case.

3. Where to get
---------------
You can compile SnK HotkeySuite by yourself (refer to COMPILE.TXT that comes 
with the sources) or download binary distribution (in the form of installer or
ZIP archive) from Sourceforge:

    https://sourceforge.net/projects/sandk/files/SnK%20HotkeySuite/
    
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
used to launch SnK and edit SnK scripts used when SnK is launched.

6. Usage example
----------------                       
For this example it's assumed that SnK HotkeySuite was installed using
installer with default options: SnK binaries are present and known to
HotkeySuite, HotkeySuite is already launched, default SnK script is used, key
combination is Ctrl+Alt+Backspace.

Default SnK script is specifically tailored to be used with fullscreen video
games, which are major source of nuisance associated with hung apps. It doesn't
mean that it won't work with other hung apps, but first it checks for any apps
running in fullscreen and only then switches to other hung apps detection
methods. You can check what exactly this script does by opening it: right click
on HotkeySuite icon in the tray and select "On single press..." menu item. This
will open the sctipt in default Windows TXT editor (usually Notepad). Here you
can edit it and save the changes. For now we will stick to it's default
contents.

Launch some fullscreen (exclusive or borderless windowed) video game that will
be used as guinea pig. Wait for it to load. Now press Ctrl+Alt+Backspace and
watch it die. Don't forget that any unsaved data (settings, game progress,
etc.) will be lost! As you can see, though the game hasn't hung it still was
killed by SnK because of fullscreen search criterion. SnK fully relies on the
search criteria supplied via script and doesn't check if found app is really
in hung state. So you should launch it only when you are sure that there really
is hung app on the system and it should be killed (at least this applies to
default script).

If you don't have any videogame installed, it's time for some advanced stuff.
Open SnK script (HotkeySuite "On single press..." menu item), comment current
script by placing number sign (#) exactly at the beginning of the line where
actual script starts (like all the comments above), add new line and type:

    +v /pth="calc.exe"
	
This will instruct SnK to kill Windows Calculator when launched. Save and close
the script, launch Calculator, press Ctrl+Alt+Backspace, and voila! Calculator
is killed. Once again, it is shown that SnK doesn't care if app was really
hung - it just do what is told in the script. Revert script back to it's
default state by deleting recently added line and uncommenting original script
(remove number sign). Don't forget to save the changes.
