.PHONY: setup build clean install

BUILD_TYPE ?= Release

setup:
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk && ./emsdk install latest
	cd emsdk && ./emsdk activate latest
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
	cd build && make install

clean:
	rm -rf build
	rm -rf install
