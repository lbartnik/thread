cmake_minimum_required(VERSION 3.6)
project(thread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(R_LIBRARY "~/R-library")
set(R_INCLUDE "/usr/share/R/include")

include_directories(${R_LIBRARY}/Rcpp/include)
include_directories(${R_INCLUDE})

file(GLOB SOURCE_FILES src/*.cpp)
add_library(thread ${SOURCE_FILES})

add_custom_target(
    Rinstall
    COMMAND MAKEFLAGS=-j8 nice R CMD INSTALL --libs-only --no-test-load ${thread_SOURCE_DIR}
)
