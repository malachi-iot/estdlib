# v4.0 way of doing things
#idf_component_register(SRC_DIRS "."
#                    INCLUDE_DIRS "."
#                    REQUIRES unity testable)

# Clean this guy up as per https://github.com/malachi-iot/estdlib/issues/102

set(COMPONENT_SRCDIRS ${ESTDLIB_DIR}/test/unity)
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_REQUIRES estd esp_timer unity testable)

get_filename_component(CATCH_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../../catch ABSOLUTE)

include(${CATCH_DIR}/resources/resources.cmake)

list(TRANSFORM RESOURCES PREPEND ${CATCH_DIR}/resources/)

set(COMPONENT_EMBED_FILES ${RESOURCES})

register_component()

component_compile_options("-Wstack-usage=128")

# FEATURE_ESTD_CHARTRAITS works well.  Leaving it off in pursuit of getting to the
# bottom of std::char_traits bloat
#component_compile_options("-Wstack-usage=128" "-DFEATURE_ESTD_CHARTRAITS=1")

# Won't compile, only for diagnostic
#component_compile_options("-Wstack-usage=128" "-S")