set(ROOT_DIR ../../../../../..)

MESSAGE(FATAL_ERROR "Got here")
 
set(COMPONENT_ADD_INCLUDEDIRS 
    ${COMPONENT_ADD_INCLUDEDIRS}
    ${ROOT_DIR}/src)

register_component()