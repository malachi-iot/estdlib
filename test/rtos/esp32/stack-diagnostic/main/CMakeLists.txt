set(COMPONENT_SRCS "main.cpp")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

component_compile_options("-Wstack-usage=64")
# For diagnostic only - will break compilation
component_compile_options("-Wstack-usage=64" "-S")
