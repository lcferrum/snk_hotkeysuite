# Usage:
# make BUILD=COMPILER HOST=OS_TYPE
#	Makes SnK HotkeySuite binary
# make clean
#	Cleans directory of executables
# make upx
#	Pack executables with upx
# make UPSTREAM_INC=PATH
#	Change include path for clang++ 3.6.2 and g++ 4.7.2 (default is /c/cygwin/usr/i686-w64-mingw32/sys-root/mingw/include/)
# make DEBUG=LEVEL
#	Makes debug build

# Conditionals
ifeq (,$(if $(filter-out upx install clean,$(MAKECMDGOALS)),,$(MAKECMDGOALS)))
ifeq (,$(and $(filter $(BUILD),MinGW-w64 MinGW-w64_pthreads Clang_362),$(filter $(HOST),x86-64 x86)))
$(info Compiler and/or OS type is invalid! Please correctly set BUILD and HOST variables.)
$(info Possible BUILD values: MinGW-w64, MinGW-w64_pthreads, Clang_362)
$(info Possible HOST values: x86-64, x86)
$(error BUILD/HOST is invalid)
endif
endif

# Common section
RM=rm -f
UPX=upx
CFLAGS=-std=c++11 -D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE
LDFLAGS=-static-libgcc -static-libstdc++ -lole32 -loleaut32 -lgdi32 -Wl,--enable-stdcall-fixup
UPSTREAM_INC=/c/cygwin/usr/i686-w64-mingw32/sys-root/mingw/include/
SRC=Suite.cpp SuiteCommon.cpp SuiteExterns.cpp SuiteExternalRelations.cpp SuiteMain.cpp TaskbarNotificationAreaIcon.cpp HotkeyEngine.cpp SuiteHotkeyFunctions.cpp AsmHotkeyFunctions.S SuiteSettings.cpp SuiteAboutDialog.cpp SuiteBindingDialog.cpp Res.rc
OBJ=$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.rc,%.o,$(SRC))))
TARGET=HotkeySuite.exe

MAKENSIS=makensis.exe /V2
NSISSRC=SuiteInstaller.nsi
NSISTARGET=HotkeySuiteSetup*.exe

# Debug specific common section
ifdef DEBUG
	CFLAGS+=-DDEBUG=$(DEBUG) -g
	LDFLAGS+=-g
else
	CFLAGS+=-O2
	LDFLAGS+=-O2 -s
endif

# Compiler/OS specific sections
# N.B.:
# i386 is minimum system requirement for Windows 95, maximum arch for apps is pentium2 (OS doesn't handle SSE instructions without patch)
# i486 is minimum system requirement for Windows NT4, maximum arch for apps is pentium2 (OS doesn't handle SSE instructions)
# pentium is minimum system requirement for Windows 2000

# Current MinGW-w64 with Win32 threads
# MinGW-w64 minimum supported target 32-bit Windows version is Windows 2000
# pentiumpro is MinGW-w64 default arch for 32-bit compiler
ifeq ($(BUILD),MinGW-w64)
	LDFLAGS+=-mwindows -municode
ifeq ($(HOST),x86-64)
	CC=x86_64-w64-mingw32-g++
	WINDRES=x86_64-w64-mingw32-windres
	MAKENSISFLAGS=/DINST64
	NSISTARGET=HotkeySuiteSetup64.exe
endif
ifeq ($(HOST),x86)
	CC=i686-w64-mingw32-g++
	WINDRES=i686-w64-mingw32-windres
	NSISTARGET=HotkeySuiteSetup32.exe
endif
endif

# Current MinGW-w64 with POSIX threads
# MinGW-w64 minimum supported target 32-bit Windows version is Windows 2000
# pentiumpro is MinGW-w64 default arch for 32-bit compiler
ifeq ($(BUILD),MinGW-w64_pthreads)
	LDFLAGS+=-static -lpthread -mwindows -municode
ifeq ($(HOST),x86-64)
	CC=x86_64-w64-mingw32-g++
	WINDRES=x86_64-w64-mingw32-windres
	MAKENSISFLAGS=/DINST64
	NSISTARGET=HotkeySuiteSetup64.exe
endif
ifeq ($(HOST),x86)
	CC=i686-w64-mingw32-g++
	WINDRES=i686-w64-mingw32-windres
	NSISTARGET=HotkeySuiteSetup32.exe
endif
endif

# Clang 3.6.2 with includes from current MinGW-w64
# pentium4 is Clang 3.6.2 default arch
ifeq ($(BUILD),Clang_362)
	CC=clang++
	WINDRES=windres
	INC=-I$(UPSTREAM_INC)
	CFLAGS+=-target i486-pc-windows-gnu -march=i486 -Wno-ignored-attributes -Wno-deprecated-register -Wno-inconsistent-dllimport -Wno-missing-declarations -Wno-unknown-attributes -DUMDF_USING_NTSTATUS -DOBSOLETE_WWINMAIN
	LDFLAGS+=-Wl,--subsystem,windows
	ifndef DEBUG
		CFLAGS+=-Wno-unused-value -Wno-macro-redefined
	endif
ifeq ($(HOST),x86-64)
$(error not implemented)
endif
ifeq ($(HOST),x86)
	NSISTARGET=HotkeySuiteSetup32.exe
endif
endif

.PHONY: all exe clean upx install
.INTERMEDIATE: $(OBJ)

all: exe upx install

exe: $(SRC) $(TARGET)

install: $(NSISSRC) $(NSISTARGET)

$(NSISTARGET): $(NSISSRC)
	$(MAKENSIS) $(MAKENSISFLAGS) $<

$(TARGET): $(OBJ) 
	$(CC) -o $@ $(OBJ) $(LDFLAGS)
	
%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

%.o: %.S
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)
	
%.o: %.rc
	$(WINDRES) $< $@ $(filter -D% -U% -I%,$(CFLAGS)) $(INC)
	
upx:
	$(UPX) $(TARGET) ||:

clean:
	$(RM) $(TARGET) $(NSISTARGET) $(OBJ) ||:
