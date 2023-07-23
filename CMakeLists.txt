cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
set(CMAKE_VERBOSE_MAKEFILE on)

project("DrSymLogger")

add_definitions(-DDR_BUILD)

find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)

add_library(DrSymLogger SHARED
    src/DrSymLogger.cpp
    src/Symbols.cpp
    src/args.cpp
    )

configure_DynamoRIO_client(DrSymLogger)
use_DynamoRIO_extension(DrSymLogger drmgr)
use_DynamoRIO_extension(DrSymLogger drx)
use_DynamoRIO_extension(DrSymLogger drcontainers)
use_DynamoRIO_extension(DrSymLogger drreg)
use_DynamoRIO_extension(DrSymLogger drwrap)
use_DynamoRIO_extension(DrSymLogger drsyms)
