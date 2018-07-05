cmake_minimum_required(VERSION 2.8)

project(estd)

set(SOURCE_FILES
        dummy.cpp
        estd/algorithm.h
        estd/allocators/fixed.h
        estd/allocators/generic.h
        estd/allocators/handle_desc.h
        estd/array.h
        estd/chrono.h

        estd/exp/observer.h

        estd/forward_list.h
        estd/functional.h
        estd/initializer_list.h
        estd/iterator.h

        estd/internal/buffer.h
        estd/internal/common_type.h
        estd/internal/dynamic_array.h
        estd/internal/handle_with_offset.h
        estd/internal/list_node.h
        estd/internal/platform.h
        estd/internal/priority_queue.h
        estd/internal/reverse_list.h
        estd/internal/runtime_array.h
        estd/internal/to_string.h
        estd/internal/impl/allocated_array.h
        estd/internal/impl/dynamic_array.h
        estd/internal/impl/handle_desc.h

        estd/iterators/list.h
        estd/list.h
        estd/map.h
        estd/memory.h

        estd/port/chrono.h
        estd/port/chrono.hpp
        estd/port/freertos/chrono.h

        estd/queue.h
        estd/ratio.h
        estd/span.h
        estd/string.h
        estd/string_view.h
        estd/traits/list_node.h
        estd/traits/allocator_traits.h
        estd/traits/string.h
        estd/type_traits.h
        estd/utility.h
        estd/vector.h
        )

add_library(estd_lib ${SOURCE_FILES})

