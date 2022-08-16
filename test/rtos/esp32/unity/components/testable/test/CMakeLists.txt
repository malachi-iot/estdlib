# v4.0 way of doing things
#idf_component_register(SRC_DIRS "."
#                    INCLUDE_DIRS "."
#                    REQUIRES unity testable)


# TODO: If ESTDLIB_DIR below works as expected, we can obsolete this
set(UNITY_SRCDIR "../../../../../../unity")
#set(UNITY_SRCDIR "")

# Worringly, this appears twice during build, first time with ESTDLIB_DIR populated
# and second time not.  However, compilation appears to pick up unity folder -
# so perhaps we're OK?  Test against ESP32 to be extra sure.  Note also tossing
# around NO_POLICY_SCOPE appears to make no difference
message("TESTABLE: UNIT_SRCDIR=${UNITY_SRCDIR} / ${ESTDLIB_DIR} / ${TESTESTD}")

#set(COMPONENT_SRCDIRS "." ${UNITY_SRCDIR})
set(COMPONENT_SRCDIRS "." ${ESTDLIB_DIR}/test/unity)
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_REQUIRES estdlib esp_timer unity testable)

register_component()

component_compile_options("-Wstack-usage=128")

# FEATURE_ESTD_CHARTRAITS works well.  Leaving it off in pursuit of getting to the
# bottom of std::char_traits bloat
#component_compile_options("-Wstack-usage=128" "-DFEATURE_ESTD_CHARTRAITS=1")

# Won't compile, only for diagnostic
#component_compile_options("-Wstack-usage=128" "-S")