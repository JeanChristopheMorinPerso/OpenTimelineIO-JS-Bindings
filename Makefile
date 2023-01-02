.PHONY: init-emsdk build clean install

init-emsdk:
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk && ./emsdk install latest
	cd emsdk && ./emsdk activate latest

build:
	mkdir -p build
	cd build && \
	cmake ../ -DCMAKE_INSTALL_PREFIX=$(shell pwd)/install -DCMAKE_TOOLCHAIN_FILE=$(shell pwd)/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake && \
	cmake --build . -j 16

install:
	cd build && make install

clean:
	rm -rf build
	rm -rf install
