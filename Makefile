# Description:  Medialib Build System
# Author:       Alicia Amarilla (smushyaa@gmail.com)
# File Created: March 12, 2024

MEDIA_LIB_VERSION_MAJOR=0
MEDIA_LIB_VERSION_MINOR=1
MEDIA_LIB_VERSION_PATCH=0

OUTPUT_PATH     := $(if $(OUTPUT_PATH_OVERRIDE),$(OUTPUT_PATH_OVERRIDE),./build)
OUTPUT_OBJ_PATH := $(OUTPUT_PATH)/obj

LIBCORE_NAME ?= core
LIBCORE_PATH ?= $(OUTPUT_PATH)

MAKEFLAGS += -s

SO_EXT := so

ifeq ($(OS), Windows_NT)
	RUNNING_ON_WINDOWS := 1
	SO_EXT := dll
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		RUNNING_ON_LINUX := 1
	else
		RUNNING_ON_MACOS := 1
	endif
endif

ifndef $(TARGET_PLATFORM)
	ifeq ($(RUNNING_ON_WINDOWS),1)
		TARGET_PLATFORM := win32
		ifneq '$(findstring ;,$(PATH))' ';'
			WIN32_SHELL := $(shell uname 2>/dev/null || echo Unknown)
			WIN32_SHELL := $(patsubst CYGWIN%,Cygwin,$(WIN32_SHELL))
			WIN32_SHELL := $(patsubst MSYS%,MSYS,$(WIN32_SHELL))
			WIN32_SHELL := $(patsubst MINGW%,MSYS,$(WIN32_SHELL))
			ifeq ($(WIN32_SHELL), MSYS)
				MINGW := 1
			endif
		endif
	else
		ifeq ($(UNAME_S), Linux)
			TARGET_PLATFORM := linux
		else
			TARGET_PLATFORM := macos
		endif
	endif
	TARGET_PLATFORM_IS_CURRENT_PLATFORM := 1
endif

ifndef $(TARGET_ARCH)
	ifeq ($(TARGET_PLATFORM_IS_CURRENT_PLATFORM),)
		# TODO(alicia): more robust method of obtaining default arch target
		TARGET_ARCH := $(shell uname -m)
	else
		ifeq ($(TARGET_PLATFORM),win32)
			TARGET_ARCH := x86_64
		else
			ifeq ($(TARGET_PLATFORM),linux)
				TARGET_ARCH := x86_64
			else
				TARGET_ARCH := arm64
			endif
		endif
	endif
endif

TARGET_PLATFORM_VALID := $(filter win32 linux macos ios android, $(TARGET_PLATFORM))
TARGET_ARCH_VALID     := $(filter x86 x86_64 arm arm64, $(TARGET_ARCH))

ifeq ($(TARGET_ARCH_VALID),)
	ERROR_MSG := $(error $(TARGET_ARCH) is not a supported target architecture!)
endif

ifeq ($(TARGET_PLATFORM_VALID),)
	ERROR_MSG := $(error $(TARGET_PLATFORM) is not a supported target platform!)
else
	INVALID_PLATFORM_ARCH := $(TARGET_PLATFORM) with $(TARGET_ARCH) is not supported!
	ifeq ($(TARGET_PLATFORM),win32)
		ifeq ($(TARGET_ARCH),arm)
			ERROR_MSG := $(error $(INVALID_PLATFORM_ARCH))
		endif
	else
		ifeq ($(TARGET_PLATFORM),linux)
			ifeq ($(TARGET_ARCH),arm)
				ERROR_MSG := $(error $(INVALID_PLATFORM_ARCH))
			endif
		else
			ifeq ($(TARGET_PLATFORM),android)
				TARGET_ARCH_VALID := $(filter arm arm64, $(TARGET_ARCH))
				ifeq ($(TARGET_ARCH_VALID),)
					ERROR_MSG := $(error $(INVALID_PLATFORM_ARCH))
				endif
			else
				# macos or ios
				TARGET_ARCH_VALID := $(filter arm64, $(TARGET_ARCH))
				ifeq ($(TARGET_ARCH_VALID),)
					ERROR_MSG := $(error $(INVALID_PLATFORM_ARCH))
				endif
			endif
		endif
	endif
endif

ifeq ($(SIMPLE_NAME),1)
	TARGET_NAME := libmedia
