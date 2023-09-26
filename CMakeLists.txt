cmake_minimum_required (VERSION 3.12)

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
option(DEBUG "compile with debug symbol" OFF)
option(STATIC "compile statically" OFF)
option(COVERAGE "Enable code coverage" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(BUILD_TESTS "Build tests" OFF)

if(COVERAGE)
  SET(BUILD_TESTS ON)
endif(COVERAGE)

if(BUILD_TESTS)
  find_package(Cunit REQUIRED)
endif(BUILD_TESTS)

if(INDEX)
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DRECORD_INDEX='true'")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRECORD_INDEX='true'")
endif(INDEX)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
  set(CMAKE_BUILD_TYPE Debug)
endif()

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

if(COVERAGE)
  include(CodeCoverage)
  setup_target_for_coverage(NAME coverage
    EXECUTABLE ficlip-test
    DEPENDENCIES ficlip-test
    )
  SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif(COVERAGE)

if(BUILD_TESTS)
  add_executable(ficlip-test tests/ficlip-test.c)

  target_link_libraries(ficlip-test
    ficlip
    ${CUNIT_LIBRARY}
  )
endif(BUILD_TESTS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")


if (NOT LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR lib)
endif ()

if (NOT BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR bin)
endif ()

if (NOT INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR include)
endif ()


# install binaries and library
INSTALL(TARGETS ficlip
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

# install header file
INSTALL(FILES inc/ficlip.h DESTINATION ${INCLUDE_INSTALL_DIR})
