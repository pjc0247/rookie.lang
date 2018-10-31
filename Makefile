EMCC = /home/choochoo/emsdk/emscripten/incoming/emcc
LIBS := $(wildcard ./libs/*.cpp)
SRCS := $(wildcard ./compiler/*.cpp)

all:
	export EMCC_DEBUG=1

	mkdir -p www
	$(EMCC) -O2 -std=c++17 $(SRCS) $(LIBS) -I. -Icompiler -Iincludes \
		-o www/rklang.html -Wc++11-extensions \
		-s WASM=1 \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
		-s DISABLE_EXCEPTION_CATCHING=0 \
		-s EXPORTED_FUNCTIONS='["_rk_exec"]' \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["allocate", "intArrayFromString", "ccall", "cwrap"]'
		
	export EMCC_DEBUG=""