else
	ifeq ($(NAME_OVERRIDE),)
		TARGET_NAME := libmedia-$(TARGET_PLATFORM)-$(TARGET_ARCH)-$(if $(RELEASE),release,debug)
	else
		TARGET_NAME := $(NAME_OVERRIDE)
	endif
endif

TARGET      := $(OUTPUT_PATH)/$(TARGET_NAME).$(SO_EXT)
TEST_TARGET := $(OUTPUT_PATH)/test-$(TARGET_NAME)

all: $(TARGET) clean_build_junk
	@echo "Make: "$(TARGET_NAME)" compiled successfully!"

clean_build_junk: $(TARGET)
	@rm $(SOURCES_FILE)

test: $(TEST_TARGET)
	@echo "Make: running tests . . ."
	cd $(OUTPUT_PATH); ../$(TEST_TARGET).exe

print_info:
	@echo "Make: Configuration:"
	@echo "Make:    Compiler: clang"
	@echo "Make:    Standard: C11"
	@echo "Make:    Target:   "$(TARGET_PLATFORM)_$(TARGET_ARCH)
	@echo "Make:    Output:   "$(TARGET)
	@echo "Make: Dependencies:"
	@echo "Make:    corelib path: "$(LIBCORE_PATH)/lib$(LIBCORE_NAME).$(SO_EXT)

recurse = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call recurse,$d/,$2))

COMPILER_PATH := clang

