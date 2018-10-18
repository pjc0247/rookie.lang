EMCC = /home/choochoo/emsdk/emscripten/incoming/emcc
SRCS := $(wildcard ./compiler/*.cpp)

all:
	export EMCC_DEBUG=1

	mkdir -p www
	mkdir -p intermediate
	$(EMCC) -Os -std=c++17 $(SRCS) -I. -Icompiler \
		-o www/rklang.html -Wc++11-extensions \
		-s WASM=1 \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
		-s DISABLE_EXCEPTION_CATCHING=0 \
		-s EXPORTED_FUNCTIONS='["_rk_exec", "_rk_exec_utf8"]' \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["allocate", "intArrayFromString", "ccall", "cwrap"]'
		
	export EMCC_DEBUG=""