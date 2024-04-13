.PHONY: clean upload prepare_zip build all

RAYLIB := raylib/rcore raylib/rshapes raylib/rtextures raylib/rtext raylib/rmodels raylib/utils raylib/raudio
#####################################################################
## PLATFORM DEFINITION
#####################################################################
PLATFORM ?= WIN

ifeq ($(PLATFORM),WEB)

ifndef SILENT
$(info Building for web...)
endif
CC := emcc
AR := emar
COMPILER_OPTS := -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib
PLAT_ID := web
WEB := 1
else ifeq ($(PLATFORM),WIN)
$(info Building for windows...)
CC := gcc
AR := ar
PLAT_ID := win
COMPILER_OPTS := -lopengl32 -lstdc++ -lgdi32 -lwinmm -lraylib -L./output/$(PLAT_ID)/ -I raylib -D_GNU_SOURCE -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -I raylib/external/glfw/include
DESKTOP := 1
RAYLIB += raylib/rglfw

else
$(error Bad PLATFORM specified '$(PLATFORM)'. Supported Targets: WEB, WIN)
endif

#####################################################################
## Common VARIABLES
#####################################################################

OUTPUT := output/$(PLAT_ID)
DIST_OUT := $(OUTPUT)/dist
RAYLIB_OBJS := $(foreach item,$(RAYLIB),$(OUTPUT)/$(item).o)


#####################################################################
## Primary Targets
#####################################################################
ifdef WEB
build: $(DIST_OUT)/index.html
endif
ifdef DESKTOP
build: $(DIST_OUT)/game.exe
endif
build:
ifndef SILENT
	$(info Distributions omitted to '$(DIST_OUT)')
endif

all: build

prepare_zip: $(DIST_OUT)/release.zip

upload: $(DIST_OUT)/release.zip
	butler push $(DIST_OUT)/release.zip gerolmed/internal-test-project:$(PLAT_ID)

clean:
	rm -rf output
	cd raylib; make clean RAYLIB_RELEASE_PATH=../

#####################################################################
## Secondary Targets
#####################################################################


# Web output target
$(DIST_OUT)/index.html: code/main.cpp $(OUTPUT)/libraylib.a shell.html assets
	@mkdir -p $(@D)
	$(CC) code/main.cpp $(OUTPUT)/libraylib.a -o $(DIST_OUT)/index.html $(COMPILER_OPTS) -I code -s USE_GLFW=3 -s ASYNCIFY --shell-file shell.html --preload-file assets

$(DIST_OUT)/game.exe: code/main.cpp $(OUTPUT)/libraylib.a
	@mkdir -p $(@D)
	$(CC) code/main.cpp $(OUTPUT)/libraylib.a -o $(DIST_OUT)/game.exe -lstdc++ -lopengl32 -lgdi32 -lwinmm -lraylib -L./output/$(PLAT_ID)/ -I raylib -I code

$(OUTPUT)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) -c $^ -o $@ $(COMPILER_OPTS)

$(OUTPUT)/libraylib.a: $(RAYLIB_OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $(RAYLIB_OBJS)


ifdef WEB
RELEASE_CONTENT := $(DIST_OUT)/release.zip $(DIST_OUT)/index.html $(DIST_OUT)/index.js $(DIST_OUT)/index.wasm
endif
ifdef DESKTOP
RELEASE_CONTENT := $(DIST_OUT)/game.exe
endif
$(DIST_OUT)/release.zip: $(RELEASE_CONTENT)
	@mkdir -p $(@D)
	tar.exe -a -c -f $(DIST_OUT)/release.zip $(RELEASE_CONTENT)