# check if compiler is in path
ifeq (, $(shell which $(COMPILER_PATH) 2> /dev/null))
	ERROR_MSG := $(error $(COMPILER_PATH) is not available! make sure it's installed/is in path!)
endif

STD_VERSION := -std=c11

SOURCES      := $(call recurse,impl,*.c) ./corelib/impl/platform_dllmain.c
SOURCES_FILE := $(OUTPUT_OBJ_PATH)/media_temp.c

CFLAGS        :=
CPPFLAGS      := MEDIA_ENABLE_EXPORT 
LDFLAGS       :=
INCLUDE       :=
OUTPUT_FILE   :=
WARNING_FLAGS :=

CPPFLAGS += $(if $(MEDIA_ENABLE_LOGGING),MEDIA_ENABLE_LOGGING,)

DEFINED := $(CPPFLAGS)

CPPFLAGS += MEDIA_LIB_VERSION_MAJOR=$(MEDIA_LIB_VERSION_MAJOR)
CPPFLAGS += MEDIA_LIB_VERSION_MINOR=$(MEDIA_LIB_VERSION_MINOR)
CPPFLAGS += MEDIA_LIB_VERSION_PATCH=$(MEDIA_LIB_VERSION_PATCH)

TEST_CPPFLAGS := $(subst MEDIA_ENABLE_EXPORT,,$(CPPFLAGS))
INCLUDEFLAGS  := -I. -I./corelib

OUTPUT_FILE      := -o $(TARGET)
TEST_OUTPUT_FILE := -o $(TEST_TARGET)

CFLAGS += $(if $(OPTIMIZED),-O2 -ffast-math,-O0)

ifeq ($(MINGW),1)
	CFLAGS += $(if $(NO_SYMBOLS),,-g -gcodeview)
else
	CFLAGS += $(if $(NO_SYMBOLS),,-g)
endif

WARNING_FLAGS += -Wall -Wextra -Werror -Werror=vla
CFLAGS   += $(WARNING_FLAGS)
LDFLAGS  += -shared
LDFLAGS  += $(if $(MEDIA_ENABLE_STDLIB),,-nostdlib)

LDFLAGS += -mno-stack-arg-probe -L$(LIBCORE_PATH) -l$(LIBCORE_NAME)

ifeq ($(MINGW),1)
	LDFLAGS += -fuse-ld=lld $(if $(NO_SYMBOLS),,-Wl,//debug)
	LDFLAGS += -Wl,//stack:0x100000

	ifeq ($(TARGET_PLATFORM),win32)
		LDFLAGS += -lkernel32
	endif
endif

TEST_CPPFLAGS := $(addprefix -D,$(TEST_CPPFLAGS))
DEF           := $(addprefix -D,$(CPPFLAGS))
CPPFLAGS      := $(DEF) -DMEDIA_COMMAND_LINE=\""$(COMPILER_PATH) $(STD_VERSION) $(SOURCES_FILE) $(COUT) $(INCLUDEFLAGS) $(CFLAGS) $(DEF)"\"

DISPLAY_COMMAND_LINE := $(COMPILER_PATH) $(STD_VERSION) $(SOURCES_FILE) $(OUTPUT_FILE) $(INCLUDEFLAGS) $(CFLAGS) $(DEF) $(LDFLAGS)
COMMAND_LINE         := $(COMPILER_PATH) $(STD_VERSION) $(SOURCES_FILE) $(OUTPUT_FILE) $(INCLUDEFLAGS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

TEST_FILTER_CFLAGS   := -fPIC
TEST_FILTER_LDFLAGS  := -shared -nostdlib
TEST_COMMAND_LINE := $(COMPILER_PATH) $(STD_VERSION)\
	./test/test.c $(TEST_OUTPUT_FILE) $(INCLUDEFLAGS)\
	$(filter-out $(TEST_FILTER_CFLAGS),$(CFLAGS))\
	$(TEST_CPPFLAGS)\
	$(filter-out $(TEST_FILTER_LDFLAGS),$(LDFLAGS))\
	-L./build -l$(subst lib,,$(TARGET_NAME))

$(TARGET): ./build/libcore.dll print_info $(OUTPUT_OBJ_PATH) $(SOURCES_FILE)
	@echo "Make: "$(DISPLAY_COMMAND_LINE)
	$(COMMAND_LINE)

$(LIBCORE_PATH)/lib$(LIBCORE_NAME).$(SO_EXT):
	@$(MAKE) --directory=corelib OUTPUT_PATH_OVERRIDE=../build SIMPLE_NAME=1

$(TEST_TARGET): $(TARGET)
	@echo "Make: "$(TEST_COMMAND_LINE)
	$(TEST_COMMAND_LINE)

$(OUTPUT_OBJ_PATH): 
	@echo "Make: creating output path '$(OUTPUT_PATH)' . . ."
	@mkdir $(OUTPUT_OBJ_PATH) -p

$(SOURCES_FILE): $(OUTPUT_OBJ_PATH) $(SOURCES)
	@echo "Make: generating sources file . . ."
	@echo "/* Generated on: "$(shell date)" */" > $(SOURCES_FILE)
	@echo "" >> $(SOURCES_FILE)
	for i in $(SOURCES); do echo "#include \""$$i"\"" >> $(SOURCES_FILE); done

DOXYFILE_DEFAULT := $(file <./docs/Doxyfile_default)
DOXYFILE_LOCAL   := Doxyfile_$(TARGET_PLATFORM)_$(TARGET_ARCH)
DOXYFILE         := ./docs/$(DOXYFILE_LOCAL)

DOXYFILE_SETTINGS := PREDEFINED += $(DEFINED)

ifeq ($(COMPILER),clang)
	DOXYFILE_SETTINGS += __clang__
endif
ifeq ($(TARGET_PLATFORM),win32)
	DOXYFILE_SETTINGS += _WIN32
endif
ifeq ($(TARGET_PLATFORM),linux)
	DOXYFILE_SETTINGS += __linux__
endif
ifeq ($(TARGET_PLATFORM),macos)
	DOXYFILE_SETTINGS += __APPLE__ TARGET_OS_MAC
endif
ifeq ($(TARGET_PLATFORM),android)
	DOXYFILE_SETTINGS += __ANDROID__
endif
ifeq ($(TARGET_PLATFORM),ios)
	DOXYFILE_SETTINGS += __APPLE__ TARGET_OS_IPHONE
endif
ifeq ($(TARGET_ARCH),x86)
	DOXYFILE_SETTINGS += __i386__
	ifeq ($(MINGW),1)
		DOXYFILE_SETTINGS += __MINGW32__
	endif
endif
ifeq ($(TARGET_ARCH),x86_64)
	DOXYFILE_SETTINGS += __x86_64__
	ifeq ($(MINGW),1)
		DOXYFILE_SETTINGS += __MINGW64__
	endif
endif
ifeq ($(TARGET_ARCH),arm)
	DOXYFILE_SETTINGS += __arm__
	ifeq ($(MINGW),1)
		DOXYFILE_SETTINGS += __MINGW32__
	endif
endif
ifeq ($(TARGET_ARCH),arm64)
	DOXYFILE_SETTINGS += __aarch64__
	ifeq ($(MINGW),1)
		DOXYFILE_SETTINGS += __MINGW64__
	endif
endif

define DOXYFILE_APPEND
$(DOXYFILE_SETTINGS)
PROJECT_NUMBER = $(MEDIA_LIB_VERSION_MAJOR).$(MEDIA_LIB_VERSION_MINOR).$(MEDIA_LIB_VERSION_PATCH)
endef

ifeq ($(RUNNING_ON_WINDOWS),1)
	ifneq (,$(shell which pwsh 2> /dev/null))
		OPEN_DOCS    := pwsh -Command Invoke-Expression ./docs/html/index.html
		DOCS_BROWSER := "default browser"
	endif
else
	ifeq (,$(shell which firefox 2> /dev/null))
		ifneq (,$(shell which google-chrome 2> /dev/null))
			OPEN_DOCS    := google-chrome ./docs/html/index.html
			DOCS_BROWSER := "google chrome"
		endif
	else
		OPEN_DOCS    := firefox ./docs/html/index.html
		DOCS_BROWSER := "firefox"
	endif
endif

docs:
	$(if $(shell which doxygen 2> /dev/null),,$(error doxygen is not available! make sure it's installed/is in path before generating docs!))
	@echo "Make: generating documentation . . ."
	$(file >$(DOXYFILE),$(DOXYFILE_DEFAULT)$(DOXYFILE_APPEND))
	@cd ./docs; doxygen $(DOXYFILE_LOCAL) -q
	@echo "Make: documentation generated at path ./docs/html/index.html"
	$(if $(OPEN_DOCS),echo "Make: opening documentation in $(DOCS_BROWSER) . . .",)
	$(OPEN_DOCS)

LSP_FLAGS := $(COMPILER_PATH) $(STD_VERSION) $(subst -I.,-I..,$(INCLUDEFLAGS)) $(DEF) -DCORE_COMMAND_LINE=\"\" -D_CLANGD $(WARNING_FLAGS) -nostdinc
lsp:
	@echo "Make: generating compile_flags.txt for clangd . . ."
	-@rm -f ./core/compile_flags.txt 2> /dev/null || true
	-@rm -f ./impl/compile_flags.txt 2> /dev/null || true
	for i in $(LSP_FLAGS);\
		do echo $$i >> ./core/compile_flags.txt;\
	done
	@cp ./core/compile_flags.txt ./impl/compile_flags.txt

help:
	@echo "Make recipes:"
	@echo "   all      compile project for current platform."
	@echo "   docs     run doxygen to generate documentation."
	@echo "            documentation generated takes compiler options into account."
	@echo "            windows:     if pwsh is in path, opens docs automatically in default browser"
	@echo "            linux/macos: if firefox or google-chrome is in path, opens docs automatically"
	@echo "   test     run tests to see if there are any errors (runs all first)."
	@echo "   lsp      generate compile_flags.txt"
	@echo "   help     print this message."
	@echo "   help_ex  print extended help message."
	@echo "   clean    clean output directory."
	@echo
	@echo "Compilation options for medialib:"
	@echo "   LIBCORE_NAME=[name]             set name of libcore to compile/link to (default=core)."
	@echo "   LIBCORE_PATH=[dir]              set path to libcore directory to skip compiling libcore (default=none)."
	@echo "   TARGET_PLATFORM=[platform]      set target platform (default=current)"
	@echo "                                   valid: win32, linux, macos, ios, android"
	@echo "   TARGET_ARCH=[arch]              set target architecture (default=current)"
	@echo "                                   valid: x86(win32, linux), x86_64(win32, linux), arm(android), arm64(win32, linux, macos, ios, android)"
	@echo "   OUTPUT_PATH_OVERRIDE=[path]     override output path. (default=./build)"
	@echo "   NAME_OVERRIDE=[name]            override library output name. (default=libcore-platform-arch-release_mode)"
	@echo "   SIMPLE_NAME=1                   output name is simply libcore. (takes precedence over NAME_OVERRIDE)"
	@echo "   RELEASE=1                       build in release mode. does not actually do anything other than makes generated output path /release rather than /debug."
	@echo "   OPTIMIZED=1                     build with optimizations on (default=off)."
	@echo "   NO_SYMBOLS=1                    build without debug symbols (default=with symbols, .pdb on Windows, DWARF otherwise)."

clean:
	@echo "Make: cleaning output path "$(OUTPUT_PATH)" . . ."
	-@rm -rf $(OUTPUT_PATH)/* 2> /dev/null || true
	@echo "Make: cleaning ./docs/html . . ."
	-@rm -f ./docs/html/* -r 2> /dev/null || true

err: ; $(ERROR_MSG)

.PHONY: all docs test lsp help help_ex clean build_win32_x86 build_linux_x86 build_macos_arm build_wasm64 err clean_build_junk spit


