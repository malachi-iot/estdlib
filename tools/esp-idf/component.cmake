set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

set(COMPONENT_ADD_INCLUDEDIRS 
    ${ROOT_DIR}/src)

# NOTE: Even though we do this, a special cmake_policy or similar
# must be enacted for these changes to cascade out to 'includers',
# so for the time being you'll need to expressly include 
# version_finder.cmake yourself in your own CMakeLists.txt
include(${CMAKE_CURRENT_LIST_DIR}/version_finder.cmake)

register_component()