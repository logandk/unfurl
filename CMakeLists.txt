cmake_minimum_required(VERSION 2.6)
project(unfurl)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BUILD_SHARED_LIBS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/pugixml/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/pugixml/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/zlib/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/zlib/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/zlib/src/zlib/contrib/minizip)

if (CMAKE_COMPILER_IS_GNUCXX )
  SET(CMAKE_CXX_FLAGS "-Wall -fno-rtti -O2")
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static -s -mwindows")
endif()

add_executable(unfurl src/main.cpp
  vendor/zlib/src/zlib/contrib/minizip/unzip.c
  vendor/zlib/src/zlib/contrib/minizip/ioapi.c)
target_link_libraries(unfurl shlwapi pugixml zlibstatic)

install(TARGETS unfurl DESTINATION bin)
