# v4.0 way of doing things
#idf_component_register(SRC_DIRS "."
#                    INCLUDE_DIRS "."
#                    REQUIRES unity testable)


set(UNITY_SRCDIR "../../../../../../unity")
#set(UNITY_SRCDIR "")

set(COMPONENT_SRCDIRS "." ${UNITY_SRCDIR})
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_REQUIRES estdlib unity testable)

register_component()