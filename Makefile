.PHONY: setup build clean install

BUILD_TYPE ?= Release
EMSCRIPTEN_VERSION ?= 3.1.34

setup:
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk && ./emsdk install $(EMSCRIPTEN_VERSION)
	cd emsdk && ./emsdk activate $(EMSCRIPTEN_VERSION)
	npm install

build:
	mkdir -p build
	cd build && \
	cmake ../ \
		-DCMAKE_INSTALL_PREFIX=$(shell pwd)/install \
		-DCMAKE_TOOLCHAIN_FILE=$(shell pwd)/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cd build && cmake --build . -j 16

install:
	cd build && cmake --install .

clean:
	rm -rf build
	rm -rf install

emscripten-version:
	@echo $(EMSCRIPTEN_VERSION)
