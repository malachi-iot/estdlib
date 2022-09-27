cmake_minimum_required(VERSION 3.0)

project(estd VERSION 0.3.3 LANGUAGES CXX)

set(SOURCE_FILES
        dummy.cpp
        estd/algorithm.h
        estd/allocators/fixed.h
        estd/allocators/generic.h
        estd/allocators/handle_desc.h
        estd/array.h

        estd/c++03/tuple.h

        estd/cctype.h
        estd/charconv.h
        estd/cstdint.h
        estd/cstddef.h
        estd/chrono.h
        estd/deque.h

        estd/exp/functional.h
        estd/exp/memory/memory_pool.h
        estd/exp/memory/memory_pool2.h
        estd/exp/observer.h
        estd/exp/tmr.h

        estd/forward_list.h
        estd/functional.h
        estd/initializer_list.h

        estd/iomanip.h
        estd/ios.h
        estd/iostream.h
        estd/istream.h

        estd/iterator.h

        estd/internal/accessor.h
        estd/internal/buffer.h
        estd/internal/charconv.h
        estd/internal/charconv.hpp
        estd/internal/common_type.h
        estd/internal/deduce_fixed_size.h
        estd/internal/dynamic_array.h
        estd/internal/enum.h
        estd/internal/functional.h
        estd/internal/handle_with_offset.h
        estd/internal/impl/allocated_array.h
        estd/internal/impl/dynamic_array.h
        estd/internal/impl/handle_desc.h
        estd/internal/impl/streambuf/pos.h
        estd/internal/impl/streambuf/span.h
        estd/internal/impl/streambuf/string.h
        estd/internal/impl/streambuf.h
        estd/internal/invoke_result.h
        estd/internal/iterator_standalone.h
        estd/internal/ios.h
        estd/internal/iostream.h
        estd/internal/is_base_of.h
        estd/internal/is_function.h
        estd/internal/istream.h
        estd/internal/istream_runtimearray.hpp
        estd/internal/list_node.h
        estd/internal/locale.h
        estd/internal/opts.h
        estd/internal/ostream.h
        estd/internal/ostream_basic_string.hpp
        estd/internal/platform.h
        estd/internal/priority_queue.h
        estd/internal/promoted_type.h

        estd/internal/raw/cstddef.h
        estd/internal/raw/type_traits.h

        estd/internal/reverse_list.h
        estd/internal/runtime_array.h
        estd/internal/string_convert.h
        estd/internal/string_convert.cpp
        estd/internal/to_string.h
        estd/internal/type_traits.h
        estd/internal/llvm_type_traits.h
        estd/internal/utility.cpp
        estd/internal/utility.h
        estd/internal/value_evaporator.h

        estd/iterator.h

        estd/iterators/list.h
        estd/limits.h
        estd/list.h
        estd/locale.h

        estd/map.h
        estd/memory.h
        estd/mutex.h

        estd/numeric.h

        estd/optional.h
        estd/ostream.h

        estd/policy/list.h
        estd/policy/string.h

        estd/port/arch/esp-idf.h

        estd/port/asf/chrono.h
        estd/port/asf/thread.h

        estd/port/chrono.h
        estd/port/chrono.hpp
        estd/port/identify_platform.h
        estd/port/arduino/chrono.h
        estd/port/arduino/string.h
        estd/port/arduino/thread.h
        estd/port/esp-idf/chrono.h
        estd/port/mutex.h
        estd/port/freertos/chrono.h
        estd/port/freertos/thread.h
        estd/port/freertos/mutex.h
        estd/port/posix/chrono.h
        estd/port/posix/streambuf.cpp
        estd/port/posix/streambuf.h
        estd/port/posix/thread.h
        estd/port/streambuf.h
        estd/port/string.h
        estd/port/thread.h

        estd/port/toolchain/gnuc.h
        estd/port/toolchain/msvc.h

        estd/queue.h
        estd/ratio.h
        estd/span.h
        estd/sstream.h
        estd/stack.h
        estd/streambuf.h
        estd/string.h
        estd/string_view.h
        estd/system_error.h
        estd/thread.h
        estd/traits/char_traits.h
        estd/traits/list_node.h
        estd/traits/allocator_traits.h
        estd/traits/string.h
        estd/tuple.h
        estd/type_traits.h
        estd/utility.h
        estd/variant.h
        estd/vector.h

        estd/exp/heap.h
        estd/exp/streambuf-traits.h
        estd/exp/memory_resource.h)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
add_library(estd_lib ALIAS ${PROJECT_NAME})
add_library(malachi-iot::estd ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=return-type)

