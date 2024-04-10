# Engine for Ludum Dare 55

## Prerequisites

- https://github.com/skeeto/w64devkit
- https://github.com/emscripten-core/emscripten
  - _Might require calling:_ `emsdk_env.bat`

## Makefile instructions

```
FLAGS:

PLATFORM=WIN or WEB //DEFAULT: WEB
SILENT //if set hide verbose output

INSTRUCTIONS:
make:
make build: builds the project

make prepare_zip: Prepares a ZIP file for upload

make upload: Upload project via butler CLI. See section below

make clean: Cleans up the project
```

## Publish to Itch.io via Butler

Needs to be installed for publishing. See https://itch.io/docs/butler/