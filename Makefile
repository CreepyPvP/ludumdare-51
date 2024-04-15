
RAYLIB := raylib/rcore raylib/rshapes raylib/rtextures raylib/rtext raylib/rmodels raylib/utils raylib/raudio
#####################################################################
## PLATFORM DEFINITION
#####################################################################
PLATFORM ?= WIN

ifndef PROD
ifndef SILENT
$(info DEBUG MODE)
endif
endif


ifeq ($(PLATFORM),WEB)

ifndef SILENT
$(info Building for web...)
endif
CC := emcc
AR := emar
COMPILER_OPTS := -Wno-enum-compare -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib
COMPILER_OPTS_RAYLIB := $(COMPILER_OPTS)
PLAT_ID := web
WEB := 1
else ifeq ($(PLATFORM),WIN)
ifndef SILENT
$(info Building for windows...)
endif
CC := gcc
AR := ar
PLAT_ID := win

ifndef PROD
COMPILER_OPTS := -Wno-enum-compare -lstdc++ -lopengl32 -lgdi32 -lwinmm -lraylib -I raylib -g -O0
else
COMPILER_OPTS := -Wno-enum-compare -lstdc++ -lopengl32 -lgdi32 -lwinmm -lraylib -I raylib -03
endif
COMPILER_OPTS_RAYLIB := $(COMPILER_OPTS) -D_GNU_SOURCE -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -I raylib/external/glfw/include
DESKTOP := 1
RAYLIB += raylib/rglfw

else
$(error Bad PLATFORM specified '$(PLATFORM)'. Supported Targets: WEB, WIN)
endif

#####################################################################
## Common VARIABLES
#####################################################################

OUTPUT := output/$(PLAT_ID)
DIST_OUT := .
RAYLIB_OBJS := $(foreach item,$(RAYLIB),$(OUTPUT)/$(item).o)


.PHONY: clean upload prepare_zip build all $(DIST_OUT)/game.exe
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
	butler push $(DIST_OUT)/release.zip gerolmed/summoner-defence-forces:$(PLAT_ID)

clean:
	rm -rf output
	cd raylib; make clean RAYLIB_RELEASE_PATH=../

#####################################################################
## Secondary Targets
#####################################################################


# Web output target
$(DIST_OUT)/index.html: code/main.cpp $(OUTPUT)/libraylib.a shell.html assets
	@mkdir -p $(@D)
	$(CC) code/main.cpp $(OUTPUT)/libraylib.a -o $(DIST_OUT)/index.html $(COMPILER_OPTS) -I code -L./output/$(PLAT_ID)/ -s USE_GLFW=3 -s ASYNCIFY --shell-file shell.html --preload-file assets

$(DIST_OUT)/game.exe: code/main.cpp $(OUTPUT)/libraylib.a
	@mkdir -p $(@D)
	$(CC) code/main.cpp $(OUTPUT)/libraylib.a -o $(DIST_OUT)/game.exe $(COMPILER_OPTS) -I code -L./output/$(PLAT_ID)/

$(OUTPUT)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) -c $^ -o $@ $(COMPILER_OPTS_RAYLIB)

$(OUTPUT)/libraylib.a: $(RAYLIB_OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $(RAYLIB_OBJS)


ifdef WEB
RELEASE_CONTENT := $(DIST_OUT)/index.html $(DIST_OUT)/index.js $(DIST_OUT)/index.wasm $(DIST_OUT)/index.data
endif
ifdef DESKTOP
RELEASE_CONTENT := $(DIST_OUT)/game.exe
endif
$(DIST_OUT)/release.zip: $(RELEASE_CONTENT)
	@mkdir -p $(@D)
	tar.exe -a -c -f $(DIST_OUT)/release.zip $(RELEASE_CONTENT)
