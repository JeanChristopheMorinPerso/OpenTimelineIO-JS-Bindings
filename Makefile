.PHONY: setup build clean install

BUILD_TYPE ?= Release
EMSCRIPTEN_VERSION ?= 6.0.8

setup:
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk && ./emsdk install $(EMSCRIPTEN_VERSION)
	cd emsdk && ./emsdk activate $(EMSCRIPTEN_VERSION)
	pnpm install --frozen-lockfile

build:
	mkdir -p build
	cd build && \
	cmake ../ \
		-DCMAKE_INSTALL_PREFIX=$(shell pwd)/install \
		-DCMAKE_TOOLCHAIN_FILE=$(shell pwd)/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	# Emscripten's --emit-tsd invokes the TypeScript compiler from node_modules.
	cd build && PATH=$(shell pwd)/node_modules/.bin:$$PATH cmake --build . -j 16

install:
	cd build && cmake --install .

clean:
	rm -rf build
	rm -rf install

emscripten-version:
	@echo $(EMSCRIPTEN_VERSION)
