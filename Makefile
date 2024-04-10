.PHONY: clean upload

index.html: code/main.cpp libraylib_wasm.a shell.html
	emcc -o ./index.html code/main.cpp -Os -Wall ./libraylib_wasm.a -I code -I raylib -L. -s USE_GLFW=3 -s ASYNCIFY --shell-file shell.html -DPLATFORM_WEB

# Object files are used to for the wasm build
rcore.o: raylib/rcore.c
	emcc -o rcore.o -c raylib/rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

rshapes.o: raylib/rshapes.c
	emcc -o rshapes.o -c raylib/rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

rtextures.o: raylib/rtextures.c
	emcc -o rtextures.o -c raylib/rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

rtext.o: raylib/rtext.c
	emcc -o rtext.o -c raylib/rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

rmodels.o: raylib/rmodels.c
	emcc -o rmodels.o -c raylib/rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

utils.o: raylib/utils.c
	emcc -o utils.o -c raylib/utils.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

raudio.o: raylib/raudio.c
	emcc -o raudio.o -c raylib/raudio.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I raylib

libraylib_wasm.a: rcore.o rshapes.o rtextures.o rtext.o rmodels.o utils.o raudio.o
	emar rcs libraylib_wasm.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o utils.o raudio.o

upload: release_wasm.zip
	butler push release_wasm.zip gerolmed/internal-test-project:web

release_wasm.zip: index.html index.js index.wasm
	tar.exe -a -c -f release_wasm.zip index.html index.js index.wasm

clean:
	rm -f ./*.o
	rm -f ./*.a
	rm -f ./*.exe
	rm -f ./index.*
	cd raylib; make clean RAYLIB_RELEASE_PATH=../
