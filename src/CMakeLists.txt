cmake_minimum_required(VERSION 2.8)

project(estd)

set(SOURCE_FILES
       dummy.cpp 
       platform.h
        estd/algorithm.h
        estd/allocators/fixed.h
        estd/allocators/generic.h
       estd/array.h
        estd/forward_list.h
        estd/functional.h
        estd/initializer_list.h
        estd/iterator.h
        estd/internal/dynamic_array.h
        estd/internal/handle_with_offset.h
        estd/internal/list_node.h
        estd/internal/priority_queue.h
        estd/internal/reverse_list.h
        estd/internal/to_string.h
        estd/iterators/list.h
        estd/list.h
        estd/map.h
        estd/memory.h
        estd/queue.h
        estd/string.h
        estd/traits/list_node.h
        estd/traits/allocator_traits.h
        estd/type_traits.h
        estd/utility.h
        estd/vector.h
        )

add_library(estd_lib ${SOURCE_FILES})

