EMCC = /home/choochoo/emsdk/emscripten/incoming/emcc
LIBS := $(wildcard ./libs/*.cpp)
SRCS := $(wildcard ./compiler/*.cpp)
SRCS_SYNTAX := $(wildcard ./compiler/syntax/*.cpp)

native:
	clang++ -O2 -stdlib=libc++ -std=c++17 $(SRCS) $(SRCS_SYNTAX) $(LIBS) -I. -Icompiler -Iincludes \
		-v -lstdc++fs -o rookie -Wc++11-extensions 

all:
	export EMCC_DEBUG=1

	mkdir -p www
	$(EMCC) -O2 -std=c++17 $(SRCS) $(SRCS_SYNTAX) $(LIBS) -I. -Icompiler -Iincludes \
		-g4 \
		-o www/rklang.html -Wc++11-extensions \
		-s WASM=1 \
		-s "BINARYEN_TRAP_MODE='clamp'" \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
		-s DISABLE_EXCEPTION_CATCHING=0 \
		-s EXPORTED_FUNCTIONS='["_rk_exec"]' \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["allocate", "intArrayFromString", "ccall", "cwrap"]'
		
	export EMCC_DEBUG=""