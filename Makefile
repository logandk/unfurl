.PHONY: all test vendor clean

all:
	cd build; cmake ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=$(CURDIR) ..; make all install

test: all
	cd tests/build; cmake ${CMAKE_ARGS} ..; make
	./tests/build/tests ${TEST_ARGS}

vendor:
	cd vendor; cmake ${CMAKE_ARGS} .; make

clean:
	rm -f build/*.*
	rm -f build/Makefile
	rm -f -r build/CMake*
	rm -f tests/build/*.*
	rm -f tests/build/Makefile
	rm -f -r tests/build/CMake*
	rm -f vendor/CMakeCache.txt
	rm -f vendor/cmake_install.cmake
	rm -f vendor/Makefile
	rm -f -r vendor/CMakeFiles
	rm -f -r vendor/pugixml/include
	rm -f -r vendor/pugixml/lib
	rm -f -r vendor/pugixml/src
	rm -f -r vendor/pugixml/tmp
	rm -f -r vendor/zlib/bin
	rm -f -r vendor/zlib/include
	rm -f -r vendor/zlib/lib
	rm -f -r vendor/zlib/share
	rm -f -r vendor/zlib/src
	rm -f -r vendor/zlib/tmp

