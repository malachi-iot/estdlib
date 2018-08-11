cmake_minimum_required(VERSION 2.8)

project(estd)

set(SOURCE_FILES
        dummy.cpp
        estd/algorithm.h
        estd/allocators/fixed.h
        estd/allocators/generic.h
        estd/allocators/handle_desc.h
        estd/array.h
        estd/cstdint.h
        estd/cstddef.h
        estd/chrono.h

        estd/exp/observer.h
        estd/exp/tmr.h

        estd/forward_list.h
        estd/functional.h
        estd/initializer_list.h

        estd/istream.h
        estd/ios.h
        estd/iostream.h

        estd/iterator.h

        estd/internal/accessor.h
        estd/internal/buffer.h
        estd/internal/common_type.h
        estd/internal/dynamic_array.h
        estd/internal/handle_with_offset.h
        estd/internal/is_function.h
        estd/internal/list_node.h
        estd/internal/opts.h
        estd/internal/platform.h
        estd/internal/priority_queue.h
        estd/internal/reverse_list.h
        estd/internal/runtime_array.h
        estd/internal/to_string.h
        estd/internal/llvm_type_traits.h
        estd/internal/impl/allocated_array.h
        estd/internal/impl/dynamic_array.h
        estd/internal/impl/handle_desc.h
        estd/internal/value_evaporator.h

        estd/iterators/list.h
        estd/list.h
        estd/map.h
        estd/memory.h

        estd/ostream.h

        estd/policy/list.h
        estd/policy/string.h

        estd/port/arch/esp-idf.h

        estd/port/chrono.h
        estd/port/chrono.hpp
        estd/port/string.h
        estd/port/identify_platform.h
        estd/port/arduino/chrono.h
        estd/port/arduino/string.h
        estd/port/arduino/thread.h
        estd/port/freertos/chrono.h
        estd/port/freertos/thread.h
        estd/port/thread.h

        estd/queue.h
        estd/ratio.h
        estd/span.h
        estd/streambuf.h
        estd/string.h
        estd/string_view.h
        estd/thread.h
        estd/traits/char_traits.h
        estd/traits/list_node.h
        estd/traits/allocator_traits.h
        estd/traits/string.h
        estd/type_traits.h
        estd/utility.h
        estd/vector.h
        estd/internal/deduce_fixed_size.h

        estd/internal/string_convert.h estd/port/posix/streambuf.cpp)

add_library(estd_lib ${SOURCE_FILES})

