cmake_minimum_required(VERSION 2.8)

project(estdlib)

set(SOURCE_FILES
       dummy.cpp 
       estd/array.h
       estd/functional.h
       estd/utility.h
       estd/map.h)

add_library(estdlib_lib ${SOURCE_FILES})

