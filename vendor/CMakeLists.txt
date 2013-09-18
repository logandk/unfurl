cmake_minimum_required(VERSION 2.6)
project(vendor)
include(ExternalProject)

ExternalProject_Add(
  pugixml
  URL http://pugixml.googlecode.com/files/pugixml-1.2.zip
  PREFIX pugixml
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/patch.cmake
  CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/pugixml
)

ExternalProject_Add(
  zlib
  URL http://zlib.net/zlib-1.2.8.tar.gz
  PREFIX zlib
  CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/zlib
)


