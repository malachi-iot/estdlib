cmake_minimum_required(VERSION 2.8)

project(estdlib)

set(SOURCE_FILES
       dummy.cpp 
       platform.h
       estd/array.h
        estd/forward_list.h
       estd/functional.h
       estd/iterator.h
       estd/map.h
       estd/queue.h
       estd/utility.h
        estd/vector.h
       )

add_library(estdlib_lib ${SOURCE_FILES})

