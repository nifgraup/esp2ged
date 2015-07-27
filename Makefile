.PHONY: release

all: dist/esp2ged

dist/esp2ged: esp2ged.cpp
	mkdir -p dist
	c++ -g -o dist/esp2ged esp2ged.cpp

PARAMS=esp2ged.cpp -static
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
