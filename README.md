
BEFORE:
emsdk_env.bat

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