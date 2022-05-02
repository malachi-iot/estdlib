set(COMPONENT_SRCS "ostream.cpp" "freertos_main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

component_compile_options("-Wstack-usage=32")
# For diagnostic only - will break compilation
#component_compile_options("-Wstack-usage=32" "-S")
