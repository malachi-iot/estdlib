set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

# For some reason, these 'sets' won't propagate upward even with NO_POLICY_SCOPE
#set(ESTDLIB_DIR ${ROOT_DIR})

#message("ESTDLIB building: ${ESTDLIB_DIR}")

#set(TESTESTD "hi2u2")

set(COMPONENT_ADD_INCLUDEDIRS 
    ${ROOT_DIR}/src)

# NOTE: Even though we do this, a special cmake_policy or similar
# must be enacted for these changes to cascade out to 'includers',
# so for the time being you'll need to expressly include 
# version_finder.cmake yourself in your own CMakeLists.txt
include(${CMAKE_CURRENT_LIST_DIR}/version_finder.cmake)

register_component()