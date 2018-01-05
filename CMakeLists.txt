cmake_minimum_required (VERSION 2.6)

project (ficlip)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Project version (sematic versionning)
set(ficlip_VERSION_MAJOR 0)
set(ficlip_VERSION_MINOR 0)
set(ficlip_VERSION_PATCH 1)

set(ficlip_VERSION ${ficlip_VERSION_MAJOR}.${ficlip_VERSION_MINOR}.${ficlip_VERSION_PATCH})

add_custom_target(tag
  COMMAND git tag -a ${ficlip_VERSION} -m "tagging version ${ficlip_VERSION}"
  COMMAND git push origin ${ficlip_VERSION}
)

# set version as a definition
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DFI_VERSION='\"${ficlip_VERSION}\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFI_VERSION='\"${ficlip_VERSION}\"'")

# Options
option(DEBUG     "compile with debug symbol"    OFF)
option(UNITTEST  "compile unit tests"           OFF)
option(STATIC    "compile statically"           OFF)
option(USE_CLANG "build application with clang" OFF)
option(USE_GCC   "build application with gcc"   OFF)

if(UNITTEST)
  find_package(Cunit REQUIRED)
endif(UNITTEST)

if(INDEX)
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DRECORD_INDEX='true'")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRECORD_INDEX='true'")
endif(INDEX)

if(USE_CLANG)
  set(CMAKE_CXX_COMPILER "clang++")
  set(CMAKE_CC_COMPILER  "clang"  )
endif(USE_CLANG)

if(USE_GCC)
  set(CMAKE_CXX_COMPILER "g++")
  set(CMAKE_CC_COMPILER  "gcc")
endif(USE_GCC)

if(DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
  set(CMAKE_BUILD_TYPE Debug)
endif(DEBUG)

if(STATIC)
  set(SHARED "")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(BUILD_SHARED_LIBRARIES OFF)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
else(STATIC)
  set(SHARED "SHARED")
endif(STATIC)

if(UNIX)
  link_libraries(m)
endif(UNIX)

# linking directories
LINK_DIRECTORIES(
  ${CMAKE_BINARY_DIR}/
  /usr/local/lib
  /usr/lib/
  ${CUNIT_INCLUDE_DIR}
)

# headers directories
include_directories(
  ./inc/
  /usr/include/
  /sw/include/
)

# Compile the library
add_library(ficlip
  ${SHARED}
  src/ficlip.c
)

set_target_properties(ficlip
  PROPERTIES
  VERSION ${ficlip_VERSION}
  SOVERSION ${ficlip_VERSION_MAJOR}
)

if(UNITTEST)
  add_executable(ficlip-test tests/ficlip-test.c)

  target_link_libraries(ficlip-test
    ficlip
    ${CUNIT_LIBRARY}
  )
endif(UNITTEST)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")

# install binaries and library
INSTALL(TARGETS ficlip
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# install header file
INSTALL(FILES inc/ficlip.h DESTINATION "include")
