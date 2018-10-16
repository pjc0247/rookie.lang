EMCC = /home/choochoo/emsdk/emscripten/incoming/emcc
SRCS := $(wildcard ./compiler/*.cpp)

all:
	export EMCC_DEBUG=1

	mkdir -p www
	mkdir -p intermediate
	$(EMCC) -O3 -std=c++17 $(SRCS) -I. -Icompiler \
		-o intermediate/p.bc -Wc++11-extensions
	$(EMCC) -O3 intermediate/p.bc \
		-o www/rklang.html \
		-s WASM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
		-s DISABLE_EXCEPTION_CATCHING=0 \
		-s EXPORTED_FUNCTIONS='["_rk_compile_and_run"]' \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["allocate", "intArrayFromString", "ccall", "cwrap"]'

	export EMCC_DEBUG=""