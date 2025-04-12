# v4.0 way of doing things
#idf_component_register(SRC_DIRS "."
#                    INCLUDE_DIRS "."
#                    REQUIRES unity testable)


set(COMPONENT_SRCDIRS ${ESTDLIB_DIR}/test/unity)
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_REQUIRES estd esp_timer unity testable)

register_component()

component_compile_options("-Wstack-usage=128")

# FEATURE_ESTD_CHARTRAITS works well.  Leaving it off in pursuit of getting to the
# bottom of std::char_traits bloat
#component_compile_options("-Wstack-usage=128" "-DFEATURE_ESTD_CHARTRAITS=1")

# Won't compile, only for diagnostic
#component_compile_options("-Wstack-usage=128" "-S")