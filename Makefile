.PHONY: release install-deps

all: dist/esp2ged

dist/esp2ged: esp2ged.cpp
	mkdir -p dist
	c++ -g esp2ged.cpp -lcxxtools -o dist/esp2ged

PARAMS=esp2ged.cpp -static -lcxxtools -lpthread
VERSION=$(shell head -n 1 esp2ged.cpp | cut -c 17-)
release:
	mkdir -p dist
	c++ ${PARAMS} -o dist/esp2ged
	i686-w64-mingw32-c++ ${PARAMS} -o dist/esp2ged32.exe
	x86_64-w64-mingw32-c++ ${PARAMS} -o dist/esp2ged64.exe
	strip -s dist/esp2ged dist/esp2ged32.exe dist/esp2ged64.exe
	zip -j dist/esp2ged-${VERSION}-linux64.zip dist/esp2ged
	zip -j dist/esp2ged-${VERSION}-win32.zip dist/esp2ged32.exe
	zip -j dist/esp2ged-${VERSION}-win64.zip dist/esp2ged64.exe

install-deps:
	sudo apt-get install g++-mingw-w64 libcxxtools-dev zip
