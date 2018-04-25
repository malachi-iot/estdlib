cmake_minimum_required(VERSION 2.8)

project(estd)

set(SOURCE_FILES
       dummy.cpp 
       platform.h
        estd/allocators/fixed.h
        estd/allocators/generic.h
       estd/array.h
        estd/forward_list.h
        estd/functional.h
        estd/initializer_list.h
        estd/iterator.h
        estd/iterators/list.h
        estd/list.h
       estd/map.h
        estd/memory.h
       estd/queue.h
        estd/string.h
        estd/traits/node_list.h
        estd/utility.h
        estd/vector.h
       )

add_library(estd_lib ${SOURCE_FILES})

