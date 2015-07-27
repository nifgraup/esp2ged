.PHONY: release

all: dist/esp2ged

dist/esp2ged: esp2ged.cpp
	mkdir -p dist
	c++ -g -o dist/esp2ged esp2ged.cpp

release:
	mkdir -p dist
	c++ -o dist/esp2ged esp2ged.cpp
	i686-w64-mingw32-c++ -o dist/esp2ged32.exe esp2ged.cpp
	x86_64-w64-mingw32-c++ -o dist/esp2ged64.exe esp2ged.cpp
	strip -s dist/*
