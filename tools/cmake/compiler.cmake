include_guard()

# DEBT: Add a parameter or option to change 'PRIVATE' default
# DEBT: Strongly consider making this "target_estd_compile_options" and friends


# -Wno-unused-variable want this too, remember .H files aren't compiled independently
#   so we have to report unused variables in unit tests fused with headers
# -Wextra) not ready for this just yet, but want it


macro(estd_gcc_compile_options)
    # Run, don't walk, do your nearest absent return type
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=return-type)

    # As per
    # https://stackoverflow.com/questions/31890021/mingw-too-many-sections-bug-while-compiling-huge-header-file-in-qt
    if(MSYS OR MINGW)
        message(STATUS "MSYS mode")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wa,-mbig-obj)
    endif()
endmacro()

macro(estd_msvc_compile_options)
    # Kind of a blunt tool, but we're exporting so little anyway it's not too bad
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # As per https://developercommunity.visualstudio.com/t/msvc-incorrectly-defines-cplusplus/139261
    # DEBT: We'll likely want this PUBLIC or INTERFACE
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus)
endmacro()

macro(estd_compile_options)
    # DEBT: Make a macro/function so we can compare "GNU-like" and include Clang, LLVM, etc.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        estd_gcc_compile_options()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        estd_msvc_compile_options()
    endif()
endmacro()
